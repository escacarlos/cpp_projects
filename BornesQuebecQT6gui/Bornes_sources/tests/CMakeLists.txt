# Add the test executable
add_executable(BorneTesteur BorneTesteur.cpp)
target_link_libraries(BorneTesteur PRIVATE BornesLibrary GTest::gtest_main)

add_executable(BorneFontaineTesteur BorneFontaineTesteur.cpp)
target_link_libraries(BorneFontaineTesteur PRIVATE BornesLibrary GTest::gtest_main)

add_executable(BorneStationnementTesteur BorneStationnementTesteur.cpp)
target_link_libraries(BorneStationnementTesteur PRIVATE BornesLibrary GTest::gtest_main)

add_executable(RegistreBorneTesteur RegistreBorneTesteur.cpp)
target_link_libraries(RegistreBorneTesteur PRIVATE BornesLibrary GTest::gtest_main)

# If you want to run the tests automatically after build
include(GoogleTest)
gtest_discover_tests(BorneTesteur)
gtest_discover_tests(BorneFontaineTesteur)
gtest_discover_tests(BorneStationnementTesteur)
gtest_discover_tests(RegistreBorneTesteur)