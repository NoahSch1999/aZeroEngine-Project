#pragma once
#include "ECSBase.h"
#include "VertexBufferCache.h"
#include "Texture2DCache.h"
#include "MaterialManager.h"
#include "LightManager.h"
#include <fstream>
#include <iostream>
#include "RenderSystem.h"
#include "LightSystem.h"

/** @brief Describes a scene within the engine.
*/
class Scene
{
private:
	ECS* ecs;
	VertexBufferCache* vbCache;
	MaterialManager* mManager;
	Texture2DCache* textureCache;
	LightSystem* lSystem;
	ResourceEngine* resourceEngine = nullptr;

	std::string name = "";
	std::unordered_map<int, std::string> entityIdToName;
	std::unordered_map<std::string, int> entityNameToId;
	SlottedMap<Entity> entities;

public:

	Scene() = default;

	Scene(ECS* _ecs, VertexBufferCache* _vbCache, MaterialManager* _mManager, Texture2DCache* _textureCache, 
		LightSystem* _lSystem, ID3D12Device* _device, ResourceEngine* _resourceEngine)
		:ecs(_ecs), vbCache(_vbCache), textureCache(_textureCache), mManager(_mManager), lSystem(_lSystem), 
		resourceEngine(_resourceEngine) { }

	~Scene();

	Scene(Scene&& _other) noexcept;

	void Save(const std::string& _fileDirectory, const std::string& _fileName);

	// add constructor version
	bool Load(const std::string& _fileDirectory, const std::string& _fileName);

	SlottedMap<Entity>& GetEntityMap() { return entities; }

	int FindRec(int _num);

	std::string GetName() const { return name; }
	void SetName(const std::string& _name) { name = _name; }

	Entity& CreateEntity();
	Entity& CreateEntity(const std::string& _name);

	void DeleteEntity(const std::string& _name);
	void DeleteEntity(int _ID);

	Entity& GetEntity(const std::string& _name);
	Entity& GetEntity(int _ID);

	bool EntityExists(const std::string& _name) const;

	std::string GetEntityName(Entity _entity) const;
	void RenameEntity(const std::string& _oldName, const std::string& _newName);

	std::string CheckName(const std::string _name) const;

	/**Registers a default Mesh component for the specified Entity object and binds the Entity to the appropriate systems.
	@param _entityName The Entity to register a mesh component for
	@return void
	*/
	template<typename T>
	void AddComponentToEntity(Entity& _entity, const T& _data){ ecs->RegisterComponent(_entity, _data); }

	template<typename T>
	void AddComponentToEntity(Entity& _entity) { ecs->RegisterComponent<T>(_entity); }

	/**Removes a Mesh component for the specified Entity object and unbinds the Entity from the appropriate systems.
	@param _entityName The Entity to remove a Mesh component for
	@return void
	*/
	template<typename T>
	void RemoveComponentFromEntity(Entity& _entity)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			PointLightComponent* pComp = ecs->GetComponentManager().GetComponent<PointLightComponent>(_entity);
			if (pComp)
			{
				lSystem->RemoveLight(_entity);
			}
		}
		ecs->UnregisterComponent<T>(_entity);
	}

	/**
	* HAS TO BE DEFINED!
	*/
	template<typename T>
	void UpdateComponentForEntity(Entity _entity, const T& _data)
	{
		ecs->UpdateComponent(_entity, _data);

		// constexpr if T is pointlight (non-component) -> get component -> update with input _data
		// override transform "=" so that the id3d12resource doesnt get copied, but only the data
		// if constexpr T is transform, update the cb 
	}

	template<typename T>
	T* GetComponentForEntity(Entity _entity) { return ecs->GetComponent<T>(_entity); }
};