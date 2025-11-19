#pragma once

#include "CatEngine/Renderer/Texture.h"

#include "CatEngine/Scene/Scene.h"

#include <filesystem>

namespace CatEngine
{
	class AssetBrowserPanel
	{
	public:
		AssetBrowserPanel();

		void OnImGuiRender();

        void ResetAssetDirectory();

        void SetSelectionHandle(const AssetHandle& handle = 0) { m_SelectionContext = handle; }

	private:
        static void RefreshAssetTree();

        void DrawAsset(const AssetHandle& handle);
    private:
        static inline AssetBrowserPanel* s_Instance = nullptr;

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

        AssetHandle m_SelectionContext = 0;
        AssetHandle m_PreviousSelection = 0;

        friend class ContentBrowserPanel;
	};
}
