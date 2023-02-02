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
	/// \public
	ECS* ecs; /**< Pointer to the ECS instance used within its member functions.*/
	/// \public
	EntityManager* eManager; /**< Pointer to the EntityManager instance used within its member functions.*/
	/// \public
	ComponentManager* cManager; /**< Pointer to the ComponentManager instance used within its member functions.*/
	/// \public
	VertexBufferCache* vbCache; /**< Pointer to the VertexBufferCache instance used within its member functions.*/

	MaterialManager* mManager;

	Texture2DCache* textureCache;

	ResourceManager* rManager;

public:
	MappedVector<Entity>entities;

	Scene() = default;

	/**Initiates the private member variables.
	@param _ecs Pointer to the ECS instance that should be used within its member functions
	@param _vbCache Pointer to the VertexBufferCache instance that should be used within its member functions
	*/
	Scene(ECS* _ecs, VertexBufferCache* _vbCache, MaterialManager* _mManager, ResourceManager* _rManager, Texture2DCache* _textureCache)
	{
		ecs = _ecs;
		vbCache = _vbCache;
		mManager = _mManager;
		eManager = &ecs->GetEntityManager();
		cManager = &ecs->GetComponentManager();
		rManager = _rManager;
		textureCache = _textureCache;
	}

	/**Removes all the Entity objects within Scene::entities using ECS::ObliterateEntity()
	*/
	~Scene()
	{
		std::vector<Entity>& ents = entities.GetObjects();
		for (Entity& ent : ents)
		{
			// Free vb ref id?

			ecs->ObliterateEntity(ent);
		}
	}

	void Save(const std::string& _fileDirectory, const std::string& _fileName, bool _debugASCII = false)
	{
		std::vector<Entity>& ents = entities.GetObjects();

		std::ofstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios_base::trunc | std::ios::out | std::ios::binary);

		bool yes = true;
		bool no = false;


		int size = (int)ents.size();
		file.write((char*)&size, sizeof(int));
		for (const auto [name, id] : entities.GetStringToIndexMap())
		{
			const Entity entity = entities.Get(id);

			Helper::WriteToFile(file, name);

			Matrix tf = cManager->GetComponent<Transform>(entity)->worldMatrix;
			file.write((char*)&tf, sizeof(Matrix));

			Mesh* mesh = cManager->GetComponent<Mesh>(entity);
			if (mesh != nullptr)
			{
				file.write((char*)&yes, sizeof(bool));
				std::string fileName = vbCache->GetBuffer(mesh->vbIndex)->GetFileName();
				Helper::WriteToFile(file, fileName);
			}
			else
			{
				file.write((char*)&no, sizeof(bool));
			}

			MaterialComponent* matComp = cManager->GetComponent<MaterialComponent>(entity);
			if (matComp != nullptr)
			{
				file.write((char*)&yes, sizeof(bool));

				// Handle for different type of materials by writing material enum type

				// Avoid copy of material to be written

				PhongMaterial* phong = mManager->GetMaterial<PhongMaterial>(matComp->materialID);
				Helper::WriteToFile(file, phong->name);

				phong->Save(_fileDirectory, phong->name, textureCache, _debugASCII);
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
				Transform* tf = cManager->GetComponent<Transform>(entity);
				file << tf->worldMatrix._11 << " ";
				file << tf->worldMatrix._12 << " ";
				file << tf->worldMatrix._13 << " ";
				file << tf->worldMatrix._14 << "\n";
				file << tf->worldMatrix._21 << " ";
				file << tf->worldMatrix._22 << " ";
				file << tf->worldMatrix._23 << " ";
				file << tf->worldMatrix._24 << "\n";
				file << tf->worldMatrix._31 << " ";
				file << tf->worldMatrix._32 << " ";
				file << tf->worldMatrix._33 << " ";
				file << tf->worldMatrix._34 << "\n";
				file << tf->worldMatrix._41 << " ";
				file << tf->worldMatrix._42 << " ";
				file << tf->worldMatrix._43 << " ";
				file << tf->worldMatrix._44 << "\n";

				Mesh* mesh = cManager->GetComponent<Mesh>(entity);
				if (mesh != nullptr)
				{
					file << "\n|---------Mesh Data-------------|\n";
					file << "Filename: " << vbCache->GetBuffer(mesh->vbIndex)->GetFileName() << std::endl;
				}

				MaterialComponent* material = cManager->GetComponent<MaterialComponent>(entity);
				if (material != nullptr)
				{
					// Check material type and act accordingly. Store file names for textures which are then loaded (or not) using the texture2dcache
					PhongMaterial* phongMat = mManager->GetMaterial<PhongMaterial>(material->materialID);
					//PhongMaterialInformation* info = phongMat->GetInfoPtr();
					file << "\n|---------Material Data-------------|\n";
					file << "Material Name: " << phongMat->name << "\n";
					/*file << "Diffuse Texture Filename: " << textureCache->GetTextureName(info->diffuseTextureID) << "\n";
					file << "Ambient RGB: [" << info->ambientAbsorbation.x << ":" << info->ambientAbsorbation.y << ":" << info->ambientAbsorbation.z << "]\n";
					file << "Specular RGB: [" << info->specularAbsorbation.x << ":" << info->specularAbsorbation.y << ":" << info->specularAbsorbation.z << "]\n";
					file << "Specular Exponent: " << info->specularShine << "\n";*/
				}

			}

			file.close();
		}
	}

	void Load(ID3D12Device* _device, CommandList* _cmdList, int _frameIndex, const std::string& _fileDirectory, const std::string& _fileName)
	{
		std::ifstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios::in | std::ios::binary);
		int numEntities = -1;
		file.read((char*)&numEntities, sizeof(int));
		std::cout << "Num entities: " << numEntities << std::endl;

		for (int i = 0; i < numEntities; i++)
		{
			std::cout << "|---------------------------------New entity---------------------------------|\n";

			std::string entityName;
			Helper::ReadFromFile(file, entityName);
			std::cout << "Entity Name: " << entityName << std::endl;

			// Create entity with name
			Entity& tempEnt = CreateEntity(_device, _cmdList, entityName);

			Matrix mat;
			file.read((char*)&mat, sizeof(Matrix));
			std::cout << "Matrix: \n";
			std::cout << mat._11 << " ";
			std::cout << mat._12 << " ";
			std::cout << mat._13 << " ";
			std::cout << mat._14 << "\n";
			std::cout << mat._21 << " ";
			std::cout << mat._22 << " ";
			std::cout << mat._23 << " ";
			std::cout << mat._24 << "\n";
			std::cout << mat._31 << " ";
			std::cout << mat._32 << " ";
			std::cout << mat._33 << " ";
			std::cout << mat._34 << "\n";
			std::cout << mat._41 << " ";
			std::cout << mat._42 << " ";
			std::cout << mat._43 << " ";
			std::cout << mat._44 << "\n";

			// Create transform component
			GetComponentForEntity<Transform>(tempEnt)->Update(_cmdList, mat, _frameIndex);

			bool tempCheck = false;
			file.read((char*)&tempCheck, sizeof(bool));
			if (tempCheck)
			{
				std::string name;
				Helper::ReadFromFile(file, name);
				std::cout << "Mesh Name: " << name << std::endl;

				Mesh tempMesh;

				if (vbCache->Exists(name))
				{
					tempMesh.vbIndex = vbCache->GetBufferIndex(name);
				}
				else
				{
					tempMesh.vbIndex = vbCache->LoadBuffer(_device, _cmdList, name);
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
				if (mManager->Exists(matName))
				{
					matComp.materialID = mManager->GetReferenceID<PhongMaterial>(matName);
				}
				else
				{
					mManager->CreateMaterial<PhongMaterial>(_device, rManager, _cmdList, textureCache, _fileDirectory, matName);
					matComp.materialID = mManager->GetReferenceID<PhongMaterial>(matName);
				}

				AddComponentToEntity<MaterialComponent>(tempEnt, matComp);
			}
		}

		file.close();
	}

	/**Creates an Entity and inserts it into Scene::entities with a unique name and registers a Transform component with default values.
	@return void
	*/
	Entity& CreateEntity(ID3D12Device* _device, CommandList* _cmdList)
	{
		Entity tempEnt = eManager->CreateEntity();
		std::string name("Entity_" + std::to_string(tempEnt.id));
		entities.Add(name, tempEnt);
		Entity& entity = entities.Get(name);
		std::wstring wName;
		wName.assign(name.begin(), name.end());
		cManager->RegisterComponent<Transform>(entity, Transform(_device, _cmdList))->cb.GetResource()->SetName(wName.c_str());

		// Find unique name...

		return entities.Get(name);
	}

	Entity& CreateEntity(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
	{
		Entity tempEnt = eManager->CreateEntity();
		entities.Add(_name, tempEnt);
		Entity& entity = entities.Get(_name);
		std::wstring wName;
		wName.assign(_name.begin(), _name.end());
		cManager->RegisterComponent<Transform>(entity, Transform(_device, _cmdList))->cb.GetResource()->SetName(wName.c_str());

		// Find unique name...

		return entities.Get(_name);
	}

	Entity& GetEntity(const std::string& _name)
	{
		return entities.Get(_name);
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
		cManager->RegisterComponent<Mesh>(_entity, _data);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		cManager->RegisterComponent<MaterialComponent>(_entity, _data);
	}
	

	// Add to systems...?
}

template<typename T>
inline void Scene::RemoveComponentFromEntity(Entity& _entity)
{
	if constexpr (std::is_same_v<T, Mesh>)
	{
		cManager->RemoveComponent<Mesh>(_entity);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		cManager->RegisterComponent<Mesh>(_entity);
	}

	// Unregister from all systems using the Mesh component...

	// ...
}

template<typename T>
inline void Scene::UpdateComponentForEntity(Entity _entity, const T& _data)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		*cManager->GetComponent<Transform>(_entity) = _data;
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		*cManager->GetComponent<Mesh>(_entity) = _data;
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		*cManager->GetComponent<MaterialComponent>(_entity) = _data;
	}
}

template<typename T>
inline T* Scene::GetComponentForEntity(Entity _entity)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		return cManager->GetComponent<Transform>(_entity);
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		return cManager->GetComponent<Mesh>(_entity);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		return cManager->GetComponent<MaterialComponent>(_entity);
	}

	return nullptr;
}
