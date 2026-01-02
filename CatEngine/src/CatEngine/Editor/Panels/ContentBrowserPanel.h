#pragma once

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
	};
}
