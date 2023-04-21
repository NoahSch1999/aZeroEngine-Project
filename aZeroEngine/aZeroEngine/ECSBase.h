/*
This code was created by me, Noah. It's my first ECS. If you wanna use it, feel free to do so!

Github: https://github.com/NoahSch1999
*/

#pragma once
#include <map>
#include <unordered_map>
#include <bitset>
#include <type_traits>
#include <queue>
#include "ResourceEngine.h"
#include <deque>
#include <memory>
#include <utility>
#include "UniqueIntList.h"
class BaseResource;
class VertexBuffer;
class ConstantBuffer;

// COMPONENTS
/**
Maximum components per Entity
*/
static const int MAXCOMPONENTS = 10;

/** @brief Component enumeration for usage in conjunction with the Entity std::bitset.
*/
namespace COMPONENTENUM
{
	enum COMPONENTBITID { TRANSFORM, MESH, MATERIAL, PLIGHT, DLIGHT, MAX };
	inline std::string COMPONENTNAMES[4]{ "Transform", "Mesh", "Material", "Point Light" };
}

class Transform
{
private:
	Vector3 translation = Vector3::Zero;
	Vector3 rotation = Vector3::Zero;
	Vector3 scale = Vector3(1.f, 1.f, 1.f);

	Matrix worldMatrix = Matrix::Identity;
	Matrix localMatrix = Matrix::Identity;
public:
	UINT frameIndex = 0;

	Vector3& GetTranslation() { return translation; }
	Vector3& GetRotation() { return rotation; }
	Vector3& GetScale() { return scale; }

	Matrix GetLocalMatrix() const
	{
		return  Matrix::CreateScale(scale) * Matrix::CreateFromYawPitchRoll(rotation) * Matrix::CreateTranslation(translation);
	}

	void SetWorldMatrix(const Matrix& _matrix) { worldMatrix = _matrix; }
	Matrix GetWorldMatrix() const { return worldMatrix; }

	Transform() = default;
};

class Mesh
{
private:
	int vbID;
public:
	Mesh() = default;
	Mesh(int _vbID) :vbID(_vbID) {};
	void SetID(int _vbID) { vbID = _vbID; }
	int GetID() { return vbID; }

	bool castShadows = false;
	int receiveShadows = 1;
};

enum MATERIALTYPE { PBR };

struct MaterialComponent
{
public:
	MaterialComponent() = default;
	MaterialComponent(int _materialID) :materialID(_materialID) {};

	int materialID = -1;
	MATERIALTYPE type = MATERIALTYPE::PBR;
};

// Light component
struct PointLightComponent
{
	PointLightComponent() = default;
	int id = -1;
};

struct DirectionalLightComponent
{
	DirectionalLightComponent() = default;
	int id = -1;
};
//

/** @brief Contains an ID and std::bitset which a user can register components for using the ComponentManager class.
*/
struct Entity
{
	Entity() = default;
	int id = -1; /**<Unique ID mapped to a component within the ComponentManager class that the component was registered for using ComponentManager::RegisterComponent(Entity& _entity, const T& _initValue)*/
	std::bitset<MAXCOMPONENTS> componentMask; /**<Describes what type of components that the instance of the Entity has registered*/
};

/** @brief A data structure which contains a contiguous array of elements that can be accessed through an ID.
*/
template<typename T>
class SlottedMap
{
private:
	std::unordered_map<int, int> idToIndex;
	std::unordered_map<int, int> indexToId;
	std::vector<T> objects;

public:

	/** Allocates space for number of T elements.
	@param _startSize Number of spaces that should be allocated. Total allocation in bytes are _startSize * sizeof(T).
	*/
	SlottedMap(int _startSize = 100)
	{
		objects.reserve(_startSize);
	}

	/** Adds an element to the SlottedMap object so that it can be retrieved using the input _id using SlottedMap::GetObjectByID().
	* It isn't added if an element with _id already exists within the SlottedMap.
	@param _id The id which can be used to access the element within the SlottedMap.
	@param _value The data to copy into the SlottedMap. It will always be added to the last index of the std::vector return by SlottedMap::GetObjects().
	@return void
	*/
	void Add(int _id, const T& _value)
	{
		if (Exists(_id))
			return;

		if (objects.capacity() == objects.size())
		{
			objects.reserve(objects.size() + 100);
		}

		int index = objects.size();
		objects.emplace_back(_value);
		idToIndex.emplace(_id, index);
		indexToId[index] = _id;
	}

