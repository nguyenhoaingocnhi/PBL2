# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  [[CMakeFiles\PBL2App_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\PBL2App_autogen.dir\ParseCache.txt]]
  [[CMakeFiles\backend_tests_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\backend_tests_autogen.dir\ParseCache.txt]]
  [[CMakeFiles\tutorfinder_core_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\tutorfinder_core_autogen.dir\ParseCache.txt]]
  "PBL2App_autogen"
  "backend_tests_autogen"
  "tutorfinder_core_autogen"
  )
endif()
