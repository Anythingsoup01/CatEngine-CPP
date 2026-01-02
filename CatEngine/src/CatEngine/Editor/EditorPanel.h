#pragma once

#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Event/Event.h"


namespace CatEngine
{
    class EditorPanel
    {
    public:
        virtual ~EditorPanel() = default;

        virtual void OnImGuiRender(bool& isOpen) = 0;
        virtual void OnEvent(Event& e) {}
        virtual void SetSceneContext(const Ref<Scene>& context) {}
    };
}