	/** Removes an element from the SlottedMap object.
	* It isn't removed if no element with _id exists within the SlottedMap.
	@param _id ID of the element to remove.
	@return void
	*/
	void Remove(int _id)
	{
		if (!Exists(_id))
			return;

		int index = idToIndex.at(_id);
		idToIndex.erase(_id);

		int indexOfLast = objects.size() - 1;

		if (indexOfLast == index)
		{
			indexToId.erase(index);
			objects.erase(objects.begin() + index);
			return;
		}

		objects[index] = objects[indexOfLast];

		int idOfLast = indexToId.at(indexOfLast);
		idToIndex.at(idOfLast) = index;

		indexToId.at(index) = idOfLast;
		indexToId.erase(indexOfLast);
		objects.erase(objects.begin() + indexOfLast);

		return;
	}

	/** Shrinks the internal std::vector to fit the number of elements inside it.
	* Should be used whenever you want to reduce memory consumption.
	@return void
	*/
	void ShrinkToFit()
	{
		objects.shrink_to_fit();
	}

	/** Returns whether or not an element with the given ID exists.
	@param _id ID to check for.
	@return bool TRUE: Element exists, FALSE: No element exists.
	*/
	bool Exists(int _id) const
	{
		return idToIndex.count(_id) > 0;
	}

	/** Returns a pointer to the element at the given ID.
	@param _id The ID to retrieve the element with.
	@return T*
	*/
	T* GetObjectByID(int _id) { return &objects[idToIndex.at(_id)]; }

	/** Returns a reference to the internal vector.
	* To iterate over it in a safe manner, use structure binding with an iterator or loop using the std::vector::size().
	@return std::vector<T>&
	*/
	std::vector<T>& GetObjects() { return objects; }
};

/** @brief A data structure which contains a contiguous array of elements that can be accessed through an ID (int) or key(string).
*/
template <typename T>
class NamedSlottedMap
{
private:
	SlottedMap<T>map;

	UniqueIntList idList;

	std::unordered_map<std::string, int>strToID;
	std::unordered_map<int, std::string>IDtoStr;
public:
	NamedSlottedMap(int _startMax, int _incPerEmpty = 100)
		:idList(_startMax, _incPerEmpty)
	{
	}

	/** Adds an element to the last position in the array and creates a string key to access it through NamedSlottedMap::GetObjectByStr()
	@param _key String key which will be used to access the object.
	@param _data Data to copy to the object.
	@return int The ID that can be used to access the element through NamedSlottedMap::GetObjectByID(). If an object with _key value already exists, the method returns -1.
	*/
	int Add(const std::string& _key, const T& _data)
	{
		if (strToID.count(_key) > 0)
			return -1;

		int id = idList.LendKey();

		map.Add(id, _data);

		strToID.emplace(_key, id);
		IDtoStr.emplace(id, _key);

		return id;
	}

	/** Removes the element matching the input _key.
	@param _key The key of the element to remove.
	@return void
	*/
	void Remove(const std::string& _key)
	{
		if (strToID.count(_key) == 0)
			return;

		int id = strToID.at(_key);
		map.Remove(id);
		strToID.erase(_key);
		IDtoStr.erase(id);
		idList.ReturnKey(id);
	}

	void Remove(int& _key)
	{
		if (!map.Exists(_key))
			return;

		map.Remove(_key);
		strToID.erase(IDtoStr.at(_key));
		IDtoStr.erase(_key);
		idList.ReturnKey(_key);
	}

	bool Exists(const std::string& _key) const
	{
		if (strToID.count(_key) > 0)
			return true;

		return false;
	}

	bool Exists(int _key) const
	{
		return map.Exists(_key);
	}

	T* GetObjectByKey(const std::string& _key)
	{
		if (strToID.count(_key) == 0)
			return nullptr;

		return map.GetObjectByID(strToID.at(_key));
	}

	T* GetObjectByKey(int _key)
	{
		if (!map.Exists(_key))
			return nullptr;

		return map.GetObjectByID(_key);
	}

