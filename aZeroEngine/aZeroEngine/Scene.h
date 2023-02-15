#pragma once
#include "ECSBase.h"
#include "VertexBufferCache.h"
#include "Texture2DCache.h"
#include "MaterialManager.h"
#include <fstream>
#include <iostream>

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

	ResourceManager& rManager;

	std::unordered_map<int, std::string> entityIdToName;

	std::string name = "";
public:
	MappedVector<Entity> entities;

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
	Scene(ECS& _ecs, VertexBufferCache& _vbCache, MaterialManager& _mManager, ResourceManager& _rManager, Texture2DCache& _textureCache)
		:ecs(_ecs), vbCache(_vbCache), textureCache(_textureCache), mManager(_mManager), rManager(_rManager)
	{
	}

	/**Removes all the Entity objects within Scene::entities using ECS::ObliterateEntity()
	*/
	~Scene()
	{
		for (auto& [name, index] : entities.GetStringToIndexMap())
		{
			// Free vb ref id?

			ecs.ObliterateEntity(entities.Get(index));
		}
	}

	void Save(const std::string& _fileDirectory, const std::string& _fileName, bool _debugASCII = false)
	{
		int size = (int)entities.GetStringToIndexMap().size();

		if (size == 0)
			return;

		std::ofstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios_base::trunc | std::ios::out | std::ios::binary);

		bool yes = true;
		bool no = false;

		file.write((char*)&size, sizeof(int));
		for (const auto [name, id] : entities.GetStringToIndexMap())
		{
			const Entity entity = entities.Get(id);

			Helper::WriteToFile(file, name);

			Transform* tf = ecs.GetComponentManager().GetComponent<Transform>(entity);
			file.write((char*)&tf->GetTranslation(), sizeof(Vector3));
			file.write((char*)&tf->GetRotation(), sizeof(Vector3));
			file.write((char*)&tf->GetScale(), sizeof(Vector3));

			Mesh* mesh = ecs.GetComponentManager().GetComponent<Mesh>(entity);
			if (mesh != nullptr)
			{
				file.write((char*)&yes, sizeof(bool));
				std::string fileName = vbCache.GetBuffer(mesh->GetVBIndex())->GetFileName();
				Helper::WriteToFile(file, fileName);
			}
			else
			{
				file.write((char*)&no, sizeof(bool));
			}

			MaterialComponent* matComp = ecs.GetComponentManager().GetComponent<MaterialComponent>(entity);
			if (matComp != nullptr)
			{
				file.write((char*)&yes, sizeof(bool));

				// Handle for different type of materials by writing material enum type

				// Avoid copy of material to be written

				PhongMaterial* phong = mManager.GetMaterial<PhongMaterial>(matComp->GetMaterialID());
				Helper::WriteToFile(file, phong->GetName());

				phong->Save("..\\materials\\", phong->GetName(), textureCache, _debugASCII);
			}
			else
			{
				file.write((char*)&no, sizeof(bool));
			}
		}

		if(_debugASCII)
		{
			std::ofstream file(_fileDirectory + "/" + _fileName + "_ASCII.txt", std::ios_base::trunc);

			for (const auto [name, id] : entities.GetStringToIndexMap())
			{
				const Entity entity = entities.Get(id);

				file << "--------------------------------------------------------------------\n";
				file << "Entity Name: " + name << std::endl;
				file << "|---------Transform Data--------|\n";
				Transform* tf = ecs.GetComponentManager().GetComponent<Transform>(entity);
				Matrix tfMat = tf->Compose();
				file << tfMat._11 << " ";
				file << tfMat._12 << " ";
				file << tfMat._13 << " ";
				file << tfMat._14 << "\n";
				file << tfMat._21 << " ";
				file << tfMat._22 << " ";
				file << tfMat._23 << " ";
				file << tfMat._24 << "\n";
				file << tfMat._31 << " ";
				file << tfMat._32 << " ";
				file << tfMat._33 << " ";
				file << tfMat._34 << "\n";
				file << tfMat._41 << " ";
				file << tfMat._42 << " ";
				file << tfMat._43 << " ";
				file << tfMat._44 << "\n";

				Mesh* mesh = ecs.GetComponentManager().GetComponent<Mesh>(entity);
				if (mesh != nullptr)
				{
					file << "\n|---------Mesh Data-------------|\n";
					file << "Filename: " << vbCache.GetBuffer(mesh->GetVBIndex())->GetFileName() << std::endl;
				}

				MaterialComponent* material = ecs.GetComponentManager().GetComponent<MaterialComponent>(entity);
				if (material != nullptr)
				{
					// Check material type and act accordingly. Store file names for textures which are then loaded (or not) using the texture2dcache
					PhongMaterial* phongMat = mManager.GetMaterial<PhongMaterial>(material->GetMaterialID());
					file << "\n|---------Material Data-------------|\n";
					file << "Material Name: " << phongMat->GetName() << "\n";
				}

			}

			file.close();
		}
	}

	void Load(ID3D12Device* _device, CommandList& _cmdList, int _frameIndex, const std::string& _fileDirectory, const std::string& _fileName)
	{
		std::ifstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios::in | std::ios::binary);
		int numEntities = -1;
		file.read((char*)&numEntities, sizeof(int));
		name = _fileName;

		for (int i = 0; i < numEntities; i++)
		{
			std::string entityName;
			Helper::ReadFromFile(file, entityName);

			// Create entity with name
			Entity& tempEnt = CreateEntity(_device, &_cmdList, entityName);

			// Create transform component
			Transform* tf = GetComponentForEntity<Transform>(tempEnt);

			file.read((char*)&tf->GetTranslation(), sizeof(Vector3));
			file.read((char*)&tf->GetRotation(), sizeof(Vector3));
			file.read((char*)&tf->GetScale(), sizeof(Vector3));
			tf->Update(&_cmdList, _frameIndex);

			bool tempCheck = false;
			file.read((char*)&tempCheck, sizeof(bool));
			if (tempCheck)
			{
				std::string name;
				Helper::ReadFromFile(file, name);

				Mesh tempMesh;

				if (vbCache.Exists(name))
				{
					tempMesh.SetVBIndex(vbCache.GetBufferIndex(name));
				}
				else
				{
					vbCache.LoadResource(_device, _cmdList, name);
					tempMesh.SetVBIndex(vbCache.GetBufferIndex(name));
				}
				AddComponentToEntity<Mesh>(tempEnt, tempMesh);

			}

			file.read((char*)&tempCheck, sizeof(bool));
			if (tempCheck)
			{
				// Handle for different type of materials by checking material enum type

				std::string matName;
				Helper::ReadFromFile(file, matName);

				MaterialComponent matComp;
				if (mManager.Exists(matName))
				{
					matComp.SetMaterialID(mManager.GetReferenceID<PhongMaterial>(matName));
				}
				else
				{
					mManager.LoadMaterial<PhongMaterial>(_device, _cmdList, matName);
					matComp.SetMaterialID(mManager.GetReferenceID<PhongMaterial>(matName));
				}

				AddComponentToEntity<MaterialComponent>(tempEnt, matComp);
			}
		}

		file.close();
	}

	int FindRec(int _num)
	{
		if (entities.Exists("Entity_" + std::to_string(_num)))
		{
			_num += 1;
			_num = FindRec(_num);
		}

		return _num;
	}

	/**Creates an Entity and inserts it into Scene::entities with a unique name and registers a Transform component with default values.
	@return void
	*/
	Entity& CreateEntity(ID3D12Device* _device, CommandList* _cmdList)
	{
		Entity tempEnt = ecs.GetEntityManager().CreateEntity();
		int num = tempEnt.id;
		const std::string name = CheckName("Entity_" + std::to_string(num));
		entities.Add(name, tempEnt);
		Entity& entity = entities.Get(name);
		std::wstring wName;
		wName.assign(name.begin(), name.end());
		ecs.GetComponentManager().RegisterComponent<Transform>(entity, Transform(_device, _cmdList));
		entityIdToName.emplace(tempEnt.id, name);
		// Find unique name...

		return entity;
	}

	void DeleteEntity(const std::string& _name)
	{
		ecs.ObliterateEntity(GetEntity(_name));
		entities.Remove(_name);
	}

	void DeleteEntity(int _ID)
	{
		ecs.ObliterateEntity(GetEntity(_ID));
		entities.Remove(_ID);
	}

	void RenameEntity(const std::string& _oldName, const std::string& _newName)
	{
		entities.UpdateStringKey(_oldName, _newName);
	}

	std::string CheckName(const std::string _name)
	{
		if (entities.Exists(_name))
		{
			return CheckName(_name + ".");
		}
		return std::string(_name);
	}

	Entity& CreateEntity(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
	{
		Entity tempEnt = ecs.GetEntityManager().CreateEntity();

		const std::string name = CheckName(_name);

		entities.Add(name, tempEnt);
		Entity& entity = entities.Get(name);
		std::wstring wName;
		wName.assign(name.begin(), name.end());
		ecs.GetComponentManager().RegisterComponent<Transform>(entity, Transform(_device, _cmdList));
		entityIdToName.emplace(tempEnt.id, name);
		// Find unique name...

		return entities.Get(_name);
	}

	Entity& GetEntity(const std::string& _name)
	{
		return entities.Get(_name);
	}

	Entity& GetEntity(int _ID)
	{
		return entities.Get(_ID);
	}

	const std::string GetEntityName(const Entity& _entity) const
	{
		return entityIdToName.at(_entity.id);
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
	if constexpr (std::is_same_v<T, Mesh>)
	{
		ecs.GetComponentManager().RegisterComponent<Mesh>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		ecs.GetComponentManager().RegisterComponent<MaterialComponent>(_entity, _data);
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
		ecs.GetComponentManager().RegisterComponent<Mesh>(_entity);
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

	return nullptr;
}
