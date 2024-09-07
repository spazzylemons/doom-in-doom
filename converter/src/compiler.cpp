#include <fstream>

#include <llvm/IR/Module.h>

#include "compiler.hpp"

Compiler::Compiler(const char *outDir) : outDir(outDir) {}

void Compiler::compile(llvm::Module *m) {
    handleGlobals(m);
}

void Compiler::handleGlobals(llvm::Module *m) {
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
}