	/** Returns a reference to all the objects.
	@return std::vector<T>&
	*/
	std::vector<T>& GetObjects() { return map.GetObjects(); }

	/** Returns a reference to a map containing the names of the objects and their respective IDs.
	@return std::unordered_map<std::string, int>&
	*/
	std::unordered_map<std::string, int>& GetStrToIndexMap() { return strToID; }

	/** Returns a reference to all the objects.
	@param _key Key to be used to get the object ID.
	@return int The ID of the object matching _key. If the object doesn't exist it returns -1
	*/
	int GetID(const std::string _key) const
	{
		if (strToID.count(_key) == 0)
			return -1;

		return strToID.at(_key);
	}

	std::string GetString(int _key) const
	{
		if (IDtoStr.count(_key) == 0)
			return "";

		return IDtoStr.at(_key);
	}
};

/** @brief Contains and handles all components. This class can be used in conjunction with the Entity struct to register components for an Entity object.

NOTE!!!!
Several things has to be added to this class before using.
One BiDirectionalMap should be created as a member variable for each component.
You should also write the logic within the ComponentManager::RegisterComponent(), ComponentManager::RemoveComponent(), ComponentManager::GetComponent(), and ComponentManager::GetComponentFast() for each component.
Simply replace Comp1 and COMPONENTENUM::Comp1 within the source code with the component type and COMPONENTENUM enumeration of the custom made component.
Read the source code for examples.
*/
class ComponentManager
{
private:
	SlottedMap<Transform> transformMap;
	SlottedMap<Mesh> meshMap;
	SlottedMap<MaterialComponent> materialMap;
	SlottedMap<PointLightComponent> pLightMap;
	SlottedMap<DirectionalLightComponent> dLightMap;

public:
	ComponentManager() = default;
	/** Registers a new component for the input Entity object and initializes the new component using the input component object. Uses the copy constructor to copy the input component into the newly created component.
	@param _entity The Entity to register the component for
	@param _initValue Initial value of the component
	@return Pointer to the newly registered component within a internal std::vector
	*/

	template <typename T>
	void RegisterComponent(Entity& _entity);

	template <typename T>
	T* RegisterComponent(Entity& _entity, const T& _initValue);

	/** Removes a component registered to the input Entity object. Component type is specified by using template arguments.
	Ex. RemoveComponent<ComponentX>(_entityX) will remove a registered component of type ComponentX.

	Nothing will happen if the input Entity doesn't have a component of the specified type registered for it.
	@param _entity The Entity to remove the component for
	@param templateArg Specifies which component type to remove
	@return void
	*/
	template <typename T>
	void RemoveComponent(Entity& _entity);

	template <typename T>
	void UpdateComponent(Entity _entity, const T& _component)
	{
		if constexpr (std::is_same_v<T, Transform>)
		{
			if (_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
				*transformMap.GetObjectByID(_entity.id) = _component;
		}
		else if constexpr (std::is_same_v<T, Mesh>)
		{
			if (_entity.componentMask.test(COMPONENTENUM::MESH))
				*meshMap.GetObjectByID(_entity.id) = _component;
		}
		else if constexpr (std::is_same_v<T, MaterialComponent>)
		{
			if (_entity.componentMask.test(COMPONENTENUM::MATERIAL))
				*materialMap.GetObjectByID(_entity.id) = _component;
		}
		else if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if (_entity.componentMask.test(COMPONENTENUM::PLIGHT))
				*pLightMap.GetObjectByID(_entity.id) = _component;
		}
		else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
		{
			if (_entity.componentMask.test(COMPONENTENUM::DLIGHT))
				*dLightMap.GetObjectByID(_entity.id) = _component;
		}
	}

	/** Returns a pointer to the newly registered component within a internal std::vector.
	The template argument specifies what type of component that will be returned.
	@param _entity The Entity to get the component for
	@param templateArg Specifies which component type to return
	@return Pointer to the internal component object
	@return Nullptr if the input Entity doesn't have a component of the specified type registered for it.
	*/
	template <typename T>
	T* GetComponent(const Entity& _entity);


