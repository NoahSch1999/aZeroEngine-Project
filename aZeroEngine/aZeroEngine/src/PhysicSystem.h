#pragma once
#include "ECS.h"
#include <unordered_map>
#include "aZeroModelParsing/ModelParser.h"

class PhysicSystem : public aZeroECS::System
{
private:
	reactphysics3d::PhysicsWorld* m_world = nullptr;
	reactphysics3d::PhysicsCommon m_physicsCommon;
	bool m_simulatePhysics = false;

	reactphysics3d::DebugRenderer* m_debugRenderer = nullptr;

	std::unordered_map<std::string, std::unique_ptr<aZeroFiles::LoadedModelContainer>> m_loadedConvexColliders;

public:

	enum class COLLIDERTYPE { SPHERE, BOX, CAPSULE, CONVEX };

	PhysicSystem(aZeroECS::ComponentManager& _componentManager)
		:aZeroECS::System(_componentManager)
	{
		m_componentMask.set(m_componentManager.GetComponentBit<Transform>());
		m_componentMask.set(m_componentManager.GetComponentBit<RigidBody>());

		reactphysics3d::PhysicsWorld::WorldSettings settings;
		settings.isSleepingEnabled = false;
		settings.gravity = reactphysics3d::Vector3(0, -0.001, 0);
		m_world = m_physicsCommon.createPhysicsWorld(settings);

#ifdef _DEBUG
		m_world->setIsDebugRenderingEnabled(true);

		m_debugRenderer = &m_world->getDebugRenderer();

		m_debugRenderer->setIsDebugItemDisplayed(reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
#endif // _DEBUG
	}

	virtual ~PhysicSystem()
	{
		m_physicsCommon.destroyPhysicsWorld(m_world);
	}

	virtual bool Bind(aZeroECS::Entity& _entity) override
	{
		if (aZeroECS::System::Bind(_entity))
		{
			RigidBody& rbComponent = m_componentManager.GetComponentFast<RigidBody>(_entity);

			if (!rbComponent.m_body)
			{
				Transform& transformComponent = m_componentManager.GetComponentFast<Transform>(_entity);

				reactphysics3d::Transform bodyTransform = reactphysics3d::Transform::identity();
				reactphysics3d::Vector3 startPosition;
				startPosition.x = transformComponent.GetTranslation().x;
				startPosition.y = transformComponent.GetTranslation().y;
				startPosition.z = transformComponent.GetTranslation().z;
				bodyTransform.setPosition(startPosition);

				reactphysics3d::Quaternion startRotation;
				startRotation = reactphysics3d::Quaternion::fromEulerAngles(transformComponent.GetRotation().x, transformComponent.GetRotation().y, transformComponent.GetRotation().z);
				bodyTransform.setOrientation(startRotation);

				rbComponent.m_body = m_world->createRigidBody(bodyTransform);
			}

			return true;
		}

		return false;
	}

	virtual bool UnBind(const aZeroECS::Entity& _entity) override
	{
		RigidBody* rbComponent = m_componentManager.GetComponent<RigidBody>(_entity);
		if (rbComponent)
		{
			if (rbComponent->m_body)
			{
				m_world->destroyRigidBody(rbComponent->m_body);
				rbComponent->m_body = nullptr;
			}
		}

		return aZeroECS::System::UnBind(_entity);
	}

	virtual void Update() override
	{
		if (m_simulatePhysics)
		{
			aZeroECS::ComponentArray<Transform>& transformArray = m_componentManager.GetComponentArray<Transform>();
			aZeroECS::ComponentArray<RigidBody>& bodyArray = m_componentManager.GetComponentArray<RigidBody>();

			// Execute physics simulation
			// TO DO: Add so it takes delta time into account
			reactphysics3d::decimal timeStep = 1.0f / 60.f;
			m_world->update(timeStep);

			// Apply physics simulation
			for (const aZeroECS::Entity& entity : m_entities.GetObjects())
			{
				Transform& transformComponent = transformArray.GetComponentFast(entity);
				RigidBody& bodyComponent = bodyArray.GetComponentFast(entity);
				reactphysics3d::Transform bodyTransform = bodyComponent.m_body->getTransform();

				// TO DO: Make the starting rotation the same as the transform...
				reactphysics3d::Vector3 rotation = bodyTransform.getOrientation().getVectorV();
				transformComponent.GetRotation().x = rotation.x;
				transformComponent.GetRotation().y = rotation.y;
				transformComponent.GetRotation().z = rotation.z;

				reactphysics3d::Vector3 position = bodyTransform.getPosition();
				transformComponent.GetTranslation().x = position.x;
				transformComponent.GetTranslation().y = position.y;
				transformComponent.GetTranslation().z = position.z;
			}
		}
	}

	void addBoxCollider(RigidBody& component, const std::string& colliderName)
	{
		reactphysics3d::BoxShape* shape = m_physicsCommon.createBoxShape({ 0.01f, 0.01f, 0.01f });
		reactphysics3d::Collider* collider = component.m_body->addCollider(shape, reactphysics3d::Transform::identity());
		component.m_colliders.emplace(colliderName, collider);
	}

	void addSphereCollider(RigidBody& component, const std::string& colliderName)
	{
		reactphysics3d::SphereShape* shape = m_physicsCommon.createSphereShape(0.01f);
		reactphysics3d::Collider* collider = component.m_body->addCollider(shape, reactphysics3d::Transform::identity());
		component.m_colliders.emplace(colliderName, collider);
	}

	void addCapsuleCollider(RigidBody& component, const std::string& colliderName)
	{
		reactphysics3d::CapsuleShape* shape = m_physicsCommon.createCapsuleShape(0.01f, 0.07f);
		reactphysics3d::Collider* collider = component.m_body->addCollider(shape, reactphysics3d::Transform::identity());
		component.m_colliders.emplace(colliderName, collider);
	}

	// TO-DO: Setup faces
	void addConvexCollider(RigidBody& component, const std::string& colliderName, const std::string& modelName)
	{
		if (m_loadedConvexColliders.count(modelName) == 0)
		{
			std::optional<std::unique_ptr<aZeroFiles::LoadedModelContainer>> model = aZeroFiles::LoadAZModel("../meshes/", modelName);
			if (!model.has_value())
				return;

			m_loadedConvexColliders.emplace(modelName, std::make_unique<aZeroFiles::LoadedModelContainer>(std::move(*model.value().get())));
		}

		aZeroFiles::LoadedModelContainer* convexData = m_loadedConvexColliders.at(modelName).get();
		
		reactphysics3d::uint32 numVerts = convexData->m_numVertices;
		reactphysics3d::uint32 vertexStride = sizeof(float) * 3;
		reactphysics3d::uint32 indexStride = sizeof(int);
		reactphysics3d::uint32 nbFaces = convexData->m_numVertices / 3;

		reactphysics3d::PolygonVertexArray data(
			numVerts, 
			convexData->m_rawVertexData,
			vertexStride, 
			convexData->m_rawIndexData, 
			indexStride, 
			nbFaces,
			(reactphysics3d::PolygonVertexArray::PolygonFace*)convexData->m_rawVertexData,
			reactphysics3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
			reactphysics3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

		reactphysics3d::PolyhedronMesh* pMesh = m_physicsCommon.createPolyhedronMesh(&data);
		reactphysics3d::ConvexMeshShape* shape = m_physicsCommon.createConvexMeshShape(pMesh);
		reactphysics3d::Collider* collider = component.m_body->addCollider(shape, reactphysics3d::Transform::identity());
		component.m_colliders.emplace(colliderName, collider);
	}

	void removeCollider(RigidBody& component, const std::string& colliderName)
	{
		component.m_body->removeCollider(component.m_colliders[colliderName]);
		component.m_colliders.erase(colliderName);
	}

	void setGravity(float gravity) { m_world->setGravity(reactphysics3d::Vector3(0, gravity, 0)); }
	float getGravity() const { return m_world->getGravity().y; }

	void simulatePhysics(bool onOff)
	{
		m_simulatePhysics = onOff;
	}
};