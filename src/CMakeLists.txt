file(GLOB sudachi_sources "*.cpp")

add_library(sudachi
  ${sudachi_sources}
  )
target_include_directories(sudachi
  PUBLIC ${PROJECT_SOURCE_DIR}/include 
  ${PROJECT_SOURCE_DIR}/thirdparties/TFHEpp/include
  ${PROJECT_SOURCE_DIR}/thirdparties/TFHEpp/thirdparties/randen
  ${PROJECT_SOURCE_DIR}/thirdparties/TFHEpp/thirdparties/cereal/include
  ${PROJECT_SOURCE_DIR}/thirdparties/TFHEpp/thirdparties/spqlios
  ${PROJECT_SOURCE_DIR}/thirdparties/taskflow
  ${PROJECT_SOURCE_DIR}/thirdparties/json/single_include
  ${PROJECT_SOURCE_DIR}/thirdparties/crc32
  )
target_link_libraries(sudachi INTERFACE tfhe++ Taskflow)