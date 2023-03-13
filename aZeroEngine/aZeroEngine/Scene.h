#pragma once
#include "ECSBase.h"
#include "VertexBufferCache.h"
#include "Texture2DCache.h"
#include "MaterialManager.h"
#include "LightManager.h"
#include <fstream>
#include <iostream>
#include "RenderSystem.h"

// To Do:
/*
Remove pointers? send in as params instead? pros vs cons?

Make quick render system class and test to bind etc.

Add docs for this class.

Look for horrible code sections.

Create a save binary and/or asci method.

Create a load binary and/or asci method.
*/

/** @brief Describes a scene within the engine.
*/
class Scene
{
private:
	ECS& ecs;

	VertexBufferCache& vbCache;

	MaterialManager& mManager;

	Texture2DCache& textureCache;

	DescriptorManager& dManager;

	LightManager& lManager;

	std::unordered_map<int, std::string> entityIdToName;
	std::unordered_map<std::string, int> entityNameToId;

	std::string name = "";
public:
	SlottedMap<Entity> entities;

	EntityManager& GetEntityManager() { return ecs.GetEntityManager(); }
	VertexBufferCache& GetVertexBufferCache() { return vbCache; }
	MaterialManager& GetMaterialManager() { return mManager; }
	const std::string GetName() const { return name; }
	void SetName(const std::string& _name) { name = _name; }

	Scene() = default;

	/**Initiates the private member variables.
	@param _ecs Pointer to the ECS instance that should be used within its member functions
	@param _vbCache Pointer to the VertexBufferCache instance that should be used within its member functions
	*/
	Scene(ECS& _ecs, VertexBufferCache& _vbCache, MaterialManager& _mManager, DescriptorManager& _dManager, Texture2DCache& _textureCache, LightManager& _lManager)
		:ecs(_ecs), vbCache(_vbCache), textureCache(_textureCache), mManager(_mManager), dManager(_dManager), lManager(_lManager)
	{
	}

	/**Removes all the Entity objects within Scene::entities using ECS::ObliterateEntity()
	*/
	~Scene()
	{

		
	}

	void ShutDown(BasicRendererSystem& _basicRendererSystem, ShadowPassSystem& _shadowSystem, ResourceEngine& _resourceEngine, int _frameIndex)
	{
		for (auto& ent : entities.GetObjects())
		{
			PointLightComponent* plComp = ecs.GetComponentManager().GetComponent<PointLightComponent>(ent);
			if (plComp)
			{
				lManager.RemoveLight(*plComp, _frameIndex);
			}

			DirectionalLightComponent* dlComp = ecs.GetComponentManager().GetComponent<DirectionalLightComponent>(ent);
			if (dlComp)
			{
				lManager.RemoveLight(*dlComp, _frameIndex);
			}

			Transform* tf = ecs.GetComponentManager().GetComponent<Transform>(ent);
			if (tf)
			{
				_resourceEngine.RemoveResource(tf->GetBuffer());
			}

			_basicRendererSystem.UnBind(ent);
			_shadowSystem.UnBind(ent);

			ecs.ObliterateEntity(ent);
		}
	}

	void Save(const std::string& _fileDirectory, const std::string& _fileName, bool _debugASCII = false)
	{
		int size = (int)entities.GetObjects().size();

		if (size == 0)
			return;

		std::ofstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios_base::trunc | std::ios::out | std::ios::binary);

		bool yes = true;
		bool no = false;

