set(SRCS
    src/CatEngine/ImGui/ImGuiBuild.cpp
    src/CatEngine/ImGui/ImGuiLayer.cpp
    src/CatEngine/Layer/Layer.cpp
    src/CatEngine/Layer/LayerStack.cpp
    src/CatEngine/Core/Window.cpp
    src/CatEngine/Core/Log.cpp
    src/CatEngine/Core/Application.cpp
    src/CatEngine.h
    src/cepch.h
    src/CatEngine/Event/Event.h
    src/CatEngine/Event/MouseEvent.h
    src/CatEngine/Event/KeyEvent.h
    src/CatEngine/Event/ApplicationEvent.h
    src/CatEngine/ImGui/ImGuiLayer.h
    src/CatEngine/Layer/LayerStack.h
    src/CatEngine/Layer/Layer.h
    src/CatEngine/Core/Entrypoint.h
    src/CatEngine/Core/Core.h
    src/CatEngine/Core/Log.h
    src/CatEngine/Core/MouseCodes.h
    src/CatEngine/Core/KeyCodes.h
    src/CatEngine/Core/Window.h
    src/CatEngine/Core/Application.h
)
add_library(CatEngine STATIC ${SRCS})
set_property(TARGET CatEngine PROPERTY CXX_STANDARD 20)

target_precompile_headers(CatEngine PUBLIC src/cepch.h)
target_include_directories(CatEngine PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/Glad/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
)

target_link_libraries(CatEngine
    ImGui
    glfw
    Glad
    GL
)

target_compile_definitions(CatEngine PUBLIC
    GLFW_INCLUDE_NONE
    CE_DISPLAY_WAYLAND
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
target_compile_definitions(CatEngine
PUBLIC
    CE_DEBUG
)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)
if(CMAKE_BUILD_TYPE STREQUAL Release)
target_compile_definitions(CatEngine
PUBLIC
    CE_RELEASE
)
endif(CMAKE_BUILD_TYPE STREQUAL Release)
