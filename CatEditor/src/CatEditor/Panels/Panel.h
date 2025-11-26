#pragma once

namespace CatEngine
{

    enum class PanelType
    {
        None = 0, // Error
        SceneHierarchy, ContentBrowser, AssetBrowser, ViewportPanel,
    };

    class Panel
    {
    public:
        virtual ~Panel() = default;

        virtual void OnImGuiRender() = 0;
        virtual void SetContext(const Ref<Scene>& context) {};

        Ref<Panel> Create(const PanelType& panelType);
    };
}
