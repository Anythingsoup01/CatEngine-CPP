#include "cepch.h"
#include "Components.h"



namespace CatEngine
{
#define REGISTER_COMPONENT_TYPE(TYPE) \
    template<> const char* ComponentToString<TYPE>() { return #TYPE; }

    REGISTER_COMPONENT_TYPE(TagComponent);
    REGISTER_COMPONENT_TYPE(LayerComponent);
    REGISTER_COMPONENT_TYPE(TransformComponent);
    REGISTER_COMPONENT_TYPE(CameraComponent);
    REGISTER_COMPONENT_TYPE(SpriteRendererComponent);
    REGISTER_COMPONENT_TYPE(CircleRendererComponent);
    REGISTER_COMPONENT_TYPE(BoxCollider2DComponent);
    REGISTER_COMPONENT_TYPE(CircleCollider2DComponent);
    REGISTER_COMPONENT_TYPE(Rigidbody2DComponent);
    REGISTER_COMPONENT_TYPE(ScriptComponent);

}
