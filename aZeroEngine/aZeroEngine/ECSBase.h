/*
This code was created by me, Noah. It's my first ECS, so not very good. But if you wanna use it, feel free to do so!

In the code you will see comments containing " ---------------	SOURCE CODE EXAMPLES	---------------------- ". These contain
examples of what you, the developer, has to implement for each component and ECSystem subclass you want to use.

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

class BaseResource;
class VertexBuffer;
class ConstantBuffer;

// COMPONENTS
/**
Maximum components per Entity
*/
static const int MAXCOMPONENTS = 10;

/** @brief Component enumeration for usage in conjunction with the Entity std::bitset.
* You should create an enumeration for your new custom components. The enum should be used within ComponentManager and its methods.
*/
namespace COMPONENTENUM
{	
	enum COMPONENTBITID { TRANSFORM, MESH, MATERIAL, PLIGHT, DLIGHT };
}

// ---------------	SOURCE CODE EXAMPLES	----------------------
//struct Comp1
//{
//	int age;
//	std::string name;
//	bool male;
//};
//
///// \private
//struct Comp2
//{
//	std::string x;
//};
//
///// \private
//struct Comp3
//{
//	int x;
//};
// ---------------------------------------------------------------

class Transform
{
private:
	Vector3 translation = Vector3::Zero;
	Vector3 rotation = Vector3::Zero;
	Vector3 scale = Vector3(1.f, 1.f, 1.f);
	ConstantBuffer cb;

public:

	void SetTranslation(const Vector3& _translation)
	{
		translation = _translation;
	}

	void SetTranslation(float _xPos, float _yPos, float _zPos)
	{
		translation.x = _xPos;
		translation.y = _yPos;
		translation.z = _zPos;
	}

	void SetTranslationX(float _xPos)
	{
		translation.x = _xPos;
	}

	void SetTranslationY(float _yPos)
	{
		translation.y = _yPos;
	}

	void SetTranslationZ(float _zPos)
	{
		translation.z = _zPos;
	}

	void SetRotation(const Vector3& _rotation)
	{
		rotation = _rotation;
	}

	void SetRotation(float _xRot, float _yRot, float _zRot)
	{
		rotation.x = _xRot;
		rotation.y = _yRot;
		rotation.z = _zRot;
	}

	void SetRotationX(float _xRotDegree)
	{
		rotation.x = _xRotDegree;
	}

	void SetRotationY(float _yRotDegree)
	{
		rotation.y = _yRotDegree;
	}

	void SetRotationZ(float _zRotDegree)
	{
		rotation.z = _zRotDegree;
	}

	void SetScale(const Vector3& _scale)
	{
		scale = _scale;
	}

	void SetScale(float _xScale, float _yScale, float _zScale)
	{
		scale.x = _xScale;
		scale.y = _yScale;
		scale.z = _zScale;
	}

	void SetScaleX(float _xScale)
	{
		scale.x = _xScale;
	}

	void SetScaleY(float _yScale)
	{
		scale.y = _yScale;
	}

	void SetScaleZ(float _zScale)
	{
		scale.z = _zScale;
	}

	void SetScaleUniform(float _scale)
	{
		scale = Vector3(_scale, _scale, _scale);
	}

	Vector3& GetTranslation() { return translation; }
	Vector3& GetRotation() { return rotation; }
	Vector3& GetScale() { return scale; }

	Matrix matrix = Matrix::Identity;

	Matrix& Compose()
	{
		matrix = (Matrix::CreateScale(scale)) * Matrix::CreateFromYawPitchRoll(rotation) * Matrix::CreateTranslation(translation);
		return matrix;
	}

	void Delete()
	{
		cb.GetIntermediateResource()->Release();
		cb.GetMainResource()->Release();
	}

	ConstantBuffer& GetBuffer() { return cb; }

	Transform() = default;

	Transform(ID3D12Device* _device, ResourceEngine& _resourceEngine)
	{
		Matrix temp = Matrix::Identity;
		_resourceEngine.CreateResource(_device, cb, (void*)&temp, sizeof(Matrix), true, true);
	}

