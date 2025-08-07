#include "CatEngine/Core/Application.h"
#include "CatEngine/Core/Entrypoint.h"

#include "EditorLayer.h"

namespace CatEngine
{
    Application* CreateApplication(const ApplicationCommandLineArgs& args)
    {
        ApplicationSpecification spec;
        spec.Name = "CatEditor";
        spec.CommandlineArgs = args;

        Application* app = new Application(spec);
        app->PushLayer(new EditorLayer);

        return app;
    }
}
