#pragma once

#include "CatEngine/Renderer/Framebuffer.h"
#include "CatEngine/Scene/Scene.h"

namespace CatEngine
{
    class SceneViewportPanel
    {
    public:
        SceneViewportPanel() = default;
        SceneViewportPanel(const FramebufferSpecification& spec);

        void BindFramebuffer() { m_Framebuffer->Bind(); }
        void UnbindFramebuffer() { m_Framebuffer->Unbind(); }

        void ClearFramebufferAttachment(uint32_t attachmentIndex, int value) { m_Framebuffer->ClearAttachment(attachmentIndex, value); }

        void SetContext(const Ref<Scene>& scene) { m_Scene = scene; }

        void OnImGuiRender();

        bool FramebufferResized();
        
        void SetActive(bool active) { m_IsActive = active; }
        const bool IsActive() const { return m_IsActive; }

        const bool IsFocused() const { return m_PanelFocused; }
        const bool IsHovered() const { return m_PanelHovered; }

        uint32_t GetWidth() { return m_PanelWidth; }
        uint32_t GetHeight() { return m_PanelHeight; }

        int ReadPixelData(uint32_t attachmentIndex, int x, int y) { return m_Framebuffer->ReadPixel(attachmentIndex, x, y); }

        const glm::vec2* GetPanelBounds() { return m_PanelBounds; }
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_Scene;

        bool m_IsActive = true;

        bool m_PanelFocused, m_PanelHovered;

        uint32_t m_PanelWidth = 0, m_PanelHeight = 0;

        glm::vec2 m_PanelBounds[2];
    };
}
