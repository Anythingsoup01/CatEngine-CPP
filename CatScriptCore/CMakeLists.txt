set(SRCS
    "src/CatScriptCore/InternalCalls/InternalCalls.cpp"
    "src/CatScriptCore/BaseClasses/ScriptObject.cpp"
    "src/CatScriptCore/InternalCalls/InternalCalls.h"
    "src/CatScriptCore/Types/Types.h"
    "src/CatScriptCore/Types/Vector.h"
    "src/CatScriptCore/Scene/Component/Object.h"
    "src/CatScriptCore/Scene/Component/Component.h"
    "src/CatScriptCore/BaseClasses/ScriptObject.h"
)
add_library(CatScriptCore SHARED ${SRCS})
set_property(TARGET CatScriptCore PROPERTY CXX_STANDARD 20)

target_include_directories(CatScriptCore PRIVATE
    "src"
    "${CMAKE_SOURCE_DIR}/CatEngine/src"
    "${CMAKE_SOURCE_DIR}/vendor/spdlog/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/entt"
    "${CMAKE_SOURCE_DIR}/vendor/Box2D/include"
)

target_compile_options(CatScriptCore PRIVATE
    -fPIC
    -gdwarf-2
)

