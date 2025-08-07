#include "EditorLayer.h"
#include "imgui.h"

namespace CatEngine
{

    void EditorLayer::OnAttach()
    {
        s_Instance = this;
    }

    void EditorLayer::OnDetach()
    {
        s_Instance = nullptr;
    }

    void EditorLayer::OnUpdate(float ts)
    {

    }


    void EditorLayer::OnImGuiDraw()
    {
        ImGui::ShowDemoWindow();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);

    }
}