		file.write((char*)&size, sizeof(int));
		for (auto& entity : entities.GetObjects())
		{
			//const Entity entity = entities.Get(id);
			const std::string name = entityIdToName.at(entity.id);
			Helper::WriteToFile(file, name);

			for (int i = 0; i < MAXCOMPONENTS; i++)
			{
				if (entity.componentMask.test(i))
				{
					file.write((char*)&yes, sizeof(bool));
				}
				else
				{
					file.write((char*)&no, sizeof(bool));
				}
			}

			Transform* tf = ecs.GetComponentManager().GetComponent<Transform>(entity);
			file.write((char*)&tf->GetTranslation(), sizeof(Vector3));
			file.write((char*)&tf->GetRotation(), sizeof(Vector3));
			file.write((char*)&tf->GetScale(), sizeof(Vector3));

			if (entity.componentMask.test(COMPONENTENUM::MESH))
			{
				Mesh* mesh = ecs.GetComponentManager().GetComponent<Mesh>(entity);
				std::string fileName = vbCache.GetResource(mesh->GetID())->GetFileName();
				Helper::WriteToFile(file, fileName);
				file.write((char*)&mesh->castShadows, sizeof(bool));
				file.write((char*)&mesh->receiveShadows, sizeof(float));
			}

			if (entity.componentMask.test(COMPONENTENUM::MATERIAL))
			{
				MaterialComponent* matComp = ecs.GetComponentManager().GetComponent<MaterialComponent>(entity);
				
				if (matComp->type == MATERIALTYPE::PHONG)
				{
					file.write((char*)&matComp->type, sizeof(int));

					PhongMaterial* phong = mManager.GetMaterial<PhongMaterial>(matComp->materialID);
					Helper::WriteToFile(file, phong->GetName());

					phong->Save("..\\materials\\", textureCache);
				}
				else if (matComp->type == MATERIALTYPE::PBR)
				{
					file.write((char*)&matComp->type, sizeof(int));

					PBRMaterial* pbrMat = mManager.GetMaterial<PBRMaterial>(matComp->materialID);
					Helper::WriteToFile(file, pbrMat->GetName());

					pbrMat->Save("..\\materials\\", textureCache);
				}
				// Handle for different type of materials by writing material enum type

				// Avoid copy of material to be written

				
			}

			if (entity.componentMask.test(COMPONENTENUM::PLIGHT))
			{
				PointLight* light = lManager.GetLight<PointLight>(ecs.GetComponentManager().GetComponent<PointLightComponent>(entity)->id);
				file.write((char*)light, sizeof(PointLight));
			}

			if (entity.componentMask.test(COMPONENTENUM::DLIGHT))
			{
				DirectionalLight* light = lManager.GetLight<DirectionalLight>(ecs.GetComponentManager().GetComponent<DirectionalLightComponent>(entity)->id);
				file.write((char*)light, sizeof(DirectionalLight));
			}
		}

