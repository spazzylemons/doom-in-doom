#include <cstdlib>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/TypeSize.h>
#include <ostream>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>

#include "memory.hpp"

std::string FuncPtrType::mapName() const {
    return std::string("fp") + (hasReturnValue ? "i" : "v") + std::to_string(numParams) + (isVarArg ? "v" : "");
}

FuncPtrType::FuncPtrType(const llvm::FunctionType *f)
    : hasReturnValue(f->getReturnType()->isSized())
    , isVarArg(f->isVarArg())
    , numParams(f->getNumParams()) {}

// Start storing data way past 0, for alignment and null-pointer reasons.
static constexpr uint32_t MEMORY_INITIAL_OFFSET = 1024;

void GlobalMemory::writeConstant(uint32_t offset, const llvm::Constant *c, const llvm::DataLayout& layout) {
    if (auto v = llvm::dyn_cast<llvm::ConstantInt>(c)) {
        auto intSize = layout.getTypeStoreSize(v->getType()).getFixedValue();
        if (intSize > 4) {
            fprintf(stderr, "Int too large\n");
            exit(EXIT_FAILURE);
        }
        auto intVal = v->getValue().getLimitedValue();
        writeInt(offset, intVal, intSize);
    } else if (auto v = llvm::dyn_cast<llvm::ConstantStruct>(c)) {
        auto structType = v->getType();
        auto structLayout = layout.getStructLayout(structType);
        auto numElements = structType->getNumElements();
        for (auto i = 0U; i < numElements; i++) {
            auto elementOffset = structLayout->getElementOffset(i).getFixedValue();
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
        }
    } else if (auto v = llvm::dyn_cast<llvm::ConstantArray>(c)) {
        auto arrayType = v->getType();
        auto arrayStride = layout.getTypeStoreSize(arrayType->getArrayElementType()).getFixedValue();
        auto numElements = arrayType->getNumElements();
        auto elementOffset = 0U;
        for (auto i = 0U; i < numElements; i++) {
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
            elementOffset += arrayStride;
        }
    } else if (auto v = llvm::dyn_cast<llvm::ConstantPointerNull>(c)) {
        writePtr(offset, 0);
    } else if (auto v = llvm::dyn_cast<llvm::Function>(c)) {
        writePtr(offset, getFuncIndex(v));
    } else if (auto v = llvm::dyn_cast<llvm::ConstantDataArray>(c)) {
        auto arrayType = v->getType();
        auto arrayStride = layout.getTypeStoreSize(arrayType->getArrayElementType()).getFixedValue();
        auto numElements = arrayType->getNumElements();
        auto elementOffset = 0U;
        for (auto i = 0U; i < numElements; i++) {
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
            elementOffset += arrayStride;
        }
    } else if (auto v = llvm::dyn_cast<llvm::GlobalVariable>(c)) {
        writePtr(offset, getAddress(v->getName().data()));
    } else if (auto v = llvm::dyn_cast<llvm::ConstantAggregateZero>(c)) {
        auto size = layout.getTypeStoreSize(v->getType()).getFixedValue();
        for (auto i = 0U; i < size; i++) {
            writeByte(offset++, 0);
        }
    } else if (auto v = llvm::dyn_cast<llvm::ConstantExpr>(c)) {
        auto ins = llvm::cast<llvm::GetElementPtrInst>(v->getAsInstruction());
        llvm::APInt elementOffset(32, 0);
        assert(ins->accumulateConstantOffset(layout, elementOffset) && "Global GEP is not constant");

        auto ptrName = ins->getOperand(0)->getName().str();
        auto ptrOffset = getAddress(ptrName);
        ins->deleteValue();

        auto finalOffset = ptrOffset + elementOffset.getLimitedValue();
        writePtr(offset, finalOffset);
    } else {
        llvm::errs() << "Constant type unsupported: ";
        c->print(llvm::errs());
        llvm::errs() << "\n";

        exit(EXIT_FAILURE);
    }
}

void Section::registerGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout &layout) {
    auto t = global.getInitializer()->getType();

    // Align variable.
    auto alignment = layout.getPrefTypeAlign(t).value();
    address = llvm::alignTo(address, alignment);
    if (alignment > maxAlign)
        maxAlign = alignment;

    auto& var = variables[global.getName().str()];
    var = address;

    // Add size of type.
    address += layout.getTypeStoreSize(t);
}

