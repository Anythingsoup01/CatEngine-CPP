#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

extern "C" int m_TestInt = 0;

class Test : public IScriptObject
{
public:
    void Start() override
    {
        //Put here to start at runtime
    }

    void Update(float ts) override
    {
        //Put here to run during runtime
    }
};
extern "C" CatEngine::IScriptObject* create() { return new Test; }
extern "C" void destroy(CatEngine::IScriptObject* script) { delete script; }