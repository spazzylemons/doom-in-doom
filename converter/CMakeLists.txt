cmake_minimum_required(VERSION 3.5)

set(TARGET converter)

project(${TARGET})

set(LLVM_CONFIG llvm-config)
macro(llvm_set var flag)
    set(result_code)
    execute_process(
        COMMAND ${LLVM_CONFIG} --${flag}
        RESULT_VARIABLE result_code
        OUTPUT_VARIABLE LLVM_${var}
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(result_code)
        error("Failed to execute ${LLVM_CONFIG}")
    endif()
endmacro()

llvm_set(CFLAGS cppflags)
llvm_set(CXXFLAGS cxxflags)
llvm_set(LIBS libs)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LLVM_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

add_executable(${TARGET}
    src/main.cpp)

target_link_libraries(${TARGET} ${LLVM_LIBS})

