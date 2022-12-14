#pragma once
#include "ECSBase.h"
#include "VertexBufferCache.h"

// REWORK THIS ENTIRE CLASS...!

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
public:
	std::unordered_map<std::string, Entity>entities;

	Scene() = default;

	/**Initiates the private member variables.
	@param _ecs Pointer to the ECS instance that should be used within its member functions
	@param _vbCache Pointer to the VertexBufferCache instance that should be used within its member functions
	*/
	Scene(ECS* _ecs, VertexBufferCache* _vbCache)
	{
		ecs = _ecs;
		eManager = &ecs->GetEntityManager();
		cManager = &ecs->GetComponentManager();
		vbCache = _vbCache;
	}

	/**Removes all the Entity objects within Scene::entities using ECS::ObliterateEntity()
	*/
	~Scene();

	/**Creates an Entity and inserts it into Scene::entities with a unique name and registers a Transform component with default values.
	@return void
	*/
	void CreateEntity()
	{
		Entity tempEnt = eManager->CreateEntity();
		cManager->RegisterComponent<Transform>(tempEnt, Transform());

		// Find unique name...
		std::string name("Entity_" + std::to_string(tempEnt.id));
		entities.emplace(name, tempEnt);
	}

	/**Registers a default Mesh component for the specified Entity object and binds the Entity to the appropriate systems.
	@param _entityName The Entity to register a mesh component for
	@return void
	*/
	void AddMeshToEntity(const std::string& _entityName)
	{
		if (entities.count(_entityName) > 0)
		{
			Mesh temp;
			temp.vBuffer = *vbCache->GetResource("cube");
			cManager->RegisterComponent<Mesh>(entities.at(_entityName), temp);

			// Add to systems...?

			// ...
		}
	}

	/**Removes a Mesh component for the specified Entity object and unbinds the Entity from the appropriate systems.
	@param _entityName The Entity to remove a Mesh component for
	@return void
	*/
	void RemoveMeshFromEntity(const std::string& _entityName)
	{
		if (entities.count(_entityName) > 0)
		{
			cManager->RemoveComponent<Mesh>(entities.at(_entityName));

			// Unregister from all systems using the Mesh component...
		
			// ...
		}
	}

	/**Changes the Entity objects Mesh component to the specified Mesh
	 
	NOTE!!!!
	The Mesh has to exist within Scene::VertexBufferCache. Otherwise nothing will happen.
	The specified Entity also has to have a Mesh registered prior to this function call.
	@param _entityName The Entity to change component for
	@param _meshName Filename for the new Mesh to apply
	@return void
	*/
	void ChangeMeshForEntity(const std::string& _entityName, const std::string& _meshName)
	{
		if (!vbCache->Exists(_meshName))
			return;

		if (entities.count(_entityName) > 0)
		{
			cManager->GetComponent<Mesh>(entities.at(_entityName))->vBuffer = *vbCache->GetResource(_meshName);

			// Add to systems for that specific Mesh...

			// ...
		}
	}
};