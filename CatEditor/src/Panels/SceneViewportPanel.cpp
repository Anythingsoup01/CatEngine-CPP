#include "SceneViewportPanel.h"

#include <imgui.h>

namespace CatEngine
{
    SceneViewportPanel::SceneViewportPanel(const FramebufferSpecification& spec)
    {
        m_Framebuffer = Framebuffer::Create(spec);
    }

    bool SceneViewportPanel::FramebufferResized()
    {
        m_Scene->OnViewportResize(m_PanelWidth, m_PanelHeight);

		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_PanelWidth > 0 && m_PanelHeight > 0 && // zero sized framebuffer is invalid
			(spec.Width != m_PanelWidth || spec.Height != m_PanelHeight))
		{
			m_Framebuffer->Resize(m_PanelWidth, m_PanelHeight);
		    return true;
        }

        return false;
    }


    void SceneViewportPanel::OnImGuiRender()
    {
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_PanelBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_PanelBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };


		m_PanelFocused = ImGui::IsWindowFocused();
		m_PanelHovered = ImGui::IsWindowHovered();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_PanelWidth = (uint32_t)viewportPanelSize.x;
        m_PanelHeight = (uint32_t)viewportPanelSize.y;

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
		ImGui::Image((void*)(uint64_t)textureID, { (float)m_PanelWidth, (float)m_PanelHeight }, ImVec2(0, 1), ImVec2(1, 0));
    }
}
