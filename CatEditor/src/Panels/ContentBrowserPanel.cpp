#include "ContentBrowserPanel.h"
#include <imgui.h>

#include <iostream>
#include <string>

#include "CatEngine/Scripting/SourceFileCompiler.h"

#include "CatEngine/Core/System.h" 

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/AssetManager/TextureImporter.h"

#include "AssetBrowserPanel.h"

const size_t MAX_FILE_PATH_LEN = 4096;

enum class CreateType
{
    None = 0,
    Folder, SourceFile
};

CreateType type = CreateType::None;

#include "CatEngine/Project/Project.h"

namespace CatEngine
{

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(Project::GetAssetDirectory())
	{
        // TODO: Generate these as assets
        m_DirectoryIcon = TextureImporter::ImportIconTexture("Resources/Icons/DirectoryIcon.png");
		m_FileIcon = TextureImporter::ImportIconTexture("Resources/Icons/ScriptFileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("File Browser");

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
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

            if (directoryEntry.path().extension() == ".h") continue;

            if (strncmp(filenameString.c_str(), ".", 1) != 0)
            {
                ImGui::PushID(filenameString.c_str());
                Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
                ImGui::ImageButton("##ICON", (ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                if (ImGui::BeginDragDropSource())
                {
                    std::setlocale(LC_ALL, "");
                    auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

                    wchar_t itemPath[MAX_FILE_PATH_LEN] = { 0 };
                    const char* path = relativePath.c_str();
                    mbsrtowcs(itemPath, &path, MAX_FILE_PATH_LEN, NULL);
                    ImGui::SetDragDropPayload("FILE_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Load As Asset"))
                    {
                        std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
                        Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
                        AssetBrowserPanel::RefreshAssetTree();
                    }

                    ImGui::EndPopup();
                }

                if (ImGui::IsItemHovered())
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (directoryEntry.is_directory())
                            m_CurrentDirectory /= path.filename();

                        if (directoryEntry.is_regular_file())
                            System::OpenFile(m_CurrentDirectory / path.filename());

                    }
                }
                ImGui::TextWrapped("%s", filenameString.c_str());

                ImGui::NextColumn();
                ImGui::PopID();
            }
		}
        
        if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("New Source C++ File"))
				{
                    type = CreateType::SourceFile;
					ImGui::CloseCurrentPopup();
				}
                if (ImGui::MenuItem("Folder"))
                {
                    type = CreateType::Folder;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}


		ImGui::Columns(1);

		// TODO: status bar
		ImGui::End();

        switch (type)
        {
            case CreateType::Folder:
            {
                static char folderBuffer[1024] = { 'N', 'e', 'w', 'F', 'o', 'l', 'd', 'e', 'r' };
                ImGui::Begin("Create Folder");
                {
                    ImGui::InputText("File Name: ", folderBuffer, sizeof(folderBuffer));
                    if (ImGui::Button("Confirm"))
                    {
                        std::filesystem::create_directories(m_CurrentDirectory / folderBuffer);
                        type = CreateType::None;
                    }
                    ImGui::End();
                }
                break;
            }
            case CreateType::SourceFile:
            {
                static char buffer[1024] = { 'N', 'e', 'w', 'C', 'l', 'a', 's', 's' };
                ImGui::Begin("Create New Class");
                {
                    ImGui::InputText("File Name: ", buffer, sizeof(buffer));
                    if (ImGui::Button("Confirm"))
                    {
                        std::string filePath = m_CurrentDirectory.string().append("/");
                        filePath.append(buffer).append(".h");
                    
                        std::ofstream out(filePath);
                        if (!out.is_open())
                        {
                            CE_API_ERROR("Failed to generate file: {} !", filePath);
                            type = CreateType::None;
                            break;
                        }

                        std::stringstream content;

                        content
                            << "#pragma once\n"
                            << "#include \"CatScriptCore/ScriptInclude.h\"\n"
                            << "namespace CatRuntime\n"
                            << "{\n"
                            << "    class " << buffer << " : public ScriptObject\n"
                            << "    {\n"
                            << "    public:\n"
                            << "        " << buffer << "(uint64_t entityID) : ScriptObject(entityID) {}\n"
                            << "        virtual void Start() override;\n"
                            << "        virtual void Update(float ts) override;\n"
                            << "        static " << buffer << "* Create(uint64_t);\n"
                            << "    private:"
                            << "    // Be sure to put editor variables here, or in the public field!\n"
                            << "    };\n"
                            << "}";

                        out << content.rdbuf();

                        out.close();

                        filePath.erase(filePath.length() - 1);
                        filePath.append("cpp");
                        out.open(filePath.c_str());
                        if (!out.is_open())
                        {
                            CE_API_ERROR("Failed to generate file: {} !", filePath);
                            type = CreateType::None;
                            break;
                        }

                        content.str("");

                        content
                            << "#include \"" << buffer << ".h\"\n"
                            << "namespace CatRuntime\n"
                            << "{\n\n"
                            << "    void " << buffer << "::Start()\n"
                            << "    {\n"
                            << "        // Code placed here runs immediately after pressing play\n"
                            << "    }\n\n"
                            << "    void " << buffer <<"::Update(float ts)\n"
                            << "    {\n"
                            << "        // Code placed here runs every frame\n"
                            << "    }\n\n"
                            << "    " << buffer << "* " << buffer << "::Create(uint64_t entityID) { return new " << buffer << "(entityID); }\n\n"
                            << "}";

                        out << content.rdbuf();

                        out.close();

                        SourceFileCompiler::AddFile(filePath);

                        type = CreateType::None;

                    }
                    ImGui::End();
                    break;
                }
            }
            default: break;
        }

	}

    void ContentBrowserPanel::ResetAssetDirectory()
    {
        m_CurrentDirectory = Project::GetAssetDirectory();
    }

}
