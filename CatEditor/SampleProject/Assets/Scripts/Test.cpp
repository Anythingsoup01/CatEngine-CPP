#include <CatEngine/Scripting/ScriptInclude.h>
using namespace CatEngine;

int m_TestInt = 0;

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
