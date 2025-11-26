#include "cepch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components/Components.h"

#include <yaml-cpp/yaml.h>

#include "CatEngine/Project/Project.h"

#include "CatEngine/Core/Formatter.h"


namespace CatEngine
{
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
			out << YAML::Key << "ScriptComponent" << YAML::Value;
			out << YAML::BeginMap; // ScriptComponent
            out << YAML::Key << "ScriptID" << YAML::Value << sc.ScriptID;
			out << YAML::Key << "ClassName" << YAML::Value << sc.ClassName;
            out << YAML::Key << "Fields" << YAML::Value;
            out << YAML::BeginMap; // Edited fields
            for (auto& [uuid, field] : sc.ScriptFields)
            {
                out << YAML::Key << "FieldID" << YAML::Value << uuid;
                out << YAML::Key << "FieldType" << YAML::Value << ScriptFieldTypeToString(field.FieldType);
                out << YAML::Key << "Value";
                switch (field.FieldType)
                {
                    case ScriptFieldType::Float: out << YAML::Value << field.f; break;
                    case ScriptFieldType::Double: out << YAML::Value << field.d; break;
                    case ScriptFieldType::Char: out << YAML::Value << field.c; break;
                    case ScriptFieldType::Boolean: out << YAML::Value << field.b; break;
                    case ScriptFieldType::Int16: out << YAML::Value << field.i16; break;
                    case ScriptFieldType::Int32: out << YAML::Value << field.i32; break;
                    case ScriptFieldType::Int64: out << YAML::Value << field.i64; break;
                    case ScriptFieldType::UInt16: out << YAML::Value << field.ui16; break;
                    case ScriptFieldType::UInt32: out << YAML::Value << field.ui32; break;
                    case ScriptFieldType::UInt64: out << YAML::Value << field.ui64; break;
                    case ScriptFieldType::Vector2: out << YAML::Value << field.v2; break;
                    case ScriptFieldType::Vector3: out << YAML::Value << field.v3; break;
                    case ScriptFieldType::Vector4: out << YAML::Value << field.v4; break;
                    case ScriptFieldType::Texture2D:
                    case ScriptFieldType::TransformComponent:
                    case ScriptFieldType::Rigidbody2DComponent:
                    case ScriptFieldType::SpriteRenderer: out << YAML::Value << field.id.uuid(); break;
                }
            }
            out << YAML::EndMap; // Editied fields
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
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();

					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

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
