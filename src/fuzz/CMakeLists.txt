option(FUZZ "Build libFuzzer harness (requires clang)" OFF)

if(FUZZ)
    add_executable(fuzz_algorithms fuzz_algorithms.cpp)
    target_link_libraries(fuzz_algorithms PRIVATE lonelyrunner)
    target_compile_options(fuzz_algorithms PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer -g)
    target_link_options(fuzz_algorithms PRIVATE
        -fsanitize=fuzzer,address,undefined)
endif()
