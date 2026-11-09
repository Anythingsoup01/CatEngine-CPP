add_executable(CatEditor)

set_property(TARGET CatEditor PROPERTY CXX_STANDARD 20)
target_sources(CatEditor PRIVATE
src/CatEditor/Entrypoint/CatEditorApp.cpp
src/CatEditor/Panels/SceneViewportPanel.cpp
src/CatEditor/EditorLayer.cpp
)

target_include_directories(CatEditor PRIVATE
src
../CatEngine/src
../vendor/spdlog/include
../vendor/imgui
../vendor/glm
../vendor/entt
../vendor/Glad/include
../vendor/NFD-E/src/include
../vendor/yaml/include
../vendor/ImGuizmo
../vendor/Box2D/include
../vendor/Capybara/Capybara/src
)

target_link_libraries(CatEditor
CatEngine
)

target_compile_definitions(CatEditor PUBLIC
GLFW_INCLUDE_NONE
CE_DISPLAY_WAYLAND
)



