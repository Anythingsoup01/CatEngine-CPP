#pragma once

#include "CatEngine/Renderer/Texture.h"

#include "CatEngine/Editor/EditorPanel.h"

namespace CatEngine
{
	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel();

		virtual void OnImGuiRender(bool& isOpen) override;
        virtual void OnEvent(Event& e) override {}

        void ResetAssetDirectory();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