	// Used for non-trippleframed resources
	void Update(ResourceEngine& _resourceEngine, int _frameIndex)
	{
		Matrix temp = Compose();
		_resourceEngine.Update(cb, (void*)&temp, _frameIndex);
	}
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
	float receiveShadows = 1.f;
};

enum MATERIALTYPE { PHONG, PBR };

struct MaterialComponent
{
public:
	MaterialComponent() = default;
	MaterialComponent(int _materialID) :materialID(_materialID) {};

	int materialID = -1;
	MATERIALTYPE type = MATERIALTYPE::PHONG;
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
	bool disabled = false;
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

	SlottedMap() = default;

	void Insert(int _id, const T& _value);

	void Remove(int _id);

	void Clear();

	bool Exists(int _id)
	{
		return idToIndex.count(_id) > 0;
	}

	T* GetObjectByID(int _id){ return &objects[idToIndex.at(_id)]; }

	std::vector<T>& GetObjects() { return objects; }
};

/** @brief A data structure which contains a contiguous array of elements that can be accessed through an ID (int) or key(string).
*/
template <typename T>
class NamedSlottedMap
{
private:
	SlottedMap<T>map;
	int currentMax = 0;
	int incPerEmpty = 0;
	std::deque<int> freeIDs;
	std::unordered_map<std::string, int>strToID;
	std::unordered_map<int, std::string>IDtoStr;
public:
	NamedSlottedMap(int _startMax, int _incPerEmpty = 100)
		:currentMax(_startMax), incPerEmpty(_incPerEmpty)
	{
		for (int i = 0; i < currentMax; i++)
		{
			freeIDs.push_back(i);
		}
	}

	void Reset()
	{
		freeIDs.clear();
		for (int i = 0; i < currentMax; i++)
		{
			freeIDs.push_back(i);
		}
		strToID.clear();
		IDtoStr.clear();
		map.Clear();
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

		if (freeIDs.empty())
		{
			for (int i = currentMax; i < currentMax + incPerEmpty; i++)
			{
				freeIDs.push_back(i);
			}
		}

		int id = freeIDs.front();
		freeIDs.pop_front();

		map.Insert(id, _data);

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
		freeIDs.push_front(id);
	}

	void Remove(int _key)
	{
		if (!map.Exists(_key))
			return;

		map.Remove(_key);
		strToID.erase(IDtoStr.at(_key));
		IDtoStr.erase(_key);
		freeIDs.push_front(_key);
	}

	bool Exists(const std::string& _key)
	{
		if (strToID.count(_key) > 0)
			return true;

		return false;
	}

	bool Exists(int _key)
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

	std::string GetString(int _key)
	{
		if (IDtoStr.count(_key) == 0)
			return "";

		return IDtoStr.at(_key);
	}
};

/** @brief An abstract base class for systems used within the ECS framework. New systems should inherit from this and implement appropriate functionality for the ECSystem::Update() pure virtual method.
*/
class ECSystem
{
protected:
	SlottedMap<Entity> entityIDMap; /**< A bi-directional map containing an internal std::vector of copies of bound Entity objects.*/
public:

	std::bitset<MAXCOMPONENTS> componentMask; /**<Describes what type of components a bound Entity should have registered. This should be overwritten in the constructor of an inheriting class.*/
	
	ECSystem() = default;

	/**Used to bind an Entity object to be used within the subclasses' implementation of the ECSystem::Update() pure virtual method.
	@param _entity The Entity to bind to the ECSystem
	@return void
	*/
	void Bind(const Entity& _entity)
	{
		if (entityIDMap.Exists(_entity.id))
			return;
		// Note - CHANGE THIS TO BITWISE OPERATOR... HOW TO DO THAT WHEN U WANNA CHECK FOR PATTERN?
		for (int i = 0; i < MAXCOMPONENTS; ++i)
		{
			if (componentMask.test(i))
			{
				if (!_entity.componentMask.test(i)) // No binding since Entity doesn't have that component
				{
					printf("No such component registered!\n");
					return;
				}
			}
		}

		entityIDMap.Insert(_entity.id, _entity);

		return;
	}

