#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>

#include "compiler.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <bitcode file> <output dir>", argv[0]);
        return EXIT_FAILURE;
    }

    auto tryBuf = llvm::MemoryBuffer::getFile(argv[1]);
    if (auto ec = tryBuf.getError()) {
        auto msg = ec.message();
        fprintf(stderr, "Failed to read %s: %s\n", argv[1], msg.c_str());
        exit(EXIT_FAILURE);
    }
    auto buf = tryBuf->get();

    llvm::LLVMContext ctx;
    auto tryModule = llvm::parseBitcodeFile(buf->getMemBufferRef(), ctx);
    if (!tryModule) {
        auto msg = llvm::toString(tryModule.takeError());
        fprintf(stderr, "Failed to parse %s: %s\n", argv[1], msg.c_str());
        exit(EXIT_FAILURE);
    }
    auto module = tryModule->get();

    Compiler compiler(module, argv[2]);
    compiler.compile();
    compiler.write();

    return EXIT_SUCCESS;
}


