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
            auto app = CreateApplication({argc, argv});
            app->Run();
            delete(app);
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return CatEngine::Main(argc, argv);
}
