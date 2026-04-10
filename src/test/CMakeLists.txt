add_executable(LonelyRunnerTests tests.cpp)
target_link_libraries(LonelyRunnerTests PRIVATE lonelyrunner)
add_test(NAME unit_tests COMMAND LonelyRunnerTests)
