#pragma once
//#include "ECSBase.h"
#include "ECS.h"
#include "ModelCache.h"
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
	aZeroECS::ComponentManager* m_componentManager;
	aZeroECS::EntityManager* m_entityManager;
	ModelCache* modelCache = nullptr;
	MaterialManager* mManager = nullptr;
	LightSystem* lSystem = nullptr;

	std::string name = "";
	std::unordered_map<int, std::string> entityIdToName;
	std::unordered_map<std::string, int> entityNameToId;
	MappedVector<aZeroECS::Entity> entities;

	bool loaded = true;

private:
	std::string CheckName(const std::string _name) const;
	void ShutDown();

public:
	Scene() = default;

	/**Uses dependency injection for the neccessary objects used within the Scene class.
	@param _ecs The ECS instance to use within the Scene for all operations which handles Entity modification, creation, deletion, and more...
	@param _vbCache The VertexBufferCache instance to retrieve neccessary VertexBuffer object's from when loading and saving the Scene
	@param _mManager The MaterialManager instance to retrieve neccessary Material object's from when loading and saving the Scene
	@param _lSystem The LightSystem instance to retrieve neccessary light object's from when loading and saving the Scene
	@return void
	*/
	Scene(ModelCache* _modelCache, MaterialManager* _mManager,
		LightSystem* _lSystem, aZeroECS::ComponentManager* componentManager, aZeroECS::EntityManager* entityManager)
		:modelCache(_modelCache), mManager(_mManager), lSystem(_lSystem), m_componentManager(componentManager), m_entityManager(entityManager)
		{ }

	~Scene();

	Scene(Scene&& _other) noexcept;

	// Getters / Setters
	std::string GetName() const { return name; }
	void SetName(const std::string& _name) { name = _name; }
	aZeroECS::Entity& GetEntity(const std::string& _name) { return *entities.GetByID(entityNameToId.at(_name)); }
	aZeroECS::Entity& GetEntity(int _ID) { return *entities.GetByID(_ID); }
	std::optional<std::string> GetEntityName(aZeroECS::Entity _entity) const;
	std::vector<aZeroECS::Entity>& GetEntityVector() { return entities.GetObjects(); }

	/**Saves the contents of the Scene object into a .azs file inside the specified directory and with the specified name.
	@param _fileDirectory The directory to save the scene file to
	@param _fileName The name of the scene file without the extension
	@param _textureCache The Texture2DCache to retrieve an Entity object's Material Texture2D names from
	@return void
	*/
	void Save(const std::string& _fileDirectory, const std::string& _fileName, Texture2DCache* _textureCache);

	/**Loads a scene file and store it's contents inside the class.
	* It will clear the contents of the Scene object and then load the new scene file if the object has been loaded earlier.
	@param _fileDirectory The directory containing the scene file
	@param _fileName The name of the scene file without the extension
	@return bool TRUE: The Scene was successfully loaded, FALSE: The Scene failed to be loaded
	*/
	bool Load(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& _fileDirectory, const std::string& _fileName);

	/**Generates a new Entity with a transform component and returns the reference to it.
	* The name of the Entity will be slightly modified if the name is already taken.
	@param _name Name of the new Entity
	@return Entity&
	*/
	aZeroECS::Entity& CreateEntity(const std::string& _name);

	/**Removes the Entity from the Scene if it exists.
	@param _ID The unique ID of the Entity to remove
	@return void
	*/
	void DeleteEntity(int _ID);

	/**Removes the Entity from the Scene if it exists.
	@param _name The unique name of the Entity to remove
	@return void
	*/
	void DeleteEntity(const std::string& _name);

	/**Returns whether or not an Entity with the specified name exists in the Scene.
	@param _name Name of the Entity to look for
	@return bool TRUE: The Entity exist, FALSE: The Entity doesn't exist
	*/
	bool EntityExists(const std::string& _name) const { return entityNameToId.count(_name) > 0; }
	
	/**Renames the input Entity if the name isn't already taken by another Entity within the same Scene.
	@param _entity The Entity to change the name of
	@param _newName The new name of the Entity
	@return void
	*/
	void RenameEntity(const aZeroECS::Entity& _entity, const std::string& _newName);

	/**Registers a default Mesh component for the specified Entity object and binds the Entity to the appropriate systems.
	@param _entityName The Entity to register a mesh component for
	@param _data The initial data for the registered component
	@return void
	*/
	template<typename T>
	void AddComponentToEntity(aZeroECS::Entity& _entity, T&& _data)
	{ 
		m_componentManager->AddComponent<T>(_entity, std::move(_data));
	}

	/**Registers a default Mesh component for the specified Entity object WITHOUT binding it to the appropriate systems.
	@param _entityName The Entity to register a mesh component for
	@return void
	*/
	template<typename T>
	void AddComponentToEntity(aZeroECS::Entity& _entity) { m_componentManager->AddComponent<T>(_entity); }

	/**Removes a Mesh component for the specified Entity object and unbinds the Entity from the appropriate systems.
	@param _entityName The Entity to remove a Mesh component for
	@return void
	*/
	template<typename T>
	void RemoveComponentFromEntity(aZeroECS::Entity& _entity)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			PointLightComponent* pComp = m_componentManager->GetComponent<PointLightComponent>(_entity);
			if (pComp)
			{
				lSystem->RemoveLight(_entity);
			}
		}
		m_componentManager->RemoveComponent<T>(_entity);
	}

	/**Returns a pointer for the input Entity objects component specified by the template parameter.
	* Returns nullptr if the Entity doesn't have a component of the specified type registered.
	@param _entity The Entity to get the component for
	@return T*
	*/
	template<typename T>
	T* GetComponentForEntity(aZeroECS::Entity _entity) { return m_componentManager->GetComponent<T>(_entity); }
};