#include <filesystem>

#include "CatEngine/Core/UUID.h"
#include "CatScriptCore.h"

#include "SourceFileCompiler.h"

#include "ScriptInstance.h"
#include "ScriptClass.h"

namespace CatEngine
{
    enum class CollisionType
    {
        None = 0,
        Begin, End,
    };

    class Scene;
    class Entity;

    using ScriptFieldMap = std::unordered_map<UUID, ScriptFieldInstance>;

    class ScriptEngine
    {
	public:
	    void Init();
		void Shutdown();

		bool LoadFileWatcher(const std::filesystem::path& filePath);
        void InitializeFileSystems();

		void SetSceneContext(Ref<Scene> scene);
		void OnRuntimeStop();

		void ReloadBinaries();

		bool ScriptClassExists(const std::string& fullClassName);
		void OnStartEntity(Entity e);
		void OnUpdateEntity(Entity e, float ts);

        void DispatchCollisionEvent(UUID entityA, UUID entityB, CollisionType type);

		Ref<Scene> GetSceneContext();
		Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

        const ScriptFieldMap& GetInitializedFields(const UUID& entityID) const;

		std::unordered_map<std::string, Ref<ScriptClass>>& GetScriptClasses();
		Ref<ScriptClass> GetScriptClass(const std::string& name);
		ScriptFieldMap& GetScriptFieldMap(Entity entity);
		ScriptFieldMap& GetScriptFieldMap(UUID entity);

        UUID GetUUIDFromStringHash(const std::string& nameSpace, const std::string& className, const std::string& fieldName);

        static ScriptEngine& GetMutable();
    private:

	private:
        friend class ScriptGlue;
        friend class Application;
        friend class Scene;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
	};
}
