# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "Beam_Playground_autogen"
  "CMakeFiles\\Beam_Playground_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Beam_Playground_autogen.dir\\ParseCache.txt"
  )
endif()
