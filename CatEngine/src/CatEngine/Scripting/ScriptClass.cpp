#include "cepch.h"
#include "ScriptClass.h"

#include "ScriptInstance.h"


#ifdef CE_PLATFORM_LINUX
#include <dlfcn.h>
#endif

namespace CatEngine
{
    ScriptClass::ScriptClass(CapyImage* image, const std::string& nameSpace, const std::string& className)
    {
        m_CapyClass = capy_class_from_name(image, nameSpace, className);
    }

    void* ScriptClass::Instantiate(UUID entityID)
    {
        CapyMethod* m = capy_method_from_class(m_CapyClass, "Create");
        if (!m)
        {
            CE_API_CRITICAL("NO CREATE FUNCTION FOUND!");
            return nullptr;
        }
        return capy_function_call_from_method(m, { entityID.uuid() });
    }

    CapyMethod* ScriptClass::GetMethod(const std::string& methodName)
    {
        return capy_method_from_class(m_CapyClass, methodName);
    }

    void* ScriptClass::InvokeMethod(CapyMethod* m, void* instance, const std::vector<RuntimeValue>& values)
    {
        std::vector<RuntimeValue> vals;
        vals = values;
        vals.insert(vals.begin(), instance);
        return capy_function_call_from_method(m, vals);
    }



}
