add_executable(c3540_client client.cpp)
target_link_libraries(c3540_client tfhe++)
file(COPY verilog DESTINATION .)
file(COPY build.ys DESTINATION .)
file(COPY test.bash DESTINATION .)
add_custom_command(TARGET c3540_client PRE_BUILD COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && yosys build.ys)