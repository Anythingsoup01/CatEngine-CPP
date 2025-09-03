#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

Rigidbody2DComponent m_RB2D = Rigidbody2DComponent();

glm::vec2 m_Impulse = {1, 1};
glm::vec2 m_Point = {0, 0};

class PlayerController : public IScriptObject
{
public:
    void Start() override
    {
        m_RB2D = GetComponent<Rigidbody2DComponent>();
    }

    void Update(float ts) override
    {
        ApplyForce(m_RB2D, m_Impulse, m_Point, true);
    }
};