	/** Returns a pointer to the newly registered component within a internal std::vector.
	The template argument specifies what type of component that will be returned.

	NOTE!!!!
	Be careful when using this method. The application will crash if the input Entity doesn't have a component of the specified type registered.
	Because of this, only use this function when efficiency is the prioriy and you're sure the Entity has the specified component registered.
	@param _entity The Entity to get the component for
	@param templateArg Specifies which component type to return
	@return Pointer to the internal component object
	*/
	template <typename T>
	T* GetComponentFast(const Entity& _entity);
};

/** @brief An abstract base class for systems used within the ECS framework. New systems should inherit from this and implement appropriate functionality for the ECSystem::Update() pure virtual method.
*/
class ECSystem
{
protected:
	SlottedMap<Entity> entityIDMap; /**< A bi-directional map containing an internal std::vector of copies of bound Entity objects.*/
	ComponentManager& componentManager;
public:

	std::bitset<MAXCOMPONENTS> componentMask; /**<Describes what type of components a bound Entity should have registered. This should be overwritten in the constructor of an inheriting class.*/

	ECSystem(ComponentManager& _componentManager)
		:componentManager(_componentManager)
	{

	}

	/**Used to bind an Entity object to be used within the subclasses' implementation of the ECSystem::Update() pure virtual method.
	@param _entity The Entity to bind to the ECSystem
	@return void
	*/
	virtual bool Bind(const Entity& _entity)
	{
		// Note - CHANGE THIS TO BITWISE OPERATOR... HOW TO DO THAT WHEN U WANNA CHECK FOR PATTERN?
		for (int i = 0; i < MAXCOMPONENTS; ++i)
		{
			if (componentMask.test(i))
			{
				if (!_entity.componentMask.test(i)) // No binding since Entity doesn't have that component
				{
					printf("No such component registered!\n");
					return false;
				}
			}
		}

		if (entityIDMap.Exists(_entity.id))
			return true;

		entityIDMap.Add(_entity.id, _entity);

		return true;
	}

	/**Used to bind an Entity object to be used within the subclasses' implementation of the ECSystem::Update() pure virtual method.

	NOTE!!!!
	Be careful when using this since there is no check if the input Entity object has the required components registered.
	@param _entity The Entity to bind to the ECSystem
	@return void
	*/
	virtual void BindFast(const Entity& _entity)
	{
		entityIDMap.Add(_entity.id, _entity);
	}

	/**Used to unbind an Entity object from the ECSystem.
	@param _entity The Entity to unbind from the ECSystem
	@return void
	*/
	virtual bool UnBind(const Entity& _entity)
	{
		if (!entityIDMap.Exists(_entity.id))
			return false;
		entityIDMap.Remove(_entity.id);
		return true;
	}

	virtual void UnBindFast(const Entity& _entity)
	{
		entityIDMap.Remove(_entity.id);
	}

	/** @brief A pure virtual function that should be implemented for an inheriting subclass. It should operate on the bound Entity objects, but that isn't mandatory.
	*/
	virtual void Update() = 0;
};

/** @brief Used to generate new Entity objects.
*/
class EntityManager
{
private:
	unsigned int maxEntities;
	std::queue<int> freeIDs;
public:
	/** Initiates free Entity object IDs.
	@param _maxEntities Specifies how many Entity IDs that will be generated.
	*/
	EntityManager(unsigned int _maxEntities)
	{
		maxEntities = _maxEntities;

		// Note - Expensive...
		for (unsigned int i = 0; i < _maxEntities; ++i)
		{
			freeIDs.emplace(i);
		}
	}

	/** Create and returns a new unique Entity object.
	@return Copy of the newly created Entity object
	*/
	Entity CreateEntity()
	{
		Entity entity;
		if (freeIDs.empty())
		{
			throw;
		}
		entity.id = freeIDs.front();
		freeIDs.pop();
		return entity;
	}

	/** Recycles the Entity by resetting the Entity::componentMask and enabling the Entity::id to be reused using the EntityManager::CreateEntity() method.
	@param _entity The Entity to recycle
	@return void
	*/
	void RemoveEntity(Entity& _entity)
	{
		freeIDs.push(_entity.id);
		_entity.id = -1;
		_entity.componentMask.reset();
	}

