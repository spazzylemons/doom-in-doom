#ifndef CONVERTER_COMPILER_H
#define CONVERTER_COMPILER_H

#include "memory.hpp"

namespace llvm {
    class Module;
}

class Compiler {
    const char *outDir;

public:
    Compiler(const char *outDir);

    void compile(llvm::Module *m);

private:
    void handleGlobals(llvm::Module *m);

    GlobalMemory globalMemory;
};

#endif
