# Add the library target
add_library(BornesLibrary #SHARED #if dynamic library
    Borne.cpp
    BorneFontaine.cpp
    BorneStationnement.cpp
    RegistreBorne.cpp
    ContratException.cpp
    ValidationFormat.cpp
)

# Include directories
target_include_directories(BornesLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# If you want to link against GTest for testing (gtest must be in the project)
#add_subdirectory(googletest)
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

enable_testing()

add_subdirectory(tests)