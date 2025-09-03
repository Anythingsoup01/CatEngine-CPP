#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

int m_Test = 0;

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
