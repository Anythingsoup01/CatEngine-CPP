#include "cepch.h"
#include "AssetBrowserPanel.h"
#include <imgui.h>

#include <iostream>
#include <string>

#include "CatEngine/Core/System.h" 

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/AssetManager/TextureImporter.h"

#include "CatEngine/Core/Application.h"

#include "ImGui/ImGuiDraw.h"

const size_t MAX_FILE_PATH_LEN = 4096;

#include "CatEngine/Project/Project.h"

namespace CatEngine
{

	AssetBrowserPanel::AssetBrowserPanel()
        : m_CurrentDirectory(Project::GetAssetDirectory())
	{
        s_Instance = this;
        // TODO: Generate these as assets
        m_TreeNodes.push_back(TreeNode(".", 0));
        m_DirectoryIcon = TextureImporter::ImportIconTexture("Resources/Icons/DirectoryIcon.png");
		m_FileIcon = TextureImporter::ImportIconTexture("Resources/Icons/ScriptFileIcon.png");
	}

	void AssetBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Assets");
		if (m_CurrentDirectory != Project::GetAssetDirectory())
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}
		static float padding = 5.f;
		static float thumbnailSize = 75.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

        TreeNode* node = &m_TreeNodes[0];
        std::filesystem::path currentDir = std::filesystem::relative(m_CurrentDirectory, Project::GetAssetDirectory());
        for (const auto& p : currentDir)
        {
            // Single level directory
            if (node->Path == currentDir)
                break;

            if (node->Children.find(p) != node->Children.end())
            {
                node = &m_TreeNodes[node->Children[p]];
                continue;
            }

            CE_API_ASSERT(false, "Child Not Found!");

        }

        for (const auto& [item, treeNodeIndex] : node->Children)
        {
            bool isDirectory = std::filesystem::is_directory(Project::GetAssetDirectory() / item);
            bool isRegularFile = std::filesystem::is_regular_file(Project::GetAssetDirectory() / item);

            std::string itemStr = item.generic_string();

            ImGui::PushID(itemStr.c_str());
            Ref<Texture2D> fileIcon;
            AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;

            AssetType type;

            const Ref<Asset>& asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);
            if (asset)
            {
                type = asset->GetType();
            }



            if (type == AssetType::Texture2D && !item.extension().string().empty())
            {
                const auto& asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);
                fileIcon = (const Ref<Texture2D>&)asset;
                if (fileIcon == nullptr)
                {
                    CE_CLI_ERROR("Failed to load Texture2D!");
                }
            }

            fileIcon = fileIcon == nullptr ? m_FileIcon : fileIcon;
            Ref<Texture2D> icon = isDirectory ? m_DirectoryIcon : fileIcon;
            ImGui::ImageButton("##ICON", (ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            if (ImGui::BeginDragDropSource())
            {
                AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
                ImGui::SetDragDropPayload("ASSET_MANAGER_ITEM", &handle, sizeof(handle), ImGuiCond_Once);
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
                    Application::Get().SubmitToMainThread([handle, this]()
                    {
                        Project::GetActive()->GetEditorAssetManager()->DeleteAsset(handle);
                        RefreshAssetTree();
                    });
                }


                ImGui::EndPopup();
            }

            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (isDirectory)
                        m_CurrentDirectory /= item.filename();
                    else
                        m_SelectionContext = handle;
                    // TODO: Make this load the item from it's path instead, i.e textures will open the underlying image
                    // if (isRegularFile)
                    //     System::OpenFile(m_CurrentDirectory / item.filename());

                }
            }

            std::string fileName = item.string();
            size_t extensionLen = item.extension().string().length();
            fileName.erase(fileName.length() - extensionLen);
            ImGui::Text("%s", fileName.c_str());

            ImGui::NextColumn();
            ImGui::PopID();

        }

        ImGui::End();

        ImGui::Begin("AssetData");
		if (m_SelectionContext != 0)
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			DrawAsset(m_SelectionContext);
		}

		// TODO: status bar
		ImGui::End();


	}

    void AssetBrowserPanel::RefreshAssetTree()
    {
        const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
        

        for(const auto& [handle, metaData] : assetRegistry)
        {
            uint32_t currentNodeIndex = 0;
            for (const auto& p : metaData.FilePath)
            {
                auto it = m_TreeNodes[currentNodeIndex].Children.find(p.generic_string());
                if (it != m_TreeNodes[currentNodeIndex].Children.end())
                {
                    currentNodeIndex = it->second;
                }
                else
                {
                    TreeNode newNode(p, handle);
                    newNode.Parent = currentNodeIndex;
                    m_TreeNodes.push_back(newNode);

                    m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
                    currentNodeIndex = m_TreeNodes.size() - 1;

                }
            }
        }
    }

    static std::vector<const char*> s_TextureFormats = {
        "R8",
        "RGB8",
        "RGBA8",
    };

    static std::vector<const char*> s_TextureFilters = {
        "Linear",
        "Nearest"
    };

    static std::vector<const char*> s_TextureWrapFilters = {
        "Repeat",
        "Mirrored Repeat",
        "Clamp To Edge",
        "Clamp To Border",
    };

    void AssetBrowserPanel::DrawAsset(const AssetHandle& handle)
    {
        const auto& asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);

        switch (asset->GetType())
        {
            case AssetType::Scene:
            {
                Ref<Scene> sceneAsset = (Ref<Scene>&)asset;


                if (!ImGui::BeginTable("##SceneData", 2))
                    break;

                ImGui::TableSetupColumn("##INFOTAG", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("##INFODATA", ImGuiTableColumnFlags_WidthStretch);
                ImGui::EndTable();
                break;
            }

            case AssetType::Texture2D:
            {
                Ref<Texture2D> textureAsset = (Ref<Texture2D>&)asset;

                const Asset::MetaData& metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);

                ImGuiDraw::StaticString("Name", metaData.AssetName);
                ImGuiDraw::Combo("Min Filter", (int&)metaData.TextureMinFilter, s_TextureFilters);
                ImGuiDraw::Combo("Mag Filter", (int&)metaData.TextureMagFilter, s_TextureFilters);
                ImGuiDraw::Combo("Wrap Options", (int&)metaData.TextureWrap, s_TextureWrapFilters);
                ImGuiDraw::StaticString("Format", ImageFormatToString(metaData.TextureFormat));

                if (ImGui::Button("Apply"))
                {
                    TextureSpecification spec;
                    spec.MinFilter = metaData.TextureMinFilter;
                    spec.MagFilter = metaData.TextureMagFilter;
                    spec.WrapOption = metaData.TextureWrap;
                    spec.Format = metaData.TextureFormat;

                    Buffer data = TextureHelper::BufferTexture(Project::GetAssetDirectory() / metaData.FilePath, (int&)spec.Width, (int&)spec.Height);
                    textureAsset->RecreateTextureWithNewSpecification(spec, data);
                }
            }
        }
    }

    void AssetBrowserPanel::ResetAssetDirectory()
    {
        m_CurrentDirectory = Project::GetAssetDirectory();
        RefreshAssetTree();
    }
}