	/** Generates new IDs to give new Entity objects returned by EntityManager::CreateEntity().
	@param _amount How many IDs to generate
	@return void
	*/
	void Expand(unsigned int _amount)
	{
		int lastID = maxEntities;
		maxEntities += _amount;
		for (unsigned int i = 0; i < _amount; ++i)
		{
			freeIDs.emplace(lastID + i);
		}
	}
};

class SystemManager
{
private:
	std::unordered_map<std::string, std::shared_ptr<ECSystem>>systems;
public:
	SystemManager() = default;

	template<typename T>
	std::shared_ptr<T> RegisterSystem(ComponentManager& _componentManager)
	{
		const std::string sysName(typeid(T).name());
		if (systems.count(sysName) > 0)
			return std::make_shared<T>(_componentManager);

		auto newSystem = std::make_shared<T>(_componentManager);
		systems.insert({ sysName, newSystem });
		return newSystem;
	}

	template<typename T>
	void UnregisterSystem()
	{
		const std::string sysName(typeid(T).name());
		if (systems.count(sysName) == 0)
			return;

		systems.erase(sysName);
	}

	void EntityModified(Entity _entity)
	{
		for (auto& system : systems)
		{
			bool bound = system.second->Bind(_entity);
			if (!bound)
			{
				system.second->UnBind(_entity);
			}
		}
	}
};
/** @brief Contains everything used to handle a singular Entity Component System.
Enables creation and management of Entity objects and components through the internal EntityManager and ComponentManager member variables used
via the ECS::GetEntityManager() and ECS::GetComponentManager() methods.

NOTE!!!!
Custom made ECSystem subclasses has to be added manually to this class. ECSystem::UnBind() also has to be called within ECS::ObliterateEntity(), otherwise it won't be unbound from that ECSystem.
Same goes for components. See the commented examples within this classes source code.
*/
class ECS
{
private:
	ComponentManager componentManager; /**< Is used internally to store and handle all the components registered. Used by the different ECSystem subclass member variables.*/
	EntityManager entityManager; /**< Is used internally to store and generate new Entity objects.*/
	SystemManager systemManager;

public:
	/** Initiates the internal EntityManager variable with the input value.
	@param _maxEntities Maximum number of Entity objects that the ECS instance can contain
	*/
	ECS(unsigned int _maxEntities)
		:entityManager(_maxEntities)
	{

	}

	~ECS() = default;

	/** Returns a reference to a internal EntityManager object.
	@return Reference to a internal EntityManager object
	*/
	//EntityManager& GetEntityManager() { return entityManager; }

	/** Returns a reference to a internal ComponentManager object.
	@return Reference to a internal ComponentManager object
	*/
	ComponentManager& GetComponentManager() { return componentManager; }

	Entity CreateEntity(){return entityManager.CreateEntity(); }

	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return systemManager.RegisterSystem<T>(componentManager);
	}

	void ForceUpdate(Entity _entity)
	{
		systemManager.EntityModified(_entity);
	}

	/** Completely removes the Entity object, and everything linked to it, from the ECS instance by calling ComponentManager::RemoveComponent(), ECSystem::UnBind(), and EntityManager::RemoveEntity().
	@param _entity The Entity to obliterate
	@return void
	*/
	void ObliterateEntity(Entity& _entity)
	{
		componentManager.RemoveComponent<Transform>(_entity);

		componentManager.RemoveComponent<Mesh>(_entity);

		componentManager.RemoveComponent<MaterialComponent>(_entity);

		componentManager.RemoveComponent<PointLightComponent>(_entity);

		componentManager.RemoveComponent<DirectionalLightComponent>(_entity);

		systemManager.EntityModified(_entity);

		entityManager.RemoveEntity(_entity);
	}

	template<typename T>
	void RegisterComponent(Entity& _entity)
	{
		componentManager.RegisterComponent<T>(_entity);
	}

	template<typename T>
	void RegisterComponent(Entity& _entity, const T& _data)
	{
		componentManager.RegisterComponent(_entity, _data);
		systemManager.EntityModified(_entity);
	}

	template<typename T>
	void UnregisterComponent(Entity& _entity)
	{
		componentManager.RemoveComponent<T>(_entity);
		systemManager.EntityModified(_entity);
	}

	template<typename T>
	void UpdateComponent(Entity _entity, const T& _component)
	{
		componentManager.UpdateComponent(_entity, _component);
	}

	template<typename T>
	T* GetComponent(const Entity& _entity)
	{
		return componentManager.GetComponent<T>(_entity);
	}
};