		file.close();
	}

	void Load(ID3D12Device* _device, ResourceEngine& _resourceEngine, int _frameIndex, const std::string& _fileDirectory, const std::string& _fileName)
	{
		std::ifstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios::in | std::ios::binary);
		int numEntities = -1;
		file.read((char*)&numEntities, sizeof(int));
		name = _fileName;

		for (int i = 0; i < numEntities; i++)
		{
			std::string entityName;
			Helper::ReadFromFile(file, entityName);

			std::bitset<MAXCOMPONENTS>tempSet;
			for (int i = 0; i < MAXCOMPONENTS; i++)
			{
				bool tempBo = false;
				file.read((char*)&tempBo, sizeof(bool));
				tempSet.set(i, tempBo);
			}

			// Create entity with name
			Entity& tempEnt = CreateEntity(_device, _resourceEngine, entityName);

			Transform tfTemp;
			file.read((char*)&tfTemp.GetTranslation(), sizeof(Vector3));
			file.read((char*)&tfTemp.GetRotation(), sizeof(Vector3));
			file.read((char*)&tfTemp.GetScale(), sizeof(Vector3));

			_resourceEngine.CreateResource(_device, tfTemp.GetBuffer(), (void*)&tfTemp.Compose(), sizeof(Matrix), true, true);
			AddComponentToEntity<Transform>(tempEnt, tfTemp);

			if (tempSet.test(COMPONENTENUM::MESH))
			{
				std::string name;
				Helper::ReadFromFile(file, name);

				Mesh tempMesh;

				if (vbCache.Exists(name))
				{
					tempMesh.SetID(vbCache.GetID(name));
				}
				else
				{
					vbCache.LoadResource(_device, name, "..\\meshes\\");
					tempMesh.SetID(vbCache.GetID(name));
				}

				file.read((char*)&tempMesh.castShadows, sizeof(bool));
				file.read((char*)&tempMesh.receiveShadows, sizeof(float));

				AddComponentToEntity<Mesh>(tempEnt, tempMesh);

			}

			if (tempSet.test(COMPONENTENUM::MATERIAL))
			{
				MaterialComponent matComp;

				file.read((char*)&matComp.type, sizeof(int));

				std::string matName;
				Helper::ReadFromFile(file, matName);

				if (matComp.type == MATERIALTYPE::PHONG)
				{
					if (mManager.Exists<PhongMaterial>(matName))
					{
						matComp.materialID = mManager.GetReferenceID<PhongMaterial>(matName);
					}
					else
					{
						mManager.LoadMaterial<PhongMaterial>(_device, matName);
						matComp.materialID = mManager.GetReferenceID<PhongMaterial>(matName);
					}
				}
				else if (matComp.type == MATERIALTYPE::PBR)
				{
					if (mManager.Exists<PBRMaterial>(matName))
					{
						matComp.materialID = mManager.GetReferenceID<PBRMaterial>(matName);
					}
					else
					{
						mManager.LoadMaterial<PBRMaterial>(_device, matName);
						matComp.materialID = mManager.GetReferenceID<PBRMaterial>(matName);
					}
				}

				AddComponentToEntity<MaterialComponent>(tempEnt, matComp);
			}

			if (tempSet.test(COMPONENTENUM::PLIGHT))
			{
				PointLight pLight;
				PointLightComponent pComp;
				file.read((char*)&pLight, sizeof(PointLight));
				lManager.AddLight(_device, pComp.id, pLight, _frameIndex);
				AddComponentToEntity<PointLightComponent>(tempEnt, pComp);
			}

			if (tempSet.test(COMPONENTENUM::DLIGHT))
			{
				DirectionalLight dLight;
				DirectionalLightComponent dComp;
				file.read((char*)&dLight, sizeof(DirectionalLight));
				lManager.AddLight(_device, dComp.id, dLight, _frameIndex);
				AddComponentToEntity<DirectionalLightComponent>(tempEnt, dComp);
			}
		}

		file.close();
	}

	int FindRec(int _num)
	{
		if (entityNameToId.count("Entity_" + std::to_string(_num)) > 0)
		{
			_num += 1;
			_num = FindRec(_num);
		}

		return _num;
	}

	std::string GetEntityName(Entity _entity)
	{
		if(entityIdToName.count(_entity.id) > 0)
			return entityIdToName.at(_entity.id);
		return "";
	}

	/**Creates an Entity and inserts it into Scene::entities with a unique name and registers a Transform component with default values.
	@return void
	*/
	Entity& CreateEntity(ID3D12Device* _device, ResourceEngine& _resourceEngine)
	{
		Entity tempEnt = ecs.GetEntityManager().CreateEntity();
	//	int num = tempEnt.id;
		const std::string name = CheckName("Entity_" + std::to_string(tempEnt.id));
		entities.Insert(tempEnt.id, tempEnt);
		entityNameToId.emplace(name, tempEnt.id);
		entityIdToName.emplace(tempEnt.id, name);
		Entity* entity = entities.GetObjectByID(tempEnt.id);
		std::wstring wName;
		wName.assign(name.begin(), name.end());
		Transform tfTemp;
		_resourceEngine.CreateResource(_device, tfTemp.GetBuffer(), (void*)&tfTemp.Compose(), sizeof(Matrix), true, true);
		//ecs.GetComponentManager().RegisterComponent<Transform>(entity, Transform(_device, _resourceEngine));
		AddComponentToEntity<Transform>(*entity, tfTemp);
		//entityIdToName.emplace(entity.id, name);
		// Find unique name...

		return *entity;
	}

	void DeleteEntity(const std::string& _name)
	{
		if (entities.Exists(entityNameToId.count(_name) == 0))
			return;

		ecs.ObliterateEntity(GetEntity(_name));
		entities.Remove(entityNameToId.at(_name));
	}

	void DeleteEntity(int _ID)
	{
		ecs.ObliterateEntity(GetEntity(_ID));
		entities.Remove(_ID);
	}

	void RenameEntity(const std::string& _oldName, const std::string& _newName)
	{
		//entities.UpdateStringKey(_oldName, _newName);
		if (entityNameToId.count(_newName) > 0)
			return;

		entityNameToId.emplace(_newName, entityNameToId.at(_oldName));
		entityNameToId.erase(_oldName);

		entityIdToName.at(entityNameToId.at(_newName)) = _newName;
		
	}

	std::string CheckName(const std::string _name)
	{
		if (entityNameToId.count(_name) > 0)
		{
			return CheckName(_name + ".");
		}
		return std::string(_name);
	}

	// crash on resource creation of transform when too fast... sync issue?
	Entity& CreateEntity(ID3D12Device* _device, ResourceEngine& _resourceEngine, const std::string& _name)
	{
		Entity tempEnt = ecs.GetEntityManager().CreateEntity();

		const std::string name = CheckName(_name);

		entities.Insert(tempEnt.id, tempEnt);
		Entity* entity = entities.GetObjectByID(tempEnt.id);
		std::wstring wName;
		wName.assign(name.begin(), name.end());
		//ecs.GetComponentManager().RegisterComponent<Transform>(entity, Transform(_device, _resourceEngine));
		entityIdToName.emplace(tempEnt.id, name);
		entityNameToId.emplace(name, tempEnt.id);
		// Find unique name...

		return *entities.GetObjectByID(tempEnt.id);
	}

	Entity& GetEntity(const std::string& _name)
	{
		return *entities.GetObjectByID(entityNameToId.at(_name));
	}

	Entity& GetEntity(int _ID)
	{
		return *entities.GetObjectByID(_ID);
	}

	bool EntityExists(const std::string& _name)
	{
		return entityNameToId.count(_name) > 0;
	}

	/**Registers a default Mesh component for the specified Entity object and binds the Entity to the appropriate systems.
	@param _entityName The Entity to register a mesh component for
	@return void
	*/
	template<typename T>
	void AddComponentToEntity(Entity& _entity, const T& _data /*Add template args that works with any inputs*/);


	/**Removes a Mesh component for the specified Entity object and unbinds the Entity from the appropriate systems.
	@param _entityName The Entity to remove a Mesh component for
	@return void
	*/
	template<typename T>
	void RemoveComponentFromEntity(Entity& _entity /*Add template args that works with any inputs*/);

	template<typename T>
	void UpdateComponentForEntity(Entity _entity, const T& _data /*Add template args that works with any inputs*/);

	template<typename T>
	T* GetComponentForEntity(Entity _entity /*Add template args that works with any inputs*/);
};