	/**Used to bind an Entity object to be used within the subclasses' implementation of the ECSystem::Update() pure virtual method.
	
	NOTE!!!!
	Be careful when using this since there is no check if the input Entity object has the required components registered.
	@param _entity The Entity to bind to the ECSystem
	@return void
	*/
	void BindFast(const Entity& _entity)
	{
		entityIDMap.Insert(_entity.id, _entity);
	}

	/**Used to unbind an Entity object from the ECSystem.
	@param _entity The Entity to unbind from the ECSystem
	@return void
	*/
	void UnBind(const Entity& _entity)
	{
		if (!entityIDMap.Exists(_entity.id))
			return;
		entityIDMap.Remove(_entity.id);
	}

	void UnBindFast(const Entity& _entity)
	{
		entityIDMap.Remove(_entity.id);
	}

	/**Clears the list of Entity objects bound to the system.
	@return void
	*/
	void RemoveEntities()
	{
		entityIDMap.Clear();
	}

	/** @brief A pure virtual function that should be implemented for an inheriting subclass. It should operate on the bound Entity objects, but that isn't mandatory.
	*/
	virtual void Update() = 0;
};

// ---------------	SOURCE CODE EXAMPLES	----------------------
//class TestSystem : public ECSystem
//{
//public:
//	TestSystem()
//		:ECSystem()
//	{
//		// Note - CHANGE THIS TO BITWISE OPERATOR
//		componentMask.set(0, false);
//		componentMask.set(1, true);
//		componentMask.set(2, true);
//	}
//
//	// Inherited via ECSystem
//	virtual void Update() override;
//
//};
	// -----------------------------------------------------------

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
	// ---------------	SOURCE CODE EXAMPLES	----------------------
	// Add custom component bidirectional maps here
	//BiDirectionalMap<Comp1>comp1Map;
	//BiDirectionalMap<Comp2>comp2Map;
	//BiDirectionalMap<Comp3>comp3Map;
	// ---------------------------------------------------------------
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

		// Note - Expensive as fuck...
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
	EntityManager& GetEntityManager() { return entityManager; }

	/** Returns a reference to a internal ComponentManager object.
	@return Reference to a internal ComponentManager object
	*/
	ComponentManager& GetComponentManager() { return componentManager; }

	/** Completely removes the Entity object, and everything linked to it, from the ECS instance by calling ComponentManager::RemoveComponent(), ECSystem::UnBind(), and EntityManager::RemoveEntity().
	@param _entity The Entity to obliterate
	@return void
	*/
	void ObliterateEntity(Entity& _entity)
	{
		
		// Call for each Component
		//componentManager.RemoveComponent<Comp1>(_entity); // Works regardless if it's bound or not
		//componentManager.RemoveComponent<Comp2>(_entity); // Works regardless if it's bound or not
		//componentManager.RemoveComponent<Comp3>(_entity); // Works regardless if it's bound or not

		// Call for each ECSystem
		//tSystem.UnBind(_entity); // Works regardless if it's bound or not

		componentManager.RemoveComponent<Transform>(_entity);

		componentManager.RemoveComponent<Mesh>(_entity);

		componentManager.RemoveComponent<MaterialComponent>(_entity);

		componentManager.RemoveComponent<PointLightComponent>(_entity);

		componentManager.RemoveComponent<DirectionalLightComponent>(_entity);

		entityManager.RemoveEntity(_entity);
	}

	// Systems
	//TestSystem tSystem; // Example of adding a custom ECSystem
};

template<typename T>
inline void SlottedMap<T>::Insert(int _id, const T& _value)
{
	if (Exists(_id))
		return;

	int index = objects.size();
	objects.emplace_back(_value);
	idToIndex.emplace(_id, index);
	indexToId[index] = _id;
}

template<typename T>
inline void SlottedMap<T>::Remove(int _id)
{
	if (!Exists(_id))
		return;

	int index = idToIndex.at(_id);
	idToIndex.erase(_id);

	if (objects.size() == 1)
	{
		indexToId.erase(index);
		objects.resize(0);
		return;
	}

	int indexOfLast = objects.size() - 1;

	if (indexOfLast == index)
	{
		indexToId.erase(index);
		objects.resize(objects.size() - 1);
		return;
	}

	indexToId.at(index) = indexToId.at(indexOfLast);
	objects[index] = objects[indexOfLast];
	int idToMove = indexToId.at(indexOfLast);
	idToIndex.at(indexToId.at(indexOfLast)) = index;
	objects.resize(objects.size() - 1);
}

