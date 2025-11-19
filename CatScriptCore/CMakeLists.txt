set(SRCS
    "src/CatScriptCore/Input/Input.cpp"
    "src/CatScriptCore/InternalCalls/InternalCalls.cpp"
    "src/CatScriptCore/Scene/Component/Rigidbody2D.cpp"
    "src/CatScriptCore/Scene/Component/Transform.cpp"
    "src/CatScriptCore/Scene/Component/SpriteRenderer.cpp"
    "src/CatScriptCore/Scene/Component/Tag.cpp"
    "src/CatScriptCore/Scene/Component/CircleRenderer.cpp"
    "src/CatScriptCore/Scene/Component/Layer.cpp"
    "src/CatScriptCore/Scene/Component/Object.cpp"
    "src/CatScriptCore/BaseClasses/ScriptObject.cpp"
    "src/CatScriptCore/Input/Input.h"
    "src/CatScriptCore/Input/KeyCodes.h"
    "src/CatScriptCore/InternalCalls/InternalCalls.h"
    "src/CatScriptCore/Types/Types.h"
    "src/CatScriptCore/Types/Vector.h"
    "src/CatScriptCore/Scene/Component/Object.h"
    "src/CatScriptCore/Scene/Component/Layer.h"
    "src/CatScriptCore/Scene/Component/InternalObjectData.h"
    "src/CatScriptCore/Scene/Component/Tag.h"
    "src/CatScriptCore/Scene/Component/Rigidbody2D.h"
    "src/CatScriptCore/Scene/Component/CircleRenderer.h"
    "src/CatScriptCore/Scene/Component/SpriteRenderer.h"
    "src/CatScriptCore/Scene/Component/Transform.h"
    "src/CatScriptCore/ScriptInclude.h"
    "src/CatScriptCore/BaseClasses/ScriptObject.h"
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

