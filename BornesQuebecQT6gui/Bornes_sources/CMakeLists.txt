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

if(BUILD_TESTING)
    include(FetchContent)

    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()