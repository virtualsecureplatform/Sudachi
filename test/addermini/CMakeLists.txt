get_filename_component(testname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" testname ${ProjectId})

file(GLOB resources RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.sbt")
file(GLOB resources RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.ys")
file(GLOB_RECURSE resources RELATIVE "${CMAKE_CURRENT_LIST_DIR}/src" "*")

add_executable(${test_name} client.cpp verify.cpp)
target_link_libraries(${test_name} tfhe++)
add_custom_command(TARGET ${test_name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resources} $<TARGET_FILE_DIR:${test_name}>)