template<typename T>
inline void SlottedMap<T>::Clear()
{
	idToIndex.clear();
	indexToId.clear();
	objects.clear();
	objects.resize(0);
}

template<typename T>
inline T* ComponentManager::RegisterComponent(Entity& _entity, const T& _initValue)
{
	// ---------------	SOURCE CODE EXAMPLES	----------------------
	//if constexpr (std::is_same_v<T, Comp1>)
	//{
	//	if (!_entity.componentMask.test(COMPONENTENUM::COMP1))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP1);
	//		comp1Map.Insert(_entity, _initValue);
	//		return comp1Map.GetObjectByID(_entity.id);
	//	}
	//}
	//else if constexpr (std::is_same_v<T, Comp2>)
	//{
	//	if (!_entity.componentMask.test(COMPONENTENUM::COMP2))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP2);
	//		comp2Map.Insert(_entity, _initValue);
	//		return comp2Map.GetObjectByID(_entity.id);
	//	}
	//}
	//else if constexpr (std::is_same_v<T, Comp3>)
	//{
	//	if (!_entity.componentMask.test(COMPONENTENUM::COMP3))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP3);
	//		comp3Map.Insert(_entity, _initValue);
	//		return comp3Map.GetObjectByID(_entity.id);
	//	}
	//}
	// ---------------------------------------------------------------
	if constexpr (std::is_same_v<T, Transform>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::TRANSFORM))
		{
			_entity.componentMask.set(COMPONENTENUM::TRANSFORM);
			transformMap.Insert(_entity.id, _initValue);
			return transformMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, Mesh>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MESH))
		{
			_entity.componentMask.set(COMPONENTENUM::MESH);
			meshMap.Insert(_entity.id, _initValue);
			return meshMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, MaterialComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::MATERIAL))
		{
			_entity.componentMask.set(COMPONENTENUM::MATERIAL);
			materialMap.Insert(_entity.id, _initValue);
			return materialMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, PointLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::PLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::PLIGHT);
			pLightMap.Insert(_entity.id, _initValue);
			return pLightMap.GetObjectByID(_entity.id);
		}
	}
	else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
	{
		if (!_entity.componentMask.test(COMPONENTENUM::DLIGHT))
		{
			_entity.componentMask.set(COMPONENTENUM::DLIGHT);
			dLightMap.Insert(_entity.id, _initValue);
			return dLightMap.GetObjectByID(_entity.id);
		}
	}

	return nullptr;
}

template<typename T>
inline void ComponentManager::RemoveComponent(Entity& _entity)
{
	// ---------------	SOURCE CODE EXAMPLES	----------------------
	//if constexpr (std::is_same_v<T, Comp1>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP1))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP1, false);
	//		comp1Map.Remove(_entity);
	//	}
	//}
	//else if constexpr (std::is_same_v<T, Comp2>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP2))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP2, false);
	//		comp2Map.Remove(_entity);
	//	}
	//}
	//else if constexpr (std::is_same_v<T, Comp3>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP3))
	//	{
	//		_entity.componentMask.set(COMPONENTENUM::COMP3, false);
	//		comp3Map.Remove(_entity);
	//	}
	//}
	// ---------------------------------------------------------------

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
	// ---------------	SOURCE CODE EXAMPLES	----------------------
	//if constexpr (std::is_same_v<T, Comp1>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP1))
	//		return &comp1Map.objects[comp1Map.idToIndex.at(_entity.id)];
	//	else
	//		return nullptr;
	//}
	//else if constexpr (std::is_same_v<T, Comp2>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP2))
	//		return &comp2Map.objects[comp2Map.idToIndex.at(_entity.id)];
	//	else
	//		return nullptr;
	//}
	//else if constexpr (std::is_same_v<T, Comp3>)
	//{
	//	if (_entity.componentMask.test(COMPONENTENUM::COMP3))
	//		return &comp3Map.objects[comp3Map.idToIndex.at(_entity.id)];
	//	else
	//		return nullptr;
	//}
	// ---------------------------------------------------------------

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
