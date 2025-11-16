#pragma once

#include <entt.hpp>

#include "CatEngine/Renderer/EditorCamera.h"
#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Core/UUID.h"

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Scene/ContactListener.h"

class b2World;

namespace CatEngine
{

    class Entity;

    class Scene : public Asset
    {
    public:
        Scene();
        ~Scene();

		void CopyTo(Ref<Scene>& target);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DeleteEntity(Entity entity);
        
		void OnPauseStart();
		void OnPauseStop();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

        void OnUpdateEditor(Time ts, EditorCamera& camera);
        void OnUpdateMainCameraPreview(Time ts); // This is outside of the runtime, used for the camera preview
        void OnUpdateSimulation(Time time, EditorCamera& camera);
        void OnUpdateRuntime(Time ts);


        void ShutdownRuntime();

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

        bool IsRunning() { return m_IsRunning; }


        static AssetType GetStaticType() { return AssetType::Scene; }
        virtual AssetType GetType() const { return GetStaticType(); }

    private:
        template<typename T>
		    void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
        void OnPhysics2DUpdate(Time ts);
		void OnPhysics2DStop();

        void OnScriptStart();
    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        std::unordered_map<UUID, entt::entity> m_EntityMap;

        b2World* m_PhysicsWorld = nullptr;
        ContactListener m_ContactListenter;

		bool m_IsRunning = false;
		bool m_IsPaused = false;

        friend class Entity;
    };
}