void GlobalMemory::registerFunction(const llvm::Function *f) {
    FuncPtrType fp(f->getFunctionType());
    auto& list = functionPtrMaps[fp];
    list[f->getName().str()] = funcPtrIndex++;
}

bool FuncPtrType::operator<(const FuncPtrType &other) const {
    if (hasReturnValue < other.hasReturnValue) {
        return true;
    } else if (hasReturnValue > other.hasReturnValue) {
        return false;
    }

    if (isVarArg < other.isVarArg) {
        return true;
    } else if (isVarArg > other.isVarArg) {
        return false;
    }

    if (numParams < other.numParams) {
        return true;
    }
    return false;
}

void GlobalMemory::registerGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout &layout) {
    Section *section;
    if (global.getInitializer()->isNullValue()) {
        // If global is zero-initialized, then store it in the bss section.
        section = &bss;
    } else {
        section = &data;
    }
    section->registerGlobal(global, layout);
}

void GlobalMemory::initializeGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout &layout) {
    auto init = global.getInitializer();
    if (init->isNullValue()) {
        // Variable is in BSS.
        return;
    }

    auto offset = getAddress(global.getName().str());
    writeConstant(offset, init, layout);
}

bool Section::getAddress(const std::string& name, uint32_t& out) const {
    auto it = variables.find(name);
    if (it == variables.end()) {
        return false;
    }
    out = it->second;
    return true;
}

void GlobalMemory::writeByte(uint32_t addr, uint8_t value) {
    // Remove offset when writing.
    addr -= MEMORY_INITIAL_OFFSET;

    assert(addr <= size && "Out-of-bounds write to global memory");
    memory[addr] = value;
}

void GlobalMemory::writeInt(uint32_t addr, uint32_t value, uint8_t byteSize) {
    while (byteSize > 0) {
        writeByte(addr++, value & 0xff);
        value >>= 8;
        byteSize--;
    }
}

void Section::align() {
    address = llvm::alignTo(address, maxAlign);
}

void GlobalMemory::align() {
    bss.align();
    data.align();
}

uint32_t GlobalMemory::getAddress(const std::string& name) const {
    uint32_t result;
    if (data.getAddress(name, result)) {
        return result + MEMORY_INITIAL_OFFSET;
    }
    if (bss.getAddress(name, result)) {
        return result + MEMORY_INITIAL_OFFSET + data.size();
    }

    fprintf(stderr, "Missing symbol '%s'\n", name.c_str());
    exit(EXIT_FAILURE);
}

void GlobalMemory::allocateMemory() {
    size = data.size();
    memory = std::make_unique<uint8_t[]>(size);
}

void GlobalMemory::saveMemory(std::ostream& out) {
    out.write(reinterpret_cast<const char *>(memory.get()), size);
}

void GlobalMemory::writeFunctionMaps(std::ostream& out) {
    for (const auto& [fp, _] : functionPtrMaps) {
        out << "Map<uint,function<play ";
        if (fp.hasReturnValue) {
            out << "uint";
        } else {
            out << "void";
        }
        out << "(DoomInDoom";
        for (auto i = 0U; i < fp.numParams; i++) {
            out << ",uint";
        }
        if (fp.isVarArg) {
            out << ",VAList";
        }
        out << ")> >";
        out << fp.mapName();
        out << ";\n";
    }

    out << "void loadFuncPtrs(){\n";
    for (const auto& [fp, m] : functionPtrMaps) {
        auto name = fp.mapName();
        for (const auto& [c, idx] : m) {
            out << name << ".Insert(" << idx << ",func_" << c << ");\n";
        }
    }
    out << "}\n";
}

std::string GlobalMemory::getFuncPtr(const llvm::FunctionType *f, std::string idx) const {
    FuncPtrType fp(f);
    auto& list = functionPtrMaps.at(fp);

    return fp.mapName() + ".Get(" + idx + ").Call";
}

uint32_t GlobalMemory::getFuncIndex(const llvm::Function *f) const {
    FuncPtrType fp(f->getFunctionType());
    auto& list = functionPtrMaps.at(fp);
    return list.at(f->getName().str());
}
