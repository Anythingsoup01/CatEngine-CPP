#pragma once

#include <entt.hpp>

#include "CatEngine/Renderer/EditorCamera.h"
#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Core/UUID.h"

namespace CatEngine
{

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);


		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DeleteEntity(Entity entity);
        
        void OnUpdateEditor(Time ts, EditorCamera& camera);
        void OnUpdateMainCameraPreview(Time ts); // This is outside of the runtime, used for the camera preview

        void OnSceneStart();
        void OnUpdateRuntime(Time ts);

        void OnViewportResize(uint32_t width, uint32_t height);

		void DuplicateEntity(Entity entity);
		Entity PasteEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID entityID);

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllComponentsWith()
		{
			return m_Registry.view<Components...>();
		}

        entt::registry& GetReg() { return m_Registry; }

    private:
        template<typename T>
		    void OnComponentAdded(Entity entity, T& component);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        std::unordered_map<UUID, entt::entity> m_EntityMap;

        friend class Entity;
    };
}
