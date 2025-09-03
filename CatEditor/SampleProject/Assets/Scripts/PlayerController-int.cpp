#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

extern "C" int m_Test = 0;

class PlayerController : public IScriptObject
{
public:
    void Start() override
    {
    }

    void Update(float ts) override
    {
        glm::vec3 pos = GetPosition();
        if (KeyPressed(KeyCode::I))
            pos.x += 1 * ts;

        SetPosition(pos);
        int i = 0;
    }
};
extern "C" CatEngine::IScriptObject* create() { return new PlayerController; }
extern "C" void destroy(CatEngine::IScriptObject* script) { delete script; }