template<typename T>
inline void Scene::AddComponentToEntity(Entity& _entity, const T& _data)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		ecs.GetComponentManager().RegisterComponent<Transform>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		ecs.GetComponentManager().RegisterComponent<Mesh>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		ecs.GetComponentManager().RegisterComponent<MaterialComponent>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		ecs.GetComponentManager().RegisterComponent<PointLightComponent>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		ecs.GetComponentManager().RegisterComponent<DirectionalLightComponent>(_entity, _data);
	}
}

template<typename T>
inline void Scene::RemoveComponentFromEntity(Entity& _entity)
{
	if constexpr (std::is_same_v<T, Mesh>)
	{
		ecs.GetComponentManager().RemoveComponent<Mesh>(_entity);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		ecs.GetComponentManager().RemoveComponent<MaterialComponent>(_entity);
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		ecs.GetComponentManager().RemoveComponent<PointLightComponent>(_entity);
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		ecs.GetComponentManager().RemoveComponent<DirectionalLightComponent>(_entity);
	}
}

template<typename T>
inline void Scene::UpdateComponentForEntity(Entity _entity, const T& _data)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		*ecs.GetComponentManager().GetComponent<Transform>(_entity) = _data;
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		*ecs.GetComponentManager().GetComponent<Mesh>(_entity) = _data;
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		*ecs.GetComponentManager().GetComponent<MaterialComponent>(_entity) = _data;
	}
}

template<typename T>
inline T* Scene::GetComponentForEntity(Entity _entity)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		return ecs.GetComponentManager().GetComponent<Transform>(_entity);
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		return ecs.GetComponentManager().GetComponent<Mesh>(_entity);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		return ecs.GetComponentManager().GetComponent<MaterialComponent>(_entity);
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		return ecs.GetComponentManager().GetComponent<PointLightComponent>(_entity);
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		return ecs.GetComponentManager().GetComponent<DirectionalLightComponent>(_entity);
	}

	return nullptr;
}
