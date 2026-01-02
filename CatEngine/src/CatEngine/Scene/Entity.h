#pragma once

#include "Scene.h"
#include "Components/Components.h"

#include <entt.hpp>

namespace CatEngine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		entt::entity& GetEntityID() { return m_EntityHandle; }

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
            CE_API_ASSERT(!HasComponent<T>(), "component already exists!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		
		template<typename T>
		T& GetComponent()
		{
			CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetOrAddComponent()
		{
			if (HasComponent<T>())
			{
				return GetComponent<T>();
			}
			return AddComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
		}

		template<typename T>
		T& ResetComponent()
		{
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle);
			return component;
		}
        
        template<typename T, typename... Args>
		T& AddComponent(Args&&... args) const
		{
			CE_API_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) const 
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		
		template<typename T>
		T& GetComponent() const 
		{
			CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetOrAddComponent() const 
		{
			if (HasComponent<T>())
			{
				return GetComponent<T>();
			}
			return AddComponent<T>();
		}

		template<typename T>
		void RemoveComponent() const 
		{
			CE_API_ASSERT(HasComponent<T>(), "Entity does not have component!");

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const 
		{
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
		}

		template<typename T>
		T& ResetComponent() const 
		{
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle);
			return component;
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { static std::string nullScene = "<null scene>"; return m_Scene ? GetComponent<NameComponent>().Name : nullScene; }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }

        void SetIndex(uint32_t index) { m_HierarchyIndex = index; }
        const uint32_t GetIndex() const { return m_HierarchyIndex; }

	private:
		entt::entity m_EntityHandle{ entt::null };
        uint32_t m_HierarchyIndex = 0;
		Scene* m_Scene = nullptr;
	};
}
