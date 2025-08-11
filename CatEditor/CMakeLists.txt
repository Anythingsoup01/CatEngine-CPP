set(SRCS
    src/CatEditorApp.cpp
    src/EditorLayer.cpp
    src/EditorLayer.h
)
add_executable(CatEditor ${SRCS})
set_property(TARGET CatEditor PROPERTY CXX_STANDARD 20)

target_include_directories(CatEditor PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/CatEngine/src
    ${CMAKE_SOURCE_DIR}/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/Glad/include
)

target_link_libraries(CatEditor
    CatEngine
)

target_compile_definitions(CatEditor PUBLIC
    GLFW_INCLUDE_NONE
    CE_DISPLAY_WAYLAND
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)
if(CMAKE_BUILD_TYPE STREQUAL Release)
target_compile_definitions(CatEditor
PUBLIC
    CE_RELEASE
)
endif(CMAKE_BUILD_TYPE STREQUAL Release)
