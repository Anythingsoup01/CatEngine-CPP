#pragma once

#include "Entity.h"

namespace CatEngine
{
	class ScriptObject
	{
	public:
		virtual ~ScriptObject() {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

        template<typename T, typename... Args>
        T& AddComponent(Args... args)
        {
            return m_Entity.AddComponent<T>(std::forward<Args>(args)...);
        }

	protected:
		virtual void OnStart() {}
		virtual void OnUpdate(Time time) {}
		virtual void OnDestory() {}
	private:
		Entity m_Entity;
		friend class Scene;
	};
}
