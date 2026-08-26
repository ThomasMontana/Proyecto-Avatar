find_package(Catch2 3 REQUIRED)

add_executable(avatar_tests
    test_paths.cpp
    test_dialog.cpp
)

target_link_libraries(avatar_tests PRIVATE
    avatar_core
    Catch2::Catch2WithMain
)

target_compile_definitions(avatar_tests PRIVATE
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

add_test(NAME avatar_tests COMMAND avatar_tests)
