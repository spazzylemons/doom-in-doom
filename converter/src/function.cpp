#include <cstdio>
#include <llvm/Support/AllocatorBase.h>
#include <set>

#include <cstdlib>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

#include "function.hpp"
#include "memory.hpp"

struct FuncCompileCtx {
    const llvm::DataLayout& layout;
    const GlobalMemory& memory;

    FuncCompileCtx(const llvm::DataLayout& layout, const GlobalMemory& memory);

    std::ostringstream content;

    std::map<const llvm::BasicBlock *, uint32_t> blocks;
    std::map<const llvm::Instruction *, std::string> instructions;

    uint32_t globalCounter = 0U;
    uint32_t localCounter = 0U;
    uint32_t phiCounter = 0U;

    std::set<std::string> varNames;

    std::string getValue(const llvm::Value *val);
    std::string getSignedValue(const llvm::Value *val);

    void compile(const llvm::Function& func);

    void compileHeader(const llvm::Function& func);

    void allocRegisters(const llvm::BasicBlock& block);

    void compileBlock(const llvm::BasicBlock& block);
    void compileIns(const llvm::Instruction& ins);

    void compileTerminator(const llvm::Instruction& ins);
    void compileValue(const llvm::Instruction& ins);

    void truncateValue(llvm::Type *t);
};

FuncCompileCtx::FuncCompileCtx(const llvm::DataLayout& layout, const GlobalMemory& memory)
    : layout(layout)
    , memory(memory) {}

static std::string globalName(unsigned int index) {
    return "g" + std::to_string(index);
}

static std::string localName(unsigned int index) {
    return "l" + std::to_string(index);
}

static std::string signExtend(const std::string& value, unsigned int bits) {
    auto m = 32 - bits;
    auto v = std::to_string(m);
    // TODO test if this works in practice.
    return "(int(" + value + "<<" + v + ")>>" + v + ")";
}

std::string FuncCompileCtx::getValue(const llvm::Value *val) {
    if (auto v = llvm::dyn_cast<llvm::ConstantInt>(val)) {
        return std::to_string(v->getZExtValue()) + "U";
    } else if (auto v = llvm::dyn_cast<llvm::Instruction>(val)) {
        return instructions.at(v);
    } else if (auto v = llvm::dyn_cast<llvm::ConstantExpr>(val)) {
        auto baseIns = v->getAsInstruction();
        std::string result;
        if (auto ins = llvm::dyn_cast<llvm::GetElementPtrInst>(baseIns)) {
            llvm::APInt elementOffset(32, 0);
            if (!ins->accumulateConstantOffset(layout, elementOffset)) {
                fprintf(stderr, "Non-constant offset\n");
                exit(EXIT_FAILURE);
            }

            auto ptrName = ins->getOperand(0)->getName().str();
            auto ptrOffset = memory.getAddress(ptrName);

            result = std::to_string(ptrOffset + elementOffset.getZExtValue()) + "U";
        } else if (auto ins = llvm::dyn_cast<llvm::PtrToIntInst>(baseIns)) {
            result = std::move(getValue(ins->getOperand(0)));
        } else if (auto ins = llvm::dyn_cast<llvm::IntToPtrInst>(baseIns)) {
            result = std::move(getValue(ins->getOperand(0)));
        } else {
            fprintf(stderr, "Unsupported constant expression\n");
            exit(EXIT_FAILURE);
        }
        baseIns->deleteValue();
        return result;
    } else if (auto v = llvm::dyn_cast<llvm::GlobalVariable>(val)) {
        return std::to_string(memory.getAddress(v->getName().str())) + "U";
    } else if (auto v = llvm::dyn_cast<llvm::ConstantPointerNull>(val)) {
        return "0U";
    } else if (auto v = llvm::dyn_cast<llvm::Function>(val)) {
        return std::to_string(memory.getFuncIndex(v)) + "U";
    } else if (auto v = llvm::dyn_cast<llvm::Argument>(val)) {
        return "a" + std::to_string(v->getArgNo());
    } else if (auto v = llvm::dyn_cast<llvm::PoisonValue>(val)) {
        return "0U";
    } else if (auto v = llvm::dyn_cast<llvm::UndefValue>(val)) {
        return "0U";
    } else {
        val->printAsOperand(llvm::errs());
        llvm::errs() << "\n";

        fprintf(stderr, "Unimplemented value\n");
        exit(EXIT_FAILURE);
    }
}

