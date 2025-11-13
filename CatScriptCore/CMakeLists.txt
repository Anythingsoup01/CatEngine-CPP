set(SRCS
    "src/CatScriptCore/InternalCalls/InternalCalls.cpp"
    "src/CatScriptCore/BaseClasses/ScriptObject.cpp"
    "src/CatScriptCore/Scene/Component/Component.cpp"
    "src/CatScriptCore/Scene/Component/Transform.cpp"
    "src/CatScriptCore/Scene/Component/Object.cpp"
    "src/CatScriptCore/InternalCalls/InternalCalls.h"
    "src/CatScriptCore/Types/Types.h"
    "src/CatScriptCore/Types/Vector.h"
    "src/CatScriptCore/BaseClasses/ScriptObject.h"
    "src/CatScriptCore/ScriptIncludes.h"
    "src/CatScriptCore/Scene/Component/Object.h"
    "src/CatScriptCore/Scene/Component/Component.h"
    "src/CatScriptCore/Scene/Component/Transform.h"
    "src/CatScriptCore/Scene/Component/Tag.h"
    "src/CatScriptCore/Scene/Component/Layer.h"
)
add_library(CatScriptCore SHARED ${SRCS})
set_property(TARGET CatScriptCore PROPERTY CXX_STANDARD 20)

target_include_directories(CatScriptCore PRIVATE
    "src"
)

target_compile_options(CatScriptCore PRIVATE
    -fPIC
    -gdwarf-4
)

