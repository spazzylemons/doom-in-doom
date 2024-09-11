#include <cstdlib>
#include <fstream>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "compiler.hpp"
#include "function.hpp"

Compiler::Compiler(llvm::Module *m, const char *outDir) : m(m), outDir(outDir) {}

void Compiler::compile() {
    compileData();
    compileCode();
}

void Compiler::compileData() {
    const auto& layout = m->getDataLayout();

    // First, find where globals will live in memory.
    for (const auto& global : m->globals()) {
        globalMemory.registerGlobal(global, layout);
    }

    // Next, initialize the data.
    globalMemory.allocateMemory();
    for (const auto& global : m->globals()) {
        globalMemory.initializeGlobal(global, layout);
    }
}

void Compiler::compileCode() {
    const auto& layout = m->getDataLayout();

    for (auto& func : *m) {
        // Remove any 64-bit loads/stores from this function.
        remove64Bit(func);
        // Process any LLVM builtins.
        processBuiltins(func);
        // Make sure we didn't break it.
        llvm::verifyFunction(func);
    }

    for (const auto& func : *m) {
        for (auto user : func.users()) {
            bool isReferencedAsPtr = false;

            if (auto ins = llvm::dyn_cast<llvm::CallInst>(user)) {
                auto i = 0;
                for (const auto& arg : ins->operands()) {
                    if (++i == ins->getNumOperands() && llvm::isa<llvm::CallInst>(ins))
                        continue;

                    if (arg == &func) {
                        printf("A %s:\n", func.getName().data());
                        ins->print(llvm::outs());
                        llvm::outs() << "\n";

                        isReferencedAsPtr = true;
                        break;
                    }
                }
            } else {
                printf("B %s:\n", func.getName().data());
                user->print(llvm::outs());
                llvm::outs() << "\n";

                isReferencedAsPtr = true;
            }

            if (isReferencedAsPtr) {
                globalMemory.registerFunction(&func);
            }
        }
    }

    for (auto& func : *m) {
        // Skip any functions not defined.
        if (func.isDeclaration())
            continue;

        auto& f = functions.emplace_back();
        f.compile(func, layout, globalMemory);
    }
}

void Compiler::remove64Bit(llvm::Function& f) {
    auto& ctx = m->getContext();
    const auto& layout = m->getDataLayout();

    auto intType = llvm::IntegerType::get(ctx, 32);
    auto ptrType = llvm::PointerType::getUnqual(intType);

    std::array<llvm::Value *, 1> indices = { llvm::ConstantInt::get(intType, 1) };

    for (auto& block : f) {
        for (auto nextIns = &*block.begin(); nextIns != nullptr; ) {
            auto ins = nextIns;
            nextIns = ins->getNextNode();

            auto t = ins->getType();

            // Check if size is 64-bit.
            if (!t->isSized())
                continue;
            auto size = layout.getTypeSizeInBits(t).getFixedValue();
            if (size <= 32)
                continue;
            if (size != 64) {
                fprintf(stderr, "Strange integer size of %lu bits\n", size);
                exit(EXIT_FAILURE);
            }

            // We can only fix it if it's a load instruction.
            if (ins->getOpcode() != llvm::Instruction::Load) {
                fprintf(stderr, "64-bit non-load instruction\n");
                exit(EXIT_FAILURE);
            }

            auto srcPtrLo = ins->getOperand(0);
            auto srcPtrHi = llvm::GetElementPtrInst::Create(ptrType, srcPtrLo, indices);
            srcPtrHi->insertBefore(ins);

            auto loadLo = new llvm::LoadInst(intType, srcPtrLo, llvm::Twine(), ins);
            auto loadHi = new llvm::LoadInst(intType, srcPtrHi, llvm::Twine(), ins);

            auto users = ins->users();
            for (auto userIter = users.begin(); userIter != users.end(); ) {
                auto user = *userIter++;
                auto userIns = llvm::cast<llvm::Instruction>(user);

                auto dstPtrLo = user->getOperand(1);
                auto dstPtrHi = llvm::GetElementPtrInst::Create(ptrType, dstPtrLo, indices);
                dstPtrHi->insertBefore(userIns);

                auto storeLo = new llvm::StoreInst(loadLo, dstPtrLo, userIns);
                auto storedHi = new llvm::StoreInst(loadHi, dstPtrHi, userIns);

                if (userIns == nextIns) {
                    nextIns = nextIns->getNextNode();
                }
                userIns->eraseFromParent();
            }

            ins->eraseFromParent();
        }
    }
}

