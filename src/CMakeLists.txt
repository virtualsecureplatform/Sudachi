file(GLOB sudachi_sources "*.cpp")

add_executable(sudachi
  ${sudachi_sources}
  )

target_link_libraries(sudachi tfhe++ Threads::Threads)
target_include_directories(sudachi
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/thirdparties/taskflow
  ${PROJECT_SOURCE_DIR}/thirdparties/json/single_include
  ${PROJECT_SOURCE_DIR}/thirdparties/crc32
  )
