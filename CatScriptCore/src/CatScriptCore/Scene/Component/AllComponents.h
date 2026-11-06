#pragma once

#include "CircleRenderer.h"
#include "SpriteRenderer.h"

#include "Transform.h"

#include "Camera.h"

#include "BoxCollider2D.h"
#include "CircleCollider2D.h"
#include "Rigidbody2D.h"

#include "Tag.h"
#include "Layer.h"

#include "Relationship.h"

#include "Script.h"

namespace CatRuntime {

#define ENGINE_COMPONENT_LIST(X) \
    X(Tag)                       \
    X(Layer)                     \
    X(Relationship)              \
    X(Transform)                 \
    X(Camera)                    \
    X(SpriteRenderer)            \
    X(CircleRenderer)            \
    X(BoxCollider2D)             \
    X(CircleCollider2D)          \
    X(Rigidbody2D)               \
    X(Script)

}
