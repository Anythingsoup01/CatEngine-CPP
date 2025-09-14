#pragma once

#include "CatEngine/Renderer/Texture.h"

#include <filesystem>

namespace CatEngine
{
	class AssetBrowserPanel
	{
	public:
		AssetBrowserPanel();

		void OnImGuiRender();

        void ResetAssetDirectory();

	private:
        static void RefreshAssetTree();
    private:

        struct TreeNode
        {
            std::filesystem::path Path;
            AssetHandle Handle = 0;

            std::unordered_map<std::filesystem::path, uint32_t> Children;
            uint32_t Parent = (uint32_t)-1;

            TreeNode(const std::filesystem::path& path, AssetHandle handle)
                : Path(path), Handle(handle) {}
        };
        static inline std::vector<TreeNode> m_TreeNodes;

        std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

        friend class ContentBrowserPanel;
	};
}
