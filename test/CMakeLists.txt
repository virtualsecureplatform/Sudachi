file(GLOB test_directories "*")

foreach(test_directory ${test_directories})
    add_subdirectory(test_directory)
endforeach(test_directory ${test_directories})
