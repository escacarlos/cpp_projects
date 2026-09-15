# Add the executable target
add_executable(bqc main.cpp)

# Link against the library
target_link_libraries(bqc BornesLibrary)