std::string FuncCompileCtx::getSignedValue(const llvm::Value *val) {
    if (auto v = llvm::dyn_cast<llvm::ConstantInt>(val)) {
        return std::to_string(v->getSExtValue());
    } else {
        auto base = getValue(val);
        auto bits = layout.getTypeSizeInBits(val->getType()).getFixedValue();
        if (bits < 32) {
            return signExtend(base, bits);
        } else if (bits > 32) {
            fprintf(stderr, "Sign-extending value that is too large\n");
            exit(EXIT_FAILURE);
        } else {
            return "int(" + base + ")";
        }
    }
}

void FuncCompileCtx::allocRegisters(const llvm::BasicBlock& block) {
    std::map<const llvm::Instruction *, unsigned int> localMapping;
    std::set<const llvm::User *> localsAfter;

    for (const auto& ins : block) {
        localsAfter.insert(&ins);
    }

    auto nextIndex = 0U;
    auto localPhiCount = 0U;

    for (const auto& ins : block) {
        // Any instructions referenced only in their own block get a "local"
        // variable, otherwise get a "global" variable.
        bool isUsedOutsideOfBlock = false;
        for (const auto user : ins.users()) {
            if (auto userIns = llvm::dyn_cast<llvm::Instruction>(user)) {
                if (llvm::isa<llvm::PHINode>(userIns) || userIns->getParent() != &block) {
                    isUsedOutsideOfBlock = true;
                    break;
                }
            }
        }

        if (llvm::isa<llvm::PHINode>(ins)) {
            localPhiCount++;
        }

        std::string varName;

        if (isUsedOutsideOfBlock) {
            instructions[&ins] = globalName(globalCounter++);
            varNames.insert(instructions.at(&ins));
        } else if (ins.users().begin() != ins.users().end()) {
            // To save on locals, check if any locals we've already allocated
            // have no more uses in this block.
            unsigned int reusable;
            bool canBeReused = false;
            for (const auto [prevIns, prevIndex] : localMapping) {
                for (const auto user : prevIns->users()) {
                    if (localsAfter.find(user) != localsAfter.end()) {
                        goto alreadyUsed;
                    }
                }

                localMapping.erase(prevIns);
                canBeReused = true;
                reusable = prevIndex;
                break;

            alreadyUsed:
                ;
            }

            if (!canBeReused) {
                reusable = nextIndex++;
            }

            localMapping[&ins] = reusable;
            varName = localName(reusable);
        }

        localsAfter.erase(&ins);

        if (!varName.empty()) {
            instructions[&ins] = varName;
            varNames.insert(std::move(varName));
        }
    }

    if (nextIndex > localCounter) {
        localCounter = nextIndex;
    }

    if (localPhiCount > phiCounter) {
        phiCounter = localPhiCount;
    }
}

void FuncCompileCtx::compileHeader(const llvm::Function& func) {
    if (func.getReturnType()->isSized()) {
        content << "uint";
    } else {
        content << "void";
    }
    content << " func_" << func.getName().str() << "(";
    auto index = 0;
    for (const auto& arg : func.args()) {
        if (index) {
            content << ",";
        }
        content << "uint a" << index++;
    }

    if (func.isVarArg()) {
        if (index) {
            content << ",";
        }
        content << "VAList v";
    }

    content << ") {\n";
}

void FuncCompileCtx::compile(const llvm::Function& func) {
    compileHeader(func);

    auto i = 0U;
    for (const auto& block : func) {
        blocks[&block] = i++;
        allocRegisters(block);
    }

    for (i = 0U; i < phiCounter; i++) {
        varNames.insert("p" + std::to_string(i));
    }

    if (!varNames.empty()) {
        i = 0U;
        content << "uint ";
        for (const auto& v : varNames) {
            if (i++) content << ",";
            content << v;
        }
        content << ";\n";
    }

    auto hasOtherBlocks = ++func.begin() != func.end();

    if (hasOtherBlocks) {
        content << "uint label,lastLabel;\n";
    }

    content << "uint s=stack;\n";
    compileBlock(func.getEntryBlock());

    if (hasOtherBlocks) {
        content << "while(1)switch(label) {\n";

        i = 0U;
        for (const auto& block : func) {
            if (!block.isEntryBlock()) {
                content << "case " << i << ":\n";
                compileBlock(block);
            }
            i++;
        }
        content << "}\n";
        content << "Unreachable();\n";

        // makes gzdoom happy to have return statement here
        if (func.getReturnType()->isSized()) {
            content << "return 0;\n";
        }
    }

    content << "}\n";
}

