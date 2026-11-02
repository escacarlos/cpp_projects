cmake_minimum_required(VERSION 3.14)
project(dictionnaireSynonymes LANGUAGES CXX)

include(CTest)

configure_file(PetitDico.txt PetitDico.txt COPYONLY)

add_library(DicoSynonymes DicoSynonymes.cpp)
target_include_directories(DicoSynonymes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(DicoSynonymes PUBLIC cxx_std_14)

add_executable(${PROJECT_NAME} Principal.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE DicoSynonymes)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(DicoSyn_tests testDico.cpp)
    target_link_libraries(DicoSyn_tests PRIVATE DicoSynonymes GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(DicoSyn_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
