#pragma once

#include "CatEngine/Event/Event.h"

namespace CatEngine
{
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(float ts) = 0;
        virtual void OnImGuiDraw() = 0;
        virtual void OnEvent(Event& e) = 0;

        inline const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };
}