void Compiler::processBuiltins(llvm::Function& f) {
    auto& ctx = m->getContext();
    const auto& layout = m->getDataLayout();

    for (auto& block : f) {
        for (auto nextIns = &*block.begin(); nextIns != nullptr; ) {
            auto ins = nextIns;
            nextIns = ins->getNextNode();

            if (auto callIns = llvm::dyn_cast<llvm::CallInst>(ins)) {
                if (auto calledFunc = callIns->getCalledFunction()) {
                    auto name = calledFunc->getName();

                    if (name.starts_with("llvm.lifetime")) {
                        // Remove lifetime intrinsics.
                        ins->eraseFromParent();
                    } else if (name.starts_with("llvm.memset") || name.starts_with("llvm.memcpy") || name.starts_with("llvm.memmove")) {
                        llvm::Function *replacement;
                        if (name.starts_with("llvm.memset")) {
                            replacement = m->getFunction("memset");
                        } else if (name.starts_with("llvm.memcpy")) {
                            replacement = m->getFunction("memcpy");
                        } else if (name.starts_with("llvm.memmove")) {
                            replacement = m->getFunction("memmove");
                        } else {
                            fprintf(stderr, "Unexpected memory intrinsic %s\n", name.data());
                            exit(EXIT_FAILURE);
                        }

                        // Remove last argument
                        std::vector<llvm::Value *> newArgs;
                        newArgs.push_back(callIns->getArgOperand(0));
                        newArgs.push_back(callIns->getArgOperand(1));

                        // Convert 64-bit lengths to 32-bit.
                        if (auto lenArg = llvm::dyn_cast<llvm::ConstantInt>(callIns->getArgOperand(2))) {
                            const auto& lenValue = lenArg->getValue();

                            auto newLenValue = lenValue.trunc(32);
                            auto newLen = llvm::ConstantInt::get(ctx, newLenValue);
                            newArgs.push_back(newLen);
                        } else {
                            newArgs.push_back(callIns->getArgOperand(2));
                        }

                        auto newCall = llvm::CallInst::Create(
                            replacement->getFunctionType(),
                            replacement,
                            newArgs);

                        callIns->replaceAllUsesWith(newCall);
                        newCall->insertAfter(callIns);
                        callIns->eraseFromParent();
                    }
                }
            }
        }
    }
}

void Compiler::write() {
    std::ofstream dataFile;
    auto dataFileName = outDir + std::string("/data.bin");
    dataFile.open(dataFileName);
    if (dataFile.fail()) {
        fprintf(stderr, "Failed to open %s\n", dataFileName.c_str());
        exit(EXIT_FAILURE);
    }

    globalMemory.saveMemory(dataFile);
    dataFile.flush();
    dataFile.close();

    dataFileName = outDir + std::string("/code.zs");
    dataFile.open(dataFileName);
    if (dataFile.fail()) {
        fprintf(stderr, "Failed to open %s\n", dataFileName.c_str());
        exit(EXIT_FAILURE);
    }

    dataFile << "extend class DoomInDoom{\n";

    for (const auto &function : functions) {
        dataFile << function.contents();
    }

    globalMemory.writeFunctionMaps(dataFile);

    dataFile << "}\n";
    dataFile.flush();
    dataFile.close();
}
