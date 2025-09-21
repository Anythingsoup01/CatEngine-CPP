#include <filesystem>

#include "CatEngine/Core/UUID.h"
#include "CatScriptCore.h"

#include "SourceFileCompiler.h"

#include "ScriptClass.h"
#include "ScriptInstance.h"

namespace CatEngine
{

    enum class CollisionType
    {
        None = 0,
        Begin, End,
    };

    class Scene;
    class Entity;

    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

    class ScriptEngine
    {
	public:
		static void Init();
		static void Shutdown();

		static bool LoadFileWatcher(const std::filesystem::path& filePath);

		static void SetSceneContext(Ref<Scene> scene);
		static void OnRuntimeStop();

		static void ReloadBinaries();

		static bool ScriptClassExists(const std::string& fullClassName);
		static void OnStartEntity(Entity e);
		static void OnUpdateEntity(Entity e, float ts);

        static void DispatchCollisionEvent(UUID entityA, UUID entityB, CollisionType type);

		static Ref<Scene> GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static CatScriptObject* GetManagedInstance(UUID uuid);

		static std::unordered_map<std::string, Ref<ScriptClass>>& GetScriptClasses();
		static Ref<ScriptClass> GetScriptClass(const std::string& name);
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

	private:
		friend class ScriptClass;
	};
}
