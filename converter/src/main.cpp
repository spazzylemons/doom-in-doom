#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

static void compileToIr(const char *filename) {
    // Create a file to store the data in.
    // Since it's a memfile, it'll automatically be closed when exit() is called.
    int fd = memfd_create("llvm-ir", 0);
    if (fd < 0) {
        perror("memfd_create");
        exit(EXIT_FAILURE);
    }
    pid_t parent = getpid();

    char outname[31];
    snprintf(outname, sizeof(outname), "/proc/%d/fd/%d", parent, fd);

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Run clang if child.
    if (child == 0) {
        execlp("clang", "clang", "-m32", "-S", "-emit-llvm", "-o", outname, filename, NULL);
        exit(EXIT_FAILURE);
    }

    // Check success of clang execution.
    int s;
    if (waitpid(child, &s, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    // Fail if child did not exit successfully.
    if (!WIFEXITED(s) || WEXITSTATUS(s)) {
        fprintf(stderr, "Failed to run clang\n");
        exit(EXIT_FAILURE);
    }

    // Open temp file as LLVM IR.
    llvm::SMDiagnostic err;
    llvm::LLVMContext ctx;

    auto module = llvm::parseIRFile(outname, err, ctx);
    // We're done with the memfile now, close it.
    close(fd);

    if (!module) {
        err.print(nullptr, llvm::errs());

        fprintf(stderr, "Failed to parse IR file\n");
        exit(EXIT_FAILURE);
    }

    // Perform work with the module here. We can't move the module out of the
    // lifetime of the memfile's filename.
    for (const auto &func : *module) {
        printf("%s\n", func.getName().str().c_str());
    }
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        compileToIr(argv[i]);
    }

    return EXIT_SUCCESS;
}


