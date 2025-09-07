#pragma once

#include "CatEngine/Renderer/Texture.h"

#include <filesystem>

namespace CatEngine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
        void CreateTemplateSourceFile();

        void ResetProjectDirectory();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
