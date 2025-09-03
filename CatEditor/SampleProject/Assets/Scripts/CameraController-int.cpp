#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

extern "C" float m_CameraSpeed = 1.0f;

class CameraController : public IScriptObject
{
public:
    void Start() override
    {
        
    }

    void Update(float ts) override
    {
        glm::vec3 pos = GetPosition();
        if (KeyPressed(KeyCode::W))
        {
            pos.y += m_CameraSpeed * ts;
        }  
        if (KeyPressed(KeyCode::S))
        {
            pos.y -= m_CameraSpeed * ts;
        }
        if (KeyPressed(KeyCode::A))
        {
            pos.x -= m_CameraSpeed * ts;
        }
        if (KeyPressed(KeyCode::D))
        {
            pos.x += m_CameraSpeed * ts;
        }

        SetPosition(pos);

    }
};
extern "C" CatEngine::IScriptObject* create() { return new CameraController; }
extern "C" void destroy(CatEngine::IScriptObject* script) { delete script; }