cmake_minimum_required(VERSION 4.2.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(CatEngine LANGUAGES C CXX)
add_compile_options(
    -fPIC
    -g
)

add_subdirectory(CatEngine)
add_subdirectory(CatEditor)
add_subdirectory(CatScriptCore)
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/NFD-E)
add_subdirectory(vendor/yaml)
add_subdirectory(vendor/Box2D)
add_subdirectory(vendor/Capybara)