void FuncCompileCtx::compileBlock(const llvm::BasicBlock& block) {
    auto it = block.begin();
    auto ie = block.end();

    auto phiCount = 0U;
    std::vector<const llvm::Instruction *> phiMapping;
    for (; it != ie; it++) {
        const auto& ins = *it;
        if (llvm::isa<llvm::PHINode>(ins)) {
            content << "p" << phiCount++ << "=";
            compileValue(ins);
            content << ";\n";
            phiMapping.push_back(&ins);
        } else {
            break;
        }
    }

    auto i = 0U;
    for (auto ins : phiMapping) {
        content << getValue(ins) << "=p" << i++ << ";\n";
    }

    for (; it != ie; it++) {
        compileIns(*it);
    }
}

void FuncCompileCtx::compileIns(const llvm::Instruction& ins) {
    if (ins.isTerminator()) {
        compileTerminator(ins);
    } else {
        if (ins.users().begin() != ins.users().end()) {
            content << getValue(&ins) << "=";
        }
        compileValue(ins);
        content << ";\n";
    }
}

void FuncCompileCtx::compileTerminator(const llvm::Instruction& baseIns) {
    switch (baseIns.getOpcode()) {
        case llvm::Instruction::Ret: {
            const auto& ins = llvm::cast<llvm::ReturnInst>(baseIns);
            auto value = ins.getReturnValue();

            content << "stack=s;\n";
            content << "return";
            if (value != nullptr) {
                content << " " << getValue(value);
            }
            content << ";\n";

            break;
        }

        case llvm::Instruction::Br: {
            const auto& ins = llvm::cast<llvm::BranchInst>(baseIns);

            content << "lastLabel=label;\n";
            if (ins.isConditional()) {
                content << "label=";
                content << getValue(ins.getCondition());
                content << "?";
                content << blocks.at(ins.getSuccessor(0));
                content << ":";
                content << blocks.at(ins.getSuccessor(1));
                content << ";\n";
            } else {
                content << "label=";
                content << blocks.at(ins.getSuccessor(0));
                content << ";\n";
            }
            break;
        }

        case llvm::Instruction::Switch: {
            const auto& ins = llvm::cast<llvm::SwitchInst>(baseIns);

            content << "lastLabel=label;\n";
            content << "switch(" << getValue(ins.getCondition()) << "){\n";
            for (const auto& c : ins.cases()) {
                content << "case " << c.getCaseValue()->getZExtValue() << ":\n";
                content << "label=" << blocks.at(c.getCaseSuccessor()) << ";\n";
                content << "break;\n";
            }
            content << "default:\n";
            content << "label=" << blocks.at(ins.getDefaultDest()) << ";\n";
            content << "}\n";
            break;
        }

        case llvm::Instruction::Unreachable: {
            content << "Unreachable();\n";
            break;
        }

        default: {
            fprintf(stderr, "Unsupported opcode\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!baseIns.getParent()->isEntryBlock()) {
        content << "break;\n";
    }
}

void FuncCompileCtx::compileValue(const llvm::Instruction& baseIns) {
    if (baseIns.isBinaryOp()) {
        const auto& ins = llvm::cast<llvm::BinaryOperator>(baseIns);
        auto opcode = ins.getOpcode();

        bool lhsSigned, rhsSigned;
        switch (opcode) {
            case llvm::Instruction::BinaryOps::SDiv:
            case llvm::Instruction::BinaryOps::SRem:
                lhsSigned = true;
                rhsSigned = true;
                break;
            case llvm::Instruction::BinaryOps::AShr:
                lhsSigned = true;
                rhsSigned = false;
                break;
            default:
                lhsSigned = false;
                rhsSigned = false;
                break;
        }

        bool needsTruncation;
        switch (opcode) {
            case llvm::Instruction::BinaryOps::Add:
            case llvm::Instruction::BinaryOps::Sub:
            case llvm::Instruction::BinaryOps::Mul:
            case llvm::Instruction::BinaryOps::Shl:
            case llvm::Instruction::BinaryOps::AShr:
            case llvm::Instruction::BinaryOps::SDiv:
            case llvm::Instruction::BinaryOps::SRem:
                needsTruncation = true;
                break;
            default:
                needsTruncation = false;
                break;
        }

        auto lhs = ins.getOperand(0);
        auto rhs = ins.getOperand(1);

        if (layout.getTypeSizeInBits(ins.getType()) == 32) {
            needsTruncation = false;
        }

        if (needsTruncation) {
            content << "(";
        }

        if (lhsSigned) {
            content << getSignedValue(lhs);
        } else {
            content << getValue(lhs);
        }

        switch (opcode) {
            case llvm::Instruction::BinaryOps::Add:
                content << "+";
                break;
            case llvm::Instruction::BinaryOps::Sub:
                content << "-";
                break;
            case llvm::Instruction::BinaryOps::Mul:
                content << "*";
                break;
            case llvm::Instruction::BinaryOps::UDiv:
            case llvm::Instruction::BinaryOps::SDiv:
                content << "/";
                break;
            case llvm::Instruction::BinaryOps::URem:
            case llvm::Instruction::BinaryOps::SRem:
                content << "%";
                break;
            case llvm::Instruction::BinaryOps::LShr:
            case llvm::Instruction::BinaryOps::AShr:
                content << ">>";
                break;
            case llvm::Instruction::BinaryOps::Shl:
                content << "<<";
                break;
            case llvm::Instruction::BinaryOps::And:
                content << "&";
                break;
            case llvm::Instruction::BinaryOps::Or:
                content << "|";
                break;
            case llvm::Instruction::BinaryOps::Xor:
                content << "^";
                break;
            default:
                fprintf(stderr, "Unsupported binary operator\n");
                exit(EXIT_FAILURE);
        }

        if (rhsSigned) {
            content << getSignedValue(rhs);
        } else {
            content << getValue(rhs);
        }

        if (needsTruncation) {
            content << ")";
            truncateValue(baseIns.getType());
        }
    } else {
        switch (baseIns.getOpcode()) {
            case llvm::Instruction::Load: {
                auto& ins = llvm::cast<llvm::LoadInst>(baseIns);
                auto bitWidth = layout.getTypeSizeInBits(ins.getAccessType()).getFixedValue();
                switch (bitWidth) {
                    case 1:
                        content << "Load1(";
                        break;
                    case 8:
                        content << "Load8(";
                        break;
                    case 16:
                        content << "Load16(";
                        break;
                    case 32:
                        content << "Load32(";
                        break;
                    default:
                        fprintf(stderr, "Unsupported load width %lu\n", bitWidth);
                        exit(EXIT_FAILURE);
                }
                content << getValue(ins.getOperand(0)) << ")";

                break;
            }

            case llvm::Instruction::Store: {
                auto& ins = llvm::cast<llvm::StoreInst>(baseIns);
                auto bitWidth = layout.getTypeSizeInBits(ins.getAccessType()).getFixedValue();
                switch (bitWidth) {
                    case 1:
                        content << "Store1(";
                        break;
                    case 8:
                        content << "Store8(";
                        break;
                    case 16:
                        content << "Store16(";
                        break;
                    case 32:
                        content << "Store32(";
                        break;
                    default:
                        fprintf(stderr, "Unsupported store width %lu\n", bitWidth);
                        exit(EXIT_FAILURE);
                }

                content << getValue(ins.getPointerOperand());
                content << ",";
                content << getValue(ins.getValueOperand());
                content << ")";

                break;
            }

            case llvm::Instruction::GetElementPtr: {
                auto& ins = llvm::cast<llvm::GetElementPtrInst>(baseIns);
                llvm::APInt constantOffset(32, 0);

                content << getValue(ins.getOperand(0));

                llvm::MapVector<llvm::Value *, llvm::APInt> offsets;
                if (!ins.collectOffset(layout, 32, offsets, constantOffset)) {
                    fprintf(stderr, "Failed to read GEP offset\n");
                    exit(EXIT_FAILURE);
                }

                for (auto& [val, scale] : offsets) {
                    auto scaleVal = scale.getSExtValue();
                    content << "+" << getValue(val);
                    if (scaleVal > 1) {
                        content << "*" << scaleVal;
                    } else if (scaleVal < 1) {
                        fprintf(stderr, "Negative offset not supported\n");
                        exit(EXIT_FAILURE);
                    }
                }

                auto c = constantOffset.getSExtValue();
                if (c > 0) {
                    content << "+" << c << "U";
                } else if (c < 0) {
                    content << "-" << -c << "U";
                }

                break;
            }

            case llvm::Instruction::Call: {
                auto& ins = llvm::cast<llvm::CallInst>(baseIns);
                auto needsSelf = false;

                if (auto f = ins.getCalledFunction()) {
                    auto name = f->getName();
                    if (name.starts_with("llvm.")) {
                        if (name.equals("llvm.va_start")) {
                            content << "v.Start";
                        } else if (name.equals("llvm.va_end")) {
                            content << "v.End";
                        } else if (name.starts_with("llvm.umin.")) {
                            auto rhs = getValue(ins.getArgOperand(1));
                            auto lhs = getValue(ins.getArgOperand(0));
                            content << lhs << "<" << rhs << "?" << lhs << ":" << rhs;
                            break;
                        } else if (name.starts_with("llvm.umax.")) {
                            auto lhs = getValue(ins.getArgOperand(0));
                            auto rhs = getValue(ins.getArgOperand(1));
                            content << lhs << ">" << rhs << "?" << lhs << ":" << rhs;
                            break;
                        } else if (name.starts_with("llvm.smin.")) {
                            auto sLhs = getSignedValue(ins.getArgOperand(0));
                            auto sRhs = getSignedValue(ins.getArgOperand(1));
                            auto lhs = getValue(ins.getArgOperand(0));
                            auto rhs = getValue(ins.getArgOperand(1));
                            content << sLhs << "<" << sRhs << "?" << lhs << ":" << rhs;
                            break;
                        } else if (name.starts_with("llvm.smax.")) {
                            auto sLhs = getSignedValue(ins.getArgOperand(0));
                            auto sRhs = getSignedValue(ins.getArgOperand(1));
                            auto lhs = getValue(ins.getArgOperand(0));
                            auto rhs = getValue(ins.getArgOperand(1));
                            content << sLhs << ">" << sRhs << "?" << lhs << ":" << rhs;
                            break;
                        } else if (name.starts_with("llvm.abs.")) {
                            auto a = getSignedValue(ins.getArgOperand(0));
                            content << "abs(" << a << ")";
                            break;
                        } else if (name.starts_with("llvm.fshl.")) {
                            auto a = getValue(ins.getArgOperand(0));
                            auto b = getValue(ins.getArgOperand(1));
                            auto c = getValue(ins.getArgOperand(2));
                            content << "(" << a << "<<" << c << ")|(" << b << ">>(32-" << c << "))";
                            break;
                        } else if (name.starts_with("llvm.usub.sat.")) {
                            auto a = getValue(ins.getArgOperand(0));
                            auto b = getValue(ins.getArgOperand(1));
                            content << a << "<" << b << "?0:(" << a << "-" << b << ")";
                            break;
                        } else {
                            fprintf(stderr, "Unsupported intrinsic %s\n", name.data());
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        content << "func_" << name.str();
                    }
                } else {
                    content << memory.getFuncPtr(ins.getFunctionType(), getValue(ins.getCalledOperand()));
                    needsSelf = true;
                }

                content << "(";
                if (needsSelf) {
                    content << "self";
                }

                auto t = ins.getFunctionType();
                auto numParams = t->getNumParams();
                auto numArgs = ins.arg_size();

                unsigned int index;
                for (index = 0U; index < numParams; index++) {
                    if (needsSelf || index) content << ",";
                    content << getValue(ins.getArgOperand(index));
                }

                if (t->isVarArg()) {
                    if (needsSelf || index) content << ",";

                    content << "VAList.Create(self)";
                    for (; index < numArgs; index++) {
                        content << ".Add(" << getValue(ins.getArgOperand(index)) << ")";
                    }
                }

                content << ")";

                break;
            }

            case llvm::Instruction::ICmp: {
                auto& ins = llvm::cast<llvm::ICmpInst>(baseIns);
                auto predicate = ins.getPredicate();

                bool isSigned;
                switch (predicate) {
                    case llvm::ICmpInst::Predicate::ICMP_SLT:
                    case llvm::ICmpInst::Predicate::ICMP_SLE:
                    case llvm::ICmpInst::Predicate::ICMP_SGT:
                    case llvm::ICmpInst::Predicate::ICMP_SGE:
                        isSigned = true;
                        break;
                    default:
                        isSigned = false;
                        break;
                }

                if (isSigned) {
                    content << getSignedValue(ins.getOperand(0));
                } else {
                    content << getValue(ins.getOperand(0));
                }

                switch (predicate) {
                    case llvm::ICmpInst::Predicate::ICMP_EQ:
                        content << "==";
                        break;
                    case llvm::ICmpInst::Predicate::ICMP_NE:
                        content << "!=";
                        break;
                    case llvm::ICmpInst::Predicate::ICMP_ULT:
                    case llvm::ICmpInst::Predicate::ICMP_SLT:
                        content << "<";
                        break;
                    case llvm::ICmpInst::Predicate::ICMP_ULE:
                    case llvm::ICmpInst::Predicate::ICMP_SLE:
                        content << "<=";
                        break;
                    case llvm::ICmpInst::Predicate::ICMP_UGT:
                    case llvm::ICmpInst::Predicate::ICMP_SGT:
                        content << ">";
                        break;
                    case llvm::ICmpInst::Predicate::ICMP_UGE:
                    case llvm::ICmpInst::Predicate::ICMP_SGE:
                        content << ">=";
                        break;
                    default:
                        fprintf(stderr, "Unsupported comparison\n");
                        exit(EXIT_FAILURE);
                }

                if (isSigned) {
                    content << getSignedValue(ins.getOperand(1));
                } else {
                    content << getValue(ins.getOperand(1));
                }

                break;
            }

            case llvm::Instruction::ZExt:
            case llvm::Instruction::IntToPtr:
            case llvm::Instruction::Freeze: {
                content << getValue(baseIns.getOperand(0));
                break;
            }

            case llvm::Instruction::SExt: {
                content << getSignedValue(baseIns.getOperand(0));
                truncateValue(baseIns.getType());
                break;
            }

            case llvm::Instruction::Trunc:
            case llvm::Instruction::PtrToInt: {
                content << getValue(baseIns.getOperand(0));
                truncateValue(baseIns.getType());
                break;
            }

            case llvm::Instruction::Alloca: {
                auto& ins = llvm::cast<llvm::AllocaInst>(baseIns);
                auto size = ins.getAllocationSize(layout);
                if (size == std::nullopt) {
                    fprintf(stderr, "Bad alloca\n");
                }
                auto al = ins.getAlign();
                content << "Alloca(" << size.value() << "," << al.value() << ")";
                break;
            }

            case llvm::Instruction::PHI: {
                auto& ins = llvm::cast<llvm::PHINode>(baseIns);

                auto v = ins.incoming_values().begin();
                auto b = ins.blocks().begin();
                auto num = ins.getNumIncomingValues();

                for (auto i = 0U; i < num; i++) {
                    if (i) content << ":";

                    // turn last condition into default to make valid ternary chain
                    if (i + 1 != num) {
                        content << "lastLabel==" << blocks.at(*b++) << "?";
                    }

                    content << getValue(*v++);
                }

                break;
            }

            case llvm::Instruction::Select: {
                auto& ins = llvm::cast<llvm::SelectInst>(baseIns);

                content << getValue(ins.getCondition());
                content << "?";
                content << getValue(ins.getTrueValue());
                content << ":";
                content << getValue(ins.getFalseValue());

                break;
            }

            default:
                fprintf(stderr, "Unimplemented instruction\n");
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void FuncCompileCtx::truncateValue(llvm::Type *t) {
    auto bitWidth = layout.getTypeSizeInBits(t);
    if (bitWidth > 32) {
        fprintf(stderr, "Integer too wide\n");
        exit(EXIT_FAILURE);
    } else if (bitWidth < 32) {
        content << "&" << ((1 << bitWidth) - 1) << "U";
    }
}

void Function::compile(const llvm::Function& func, const llvm::DataLayout& layout, const GlobalMemory& memory) {
    FuncCompileCtx ctx(layout, memory);
    ctx.compile(func);

    content = std::move(ctx.content.str());
}

const std::string& Function::contents() const {
    return content;
}
