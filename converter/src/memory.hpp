#ifndef CONVERTER_MEMORY_H
#define CONVERTER_MEMORY_H

#include <map>
#include <memory>

namespace llvm {
    class Constant;
    class DataLayout;
    class Function;
    class FunctionType;
    class GlobalVariable;
}

struct GlobalMemory;

class Section {
    std::map<std::string, uint32_t> variables;
    uint32_t address = 0;
    uint32_t maxAlign = 1;

public:
    void registerGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout& layout);

    bool getAddress(const std::string& name, uint32_t& out) const;

    constexpr uint32_t size() const {
        return address;
    }

    void align();
};

struct FuncPtrType {
    bool hasReturnValue;
    bool isVarArg;
    uint8_t numParams;

    FuncPtrType(const llvm::FunctionType *f);

    std::string mapName() const;

    bool operator<(const FuncPtrType &other) const;
};

class GlobalMemory {
    Section bss;
    Section data;

    std::unique_ptr<uint8_t[]> memory = nullptr;
    uint32_t size = 0;

    std::map<FuncPtrType, std::map<std::string, uint32_t>> functionPtrMaps;
    uint32_t funcPtrIndex = 1;

public:
    // Write a byte to memory.
    void writeByte(uint32_t addr, uint8_t value);

    // Write an integer to memory.
    void writeInt(uint32_t addr, uint32_t value, uint8_t byteSize);

    // Write a pointer to memory.
    inline void writePtr(uint32_t addr, uint32_t value) { writeInt(addr, value, 4); }

    // Write an LLVM constant to memory.
    void writeConstant(uint32_t offset, const llvm::Constant *c, const llvm::DataLayout& layout);

    // Get the address of a global variable.
    uint32_t getAddress(const std::string& name) const;

    // Register a function.
    void registerFunction(const llvm::Function *f);

    // Register a global variable.
    void registerGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout& layout);

    // Initialize memory for a global variable.
    void initializeGlobal(const llvm::GlobalVariable& global, const llvm::DataLayout& layout);

    void align();

    // Allocate memory to store non-zero initialized globals.
    void allocateMemory();

    // Write memory to a file.
    void saveMemory(std::ostream& out);

    void writeFunctionMaps(std::ostream& out);

    std::string getFuncPtr(const llvm::FunctionType *f, std::string idx) const;

    uint32_t getFuncIndex(const llvm::Function *f) const;
};

#endif
