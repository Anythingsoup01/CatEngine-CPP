#pragma once

extern CatEngine::Application* CatEngine::CreateApplication(const ApplicationCommandLineArgs& args);
bool g_ApplicationRunning = true;

namespace CatEngine
{
    int Main(int argc, char** argv)
    {
        Log::Init();
        while (g_ApplicationRunning)
        {
            CE_PROFILE_BEGIN_SESSION("Startup", "CatEngine-Startup.json");
            auto app = CreateApplication({argc, argv});
            CE_PROFILE_END_SESSION();

            CE_PROFILE_BEGIN_SESSION("Runtime", "CatEngine-Runtime.json");
            app->Run();
            CE_PROFILE_END_SESSION();

            CE_PROFILE_BEGIN_SESSION("Shutdown", "CatEngine-Shutdown.json");
            delete(app);
            CE_PROFILE_END_SESSION();
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return CatEngine::Main(argc, argv);
}
