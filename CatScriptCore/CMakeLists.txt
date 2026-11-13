add_library(CatScriptCore SHARED)

set_property(TARGET CatScriptCore PROPERTY CXX_STANDARD 20)
target_sources(CatScriptCore PRIVATE
src/CatScriptCore/InternalCalls/InternalCalls.cpp
src/CatScriptCore/Input/Input.cpp
src/CatScriptCore/Scene/Editor/Console.cpp
src/CatScriptCore/Scene/Component/Rigidbody2D.cpp
src/CatScriptCore/Scene/Component/SpriteRenderer.cpp
src/CatScriptCore/Scene/Component/Tag.cpp
src/CatScriptCore/Scene/Component/CircleRenderer.cpp
src/CatScriptCore/Scene/Component/CircleCollider2D.cpp
src/CatScriptCore/Scene/Component/Layer.cpp
src/CatScriptCore/Scene/Component/BoxCollider2D.cpp
src/CatScriptCore/Scene/Component/Camera.cpp
src/CatScriptCore/Scene/Component/Relationship.cpp
src/CatScriptCore/Scene/Component/AllComponents.cpp
src/CatScriptCore/Scene/Component/Transform.cpp
src/CatScriptCore/Scene/Component/Object.cpp
)

target_include_directories(CatScriptCore PRIVATE
src
)

target_compile_options(CatScriptCore PRIVATE
-fPIC
-gdwarf-4
-rdynamic
)



