add_executable(mul_client client.cpp)
add_executable(mul_verify verify.cpp)
target_link_libraries(mul_client tfhe++)
target_link_libraries(mul_verify tfhe++)
file(COPY src DESTINATION .)
file(COPY build.sbt DESTINATION .)
file(COPY build.ys DESTINATION .)
file(COPY test.bash DESTINATION .)
add_custom_command(TARGET mul_client PRE_BUILD COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && sbt run && yosys build.ys)