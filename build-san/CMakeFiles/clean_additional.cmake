# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "src/app/CMakeFiles/LonelyRunnerApp_autogen.dir/AutogenUsed.txt"
  "src/app/CMakeFiles/LonelyRunnerApp_autogen.dir/ParseCache.txt"
  "src/app/LonelyRunnerApp_autogen"
  )
endif()
