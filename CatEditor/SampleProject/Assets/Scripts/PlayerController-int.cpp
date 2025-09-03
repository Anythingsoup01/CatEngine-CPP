#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

extern "C" Rigidbody2DComponent m_RB2D = Rigidbody2DComponent();

extern "C" glm::vec2 m_Impulse = {1, 1};
extern "C" glm::vec2 m_Point = {0, 0};

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
extern "C" CatEngine::IScriptObject* create() { return new PlayerController; }
extern "C" void destroy(CatEngine::IScriptObject* script) { delete script; }