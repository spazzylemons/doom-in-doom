#ifndef CONVERTER_FUNCTION_H
#define CONVERTER_FUNCTION_H

#include <string>

namespace llvm {
    class DataLayout;
    class Function;
}

class GlobalMemory;

class Function {
    std::string content;

public:
    void compile(const llvm::Function& func, const llvm::DataLayout& layout, const GlobalMemory& memory);
    void debugPrint();

    const std::string& contents() const;
};

#endif
