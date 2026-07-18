cmake_minimum_required(VERSION 3.10)
project(AvatarTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(avatar_test
    main.cpp
    Avatar.cpp
)

target_link_libraries(avatar_test PRIVATE
    SDL3::SDL3
    $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image>,SDL3_image::SDL3_image,SDL3_image::SDL3_image-static>
    nlohmann_json::nlohmann_json
)

add_custom_command(TARGET avatar_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Avatar.png
        $<TARGET_FILE_DIR:avatar_test>/Avatar.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Avatar.json
        $<TARGET_FILE_DIR:avatar_test>/Avatar.json
)