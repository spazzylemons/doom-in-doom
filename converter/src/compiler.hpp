#ifndef CONVERTER_COMPILER_H
#define CONVERTER_COMPILER_H

#include <vector>

#include "function.hpp"
#include "memory.hpp"

namespace llvm {
    class Function;
    class Module;
}

class Compiler {
    llvm::Module *m;
    const char *outDir;

public:
    Compiler(llvm::Module *m, const char *outDir);

    void compile();
    void write();

private:
    void compileData();
    void compileCode();

    void remove64Bit(llvm::Function& f);
    void processBuiltins(llvm::Function& f);

    GlobalMemory globalMemory;
    std::vector<Function> functions;
};

#endif
