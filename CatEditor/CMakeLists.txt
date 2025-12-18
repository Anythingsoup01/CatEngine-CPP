set(SRCS
    "src/ImGui/ImGuiDraw.cpp"
    "src/CatEditor/Entrypoint/CatEditorApp.cpp"
    "src/CatEditor/Panels/SceneHierarchyPanel.cpp"
    "src/CatEditor/Panels/SceneViewportPanel.cpp"
    "src/CatEditor/Panels/AssetBrowserPanel.cpp"
    "src/CatEditor/Panels/ContentBrowserPanel.cpp"
    "src/CatEditor/EditorLayer.cpp"
    "src/ImGui/ImGuiDraw.h"
    "src/CatEditor/UI/CatGui.h"
    "src/CatEditor/Panels/ContentBrowserPanel.h"
    "src/CatEditor/Panels/Panel.h"
    "src/CatEditor/Panels/SceneViewportPanel.h"
    "src/CatEditor/Panels/SceneHierarchyPanel.h"
    "src/CatEditor/Panels/AssetBrowserPanel.h"
    "src/CatEditor/EditorLayer.h"
)
add_executable(CatEditor ${SRCS})
set_property(TARGET CatEditor PROPERTY CXX_STANDARD 20)

target_include_directories(CatEditor PRIVATE
    "src"
    "${CMAKE_SOURCE_DIR}/CatEngine/src"
    "${CMAKE_SOURCE_DIR}/vendor/spdlog/include"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/entt"
    "${CMAKE_SOURCE_DIR}/vendor/Glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/NFD-E/src/include"
    "${CMAKE_SOURCE_DIR}/vendor/yaml/include"
    "${CMAKE_SOURCE_DIR}/vendor/ImGuizmo"
    "${CMAKE_SOURCE_DIR}/vendor/Box2D/include"
    "${CMAKE_SOURCE_DIR}/vendor/Capybara/include"
)

target_link_libraries(CatEditor
    CatEngine
)

target_compile_definitions(CatEditor PUBLIC
    GLFW_INCLUDE_NONE
    CE_DISPLAY_WAYLAND
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
target_compile_definitions(CatEditor
PUBLIC
    CE_DEBUG
)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)
if(CMAKE_BUILD_TYPE STREQUAL Release)
target_compile_definitions(CatEditor
PUBLIC
    CE_RELEASE
)
endif(CMAKE_BUILD_TYPE STREQUAL Release)
