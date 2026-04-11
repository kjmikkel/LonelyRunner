add_library(lonelyrunner STATIC
    geometric.cpp
    numerical.cpp
    prime.cpp
    prime_modular.cpp
    util.cpp
    range_test.cpp
)

target_include_directories(lonelyrunner
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/third_party
    PRIVATE ${NTL_INCLUDE_DIR}
)

target_link_libraries(lonelyrunner PUBLIC ${NTL_LIB} ${GMP_LIB} m)
target_compile_options(lonelyrunner PRIVATE -O2 -Wall -Wextra)
