#include <ostream>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>

#include "memory.hpp"

// Start storing data at address 4, so nothing is stored at NULL.
static constexpr uint32_t MEMORY_INITIAL_OFFSET = 4;

static constexpr uint32_t bitSizeToByteSize(uint32_t bitSize) {
    return (bitSize + 7) >> 3;
}

static uint32_t getTypeSize(llvm::Type *t, const llvm::DataLayout& layout) {
    return bitSizeToByteSize(layout.getTypeSizeInBits(t).getFixedValue());
}

void GlobalMemory::writeConstant(uint32_t offset, const llvm::Constant *c, const llvm::DataLayout& layout) {
    if (auto v = llvm::dyn_cast<llvm::ConstantInt>(c)) {
        auto intSize = getTypeSize(v->getType(), layout);
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
            auto elementOffset = bitSizeToByteSize(structLayout->getElementOffsetInBits(i).getFixedValue());
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
        }
    } else if (auto v = llvm::dyn_cast<llvm::ConstantArray>(c)) {
        auto arrayType = v->getType();
        auto arrayStride = getTypeSize(arrayType->getArrayElementType(), layout);
        auto numElements = arrayType->getNumElements();
        auto elementOffset = 0U;
        for (auto i = 0U; i < numElements; i++) {
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
            elementOffset += arrayStride;
        }
    } else if (auto v = llvm::dyn_cast<llvm::ConstantPointerNull>(c)) {
        writePtr(offset, 0);
    } else if (auto v = llvm::dyn_cast<llvm::Function>(c)) {
        // TODO give functions addresses.
        writePtr(offset, 0);
    } else if (auto v = llvm::dyn_cast<llvm::ConstantDataArray>(c)) {
        auto arrayType = v->getType();
        auto arrayStride = getTypeSize(arrayType->getArrayElementType(), layout);
        auto numElements = arrayType->getNumElements();
        auto elementOffset = 0U;
        for (auto i = 0U; i < numElements; i++) {
            writeConstant(offset + elementOffset, v->getAggregateElement(i), layout);
            elementOffset += arrayStride;
        }
    } else if (auto v = llvm::dyn_cast<llvm::GlobalVariable>(c)) {
        writePtr(offset, getAddress(v->getName().data()));
    } else if (auto v = llvm::dyn_cast<llvm::ConstantAggregateZero>(c)) {
        auto size = getTypeSize(v->getType(), layout);
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
        fprintf(stderr, "Constant type unsupported: ");
        c->print(llvm::errs());
        fprintf(stderr, "\n");

        exit(EXIT_FAILURE);
    }
}

void Section::registerGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout &layout) {
    auto& var = variables[global.getName().str()];
    var = address;
    address += bitSizeToByteSize(layout.getTypeSizeInBits(global.getInitializer()->getType()).getFixedValue());
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

bool Section::getAddress(const std::string& name, uint32_t& out) {
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

uint32_t GlobalMemory::getAddress(const std::string& name) {
    uint32_t result;
    if (data.getAddress(name, result)) {
        return result + MEMORY_INITIAL_OFFSET;
    }
    if (bss.getAddress(name, result)) {
        return result + + MEMORY_INITIAL_OFFSET + data.size();
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
