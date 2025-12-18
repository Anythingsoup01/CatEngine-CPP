#include "cepch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components/Components.h"

#include <yaml-cpp/yaml.h>

#include "CatEngine/Project/Project.h"

#include "CatEngine/Core/Formatter.h"

#include "CatEngine/Scripting/ScriptEngine.h"


namespace CatEngine
{
    static std::string DataTypeToString(const DataType& type)
	{
        switch (type)
        {
            case DataType::Short: return "Short";
            case DataType::UShort: return "UShort";
            case DataType::Int: return "Int";
            case DataType::UInt: return "UInt";
            case DataType::Long: return "Long";
            case DataType::ULong: return "ULong";
            case DataType::Float: return "Float";
            case DataType::Double: return "Double";
            case DataType::Bool: return "Bool";
            case DataType::Vector2: return "Vector2";
            case DataType::Vector3: return "Vector3";
            case DataType::Vector4: return "Vector4";
            case DataType::Entity: return "Entity";
            case DataType::Texture2D: return "Texture2D";
            default: return "null";
        }
	}

    static DataType StringToDataType(const std::string& typeStr)
	{
            if (typeStr == "Short")             return DataType::Short;
            if (typeStr == "UShort")            return DataType::UShort;
            if (typeStr == "Int")               return DataType::Int;
            if (typeStr == "UInt")              return DataType::UInt;
            if (typeStr == "Long")              return DataType::Long;
            if (typeStr == "ULong")             return DataType::ULong;
            if (typeStr == "Float")             return DataType::Float;
            if (typeStr == "Double")            return DataType::Double;
            if (typeStr == "Bool")              return DataType::Bool;
            if (typeStr == "Vector2")           return DataType::Vector2;
            if (typeStr == "Vector3")           return DataType::Vector3;
            if (typeStr == "Vector4")           return DataType::Vector4;
            if (typeStr == "Entity")            return DataType::Entity;
            if (typeStr == "Texture2D")         return DataType::Texture2D;
            return DataType::Bool;
	}
	static std::string GetNamedComponents(YAML::Node& node, std::string component, std::string targetItem)
	{
		std::string entityComponent;
		auto entityNode = node[component];
		if (entityNode)
			entityComponent = entityNode[targetItem].as<std::string>();
		return entityComponent = "" ? std::string() : entityComponent;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}


	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		CE_PROFILE_FUNCTION();

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IDComponent>().ID;
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}
		if (entity.HasComponent<LayerComponent>())
		{
			out << YAML::Key << "LayerComponent";
			out << YAML::BeginMap; // LayerComponent

			auto& layer = entity.GetComponent<LayerComponent>().Layer;
			out << YAML::Key << "Layer" << YAML::Value << layer;

			out << YAML::EndMap; // LayerComponent
		}
		if (entity.HasComponent<NameComponent>())
		{
			out << YAML::Key << "NameComponent";
			out << YAML::BeginMap; // NameComponent

			auto& name = entity.GetComponent<NameComponent>().Name;
			out << YAML::Key << "Name" << YAML::Value << name;

			out << YAML::EndMap; // NameComponent
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.Position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& src = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << src.Color;
			out << YAML::Key << "Texture" << YAML::Value << src.Texture;
			out << YAML::Key << "Tiling" << YAML::Value << src.TilingFactor;
			out << YAML::EndMap; // SpriteRendererComponent
		}
        
		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& src = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << src.Color;
			out << YAML::Key << "Thickness" << YAML::Value << src.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << src.Fade;
			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFov();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent
			out << YAML::Key << "BodyType" << YAML::Value << (int)rb2d.Type;
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
			out << YAML::EndMap; // Rigidbody2DComponent

		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "BoxCollider2DComponent" << YAML::Value;
			out << YAML::BeginMap; // BoxCollider2DComponent
			out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
			out << YAML::Key << "Rotation" << YAML::Value << bc2d.Rotation;
			out << YAML::Key << "ShowColliderBounds" << YAML::Value << bc2d.ShowColliderBounds;
			out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.RestitutionThreshold;
			out << YAML::EndMap; // BoxCollider2DComponent

		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "CircleCollider2DComponent" << YAML::Value;
			out << YAML::BeginMap; // CircleCollider2DComponent
			out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
			out << YAML::Key << "ShowColliderBounds" << YAML::Value << cc2d.ShowColliderBounds;
			out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2d.RestitutionThreshold;
			out << YAML::EndMap; // CircleCollider2DComponent

		}
        
		if (entity.HasComponent<ScriptComponent>())
		{
			auto& sc = entity.GetComponent<ScriptComponent>();
            auto& scriptEngine = ScriptEngine::GetMutable();
			out << YAML::Key << "ScriptComponent" << YAML::Value;
			out << YAML::BeginMap; // ScriptComponent
            out << YAML::Key << "ScriptID" << YAML::Value << sc.ScriptID;
			out << YAML::Key << "ClassName" << YAML::Value << sc.ClassName;
            out << YAML::Key << "Fields" << YAML::Value;
            out << YAML::BeginSeq; // Edited fields
            const ScriptFieldMap& fieldMap = scriptEngine.GetInitializedFields(entity.GetUUID());
            for (auto& [uuid, field] : fieldMap)
            {
                out << YAML::BeginMap; // Field
                out << YAML::Key << "FieldID" << YAML::Value << uuid;
                out << YAML::Key << "FieldName" << YAML::Value << field.Field.Name;
                out << YAML::Key << "FieldType" << YAML::Value << DataTypeToString(field.Field.Type);
                out << YAML::Key << "Value";

                switch (field.Field.Type)
                {
                    case DataType::Short: out << YAML::Value << field.GetValue<int16_t>(); break;
                    case DataType::UShort: out << YAML::Value << field.GetValue<uint16_t>(); break;
                    case DataType::Int: out << YAML::Value << field.GetValue<int32_t>(); break;
                    case DataType::UInt: out << YAML::Value << field.GetValue<uint32_t>(); break;
                    case DataType::Long: out << YAML::Value << field.GetValue<int64_t>(); break;
                    case DataType::ULong: out << YAML::Value << field.GetValue<uint64_t>(); break;
                    case DataType::Float: out << YAML::Value << field.GetValue<float>(); break;
                    case DataType::Double: out << YAML::Value << field.GetValue<double>(); break;
                    case DataType::Bool: out << YAML::Value << field.GetValue<bool>(); break;
                    case DataType::Vector2: out << YAML::Value << field.GetValue<glm::vec2>(); break;
                    case DataType::Vector3: out << YAML::Value << field.GetValue<glm::vec3>(); break;
                    case DataType::Vector4: out << YAML::Value << field.GetValue<glm::vec4>(); break;
                    case DataType::Entity: 
                    case DataType::Texture2D: out << YAML::Value << field.GetValue<uint64_t>(); break;
                }
                out << YAML::EndMap; // Field
            }
            out << YAML::EndSeq; // Editied fields
			out << YAML::EndMap; // ScriptComponent

		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		CE_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "SampleScene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (auto entityID : m_Scene->GetReg().view<entt::entity>())
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		CE_PROFILE_FUNCTION();

		CE_API_ASSERT(false, "Not Implemented Yet");
	}
	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		CE_PROFILE_FUNCTION();

		YAML::Node data;
		try	{ data = YAML::LoadFile(filepath); }
		catch (YAML::ParserException ex)
		{
			CE_API_ERROR("Failed to load .catengine file '{0}'\n       {1}", filepath, ex.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				UUID uuid = entity["Entity"].as<UUID>();

				std::string name;
				auto nameComponent = entity["NameComponent"];
				if (nameComponent)
					name = nameComponent["Name"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				std::string tag;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					tag = tagComponent["Tag"].as<std::string>();
				deserializedEntity.GetComponent<TagComponent>().Tag = tag;

				std::string layer;
				auto layerComponent = entity["LayerComponent"];
				if (layerComponent)
					layer = layerComponent["Layer"].as<std::string>();
				deserializedEntity.GetComponent<LayerComponent>().Layer = layer;

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetOrAddComponent<TransformComponent>();
					tc.Position = transformComponent["Position"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    src.Texture = spriteRendererComponent["Texture"].as<AssetHandle>();
					src.TilingFactor = spriteRendererComponent["Tiling"].as<float>();
				}
                
				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<CircleRendererComponent>();
					src.Color = circleRendererComponent["Color"].as<glm::vec4>();
					src.Thickness = circleRendererComponent["Thickness"].as<float>();
					src.Fade = circleRendererComponent["Fade"].as<float>();
				}
                
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFov(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto rigidBody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidBody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = (Rigidbody2DComponent::BodyType)rigidBody2DComponent["BodyType"].as<int>();
					rb2d.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2D = entity["BoxCollider2DComponent"];
				if (boxCollider2D)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();

					bc2d.Offset = boxCollider2D["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2D["Size"].as<glm::vec2>();
					bc2d.Rotation = boxCollider2D["Rotation"].as<float>();
					bc2d.ShowColliderBounds = boxCollider2D["ShowColliderBounds"].as<bool>();
					bc2d.Density = boxCollider2D["Density"].as<float>();
					bc2d.Friction = boxCollider2D["Friction"].as<float>();
					bc2d.Restitution = boxCollider2D["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2D["RestitutionThreshold"].as<float>();

				}

				auto circleCollider2D = entity["CircleCollider2DComponent"];
				if (circleCollider2D)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();

					cc2d.Offset = circleCollider2D["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2D["Radius"].as<float>();
					cc2d.ShowColliderBounds = circleCollider2D["ShowColliderBounds"].as<bool>();
					cc2d.Density = circleCollider2D["Density"].as<float>();
					cc2d.Friction = circleCollider2D["Friction"].as<float>();
					cc2d.Restitution = circleCollider2D["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2D["RestitutionThreshold"].as<float>();

				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
                    auto scriptEngine = ScriptEngine::GetMutable();

                    auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                    sc.ScriptID = scriptComponent["ScriptID"].as<uint64_t>(0);
                    sc.ClassName = scriptComponent["ClassName"].as<std::string>();


                    auto fields = scriptComponent["Fields"];
                    for (auto field : fields)
                    {
                        UUID fieldID = field["FieldID"].as<UUID>();
                        ScriptFieldInstance& sfi = scriptEngine.GetScriptFieldMap(uuid)[fieldID];
                        sfi.Field.Name = field["FieldName"].as<std::string>();
                        sfi.Field.Type = StringToDataType(field["FieldType"].as<std::string>());

                        switch (sfi.Field.Type)
                        {
                            case DataType::Float: sfi.SetValue(field["Value"].as<float>()); break;
                            case DataType::Double: sfi.SetValue(field["Value"].as<double>()); break;
                            case DataType::Bool: sfi.SetValue(field["Value"].as<bool>()); break;
                            case DataType::Short: sfi.SetValue(field["Value"].as<int16_t>()); break;
                            case DataType::UShort: sfi.SetValue(field["Value"].as<uint16_t>()); break;
                            case DataType::Int: sfi.SetValue(field["Value"].as<int32_t>()); break;
                            case DataType::UInt: sfi.SetValue(field["Value"].as<uint32_t>()); break;
                            case DataType::Long: sfi.SetValue(field["Value"].as<int64_t>()); break;
                            case DataType::ULong: sfi.SetValue(field["Value"].as<uint64_t>()); break;
                            case DataType::Vector2: sfi.SetValue(field["Value"].as<glm::vec2>()); break;
                            case DataType::Vector3: sfi.SetValue(field["Value"].as<glm::vec3>()); break;
                            case DataType::Vector4: sfi.SetValue(field["Value"].as<glm::vec4>()); break;
                            case DataType::Texture2D:
                            case DataType::Entity: sfi.SetValue(field["Value"].as<uint64_t>()); break;
                            default: sfi.SetValue(0); break;
                        }

                    }
            
				}
			}
		}
		return true;
	}
	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		CE_PROFILE_FUNCTION();

		CE_API_ASSERT(false, "Not Implemented Yet");
		return false;
	}
}