template<typename T>
inline void ComponentManager::RegisterComponent(Entity& _entity)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
		{
			_entity.componentMask.set(COMPONENTENUM::TRANSFORM);
			transformMap.Add(_entity.id, Transform());
		}
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MESH))
		{
			_entity.componentMask.set(COMPONENTENUM::MESH);
			meshMap.Add(_entity.id, Mesh());
		}
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MATERIAL))
		{
			_entity.componentMask.set(COMPONENTENUM::MATERIAL);
			materialMap.Add(_entity.id, MaterialComponent());
		}
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::PLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::PLIGHT);
			pLightMap.Add(_entity.id, PointLightComponent());
		}
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::DLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::DLIGHT);
			dLightMap.Add(_entity.id, DirectionalLightComponent());
		}
	}
}

template<typename T>
inline T* ComponentManager::RegisterComponent(Entity& _entity, const T& _data)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
		{
			_entity.componentMask.set(COMPONENTENUM::TRANSFORM);
			transformMap.Add(_entity.id, _data);
			return transformMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MESH))
		{
			_entity.componentMask.set(COMPONENTENUM::MESH);
			meshMap.Add(_entity.id, _data);
			return meshMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MATERIAL))
		{
			_entity.componentMask.set(COMPONENTENUM::MATERIAL);
			materialMap.Add(_entity.id, _data);
			return materialMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::PLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::PLIGHT);
			pLightMap.Add(_entity.id, _data);
			return pLightMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::DLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::DLIGHT);
			dLightMap.Add(_entity.id, _data);
			return dLightMap.GetObjectByID(_entity.id);
		}
	}

	return nullptr;
}

template<typename T>
inline void ComponentManager::RemoveComponent(Entity& _entity)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
		{
			_entity.componentMask.set(COMPONENTENUM::TRANSFORM, false);
			transformMap.Remove(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::MESH))
		{
			_entity.componentMask.set(COMPONENTENUM::MESH, false);
			meshMap.Remove(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::MATERIAL))
		{
			_entity.componentMask.set(COMPONENTENUM::MATERIAL, false);
			materialMap.Remove(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::PLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::PLIGHT, false);
			pLightMap.Remove(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::DLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::DLIGHT, false);
			dLightMap.Remove(_entity.id);
		}
	}
}

template<typename T>
inline T* ComponentManager::GetComponent(const Entity& _entity)
{
	if constexpr (std::is_same_v<T, Transform>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
			return transformMap.GetObjectByID(_entity.id);
		else
			return nullptr;
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::MESH))
			return meshMap.GetObjectByID(_entity.id);
		else
			return nullptr;
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::MATERIAL))
			return materialMap.GetObjectByID(_entity.id);
		else
			return nullptr;
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::PLIGHT))
			return pLightMap.GetObjectByID(_entity.id);
		else
			return nullptr;
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		if (_entity.componentMask.test(COMPONENTENUM::DLIGHT))
			return dLightMap.GetObjectByID(_entity.id);
		else
			return nullptr;
	}

	return nullptr;
}

template<typename T>
inline T* ComponentManager::GetComponentFast(const Entity& _entity)
{
	// ---------------	SOURCE CODE EXAMPLES	----------------------
	//if constexpr (std::is_same_v<T, Comp1>)
	//{
	//	return &comp1Map.objects[comp1Map.idToIndex.at(_entity.id)];
	//}
	//else if constexpr (std::is_same_v<T, Comp2>)
	//{
	//	return &comp2Map.objects[comp2Map.idToIndex.at(_entity.id)];
	//}
	//else if constexpr (std::is_same_v<T, Comp3>)
	//{
	//	return &comp3Map.objects[comp3Map.idToIndex.at(_entity.id)];
	//}
	// ---------------------------------------------------------------

	if constexpr (std::is_same_v<T, Transform>)
	{
		return transformMap.GetObjectByID(_entity.id);
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		return meshMap.GetObjectByID(_entity.id);
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		return materialMap.GetObjectByID(_entity.id);
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		return pLightMap.GetObjectByID(_entity.id);
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		return dLightMap.GetObjectByID(_entity.id);
	}

	return nullptr;
}
