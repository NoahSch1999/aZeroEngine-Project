#pragma once
#include "Entity.h"
#include "MappedVector.h"

namespace aZeroECS
{
	class ComponentManager;

	/** @brief An abstract base class which can be inherited and registered with SystemManager::RegisterSystem().
	* Each inheriting subclass should specify what set of components an Entity needs to have to be bound when System::Bind() is called.
	* This can be done by writing "m_componentMask.set(m_componentManager.GetComponentBit<ComponentClassName>());" inside the subclass constructor.
	* A component have to be registered using ComponentManager::RegisterComponent() prior to a System subclass constructor being called for ComponentManager::GetComponentBit() to work.
	*/
	class System
	{
		friend class SystemManager;

	protected:
		std::bitset<MAXCOMPONENTS> m_componentMask;
		ComponentManager& m_componentManager;
		MappedVector<Entity> m_entities;

	public:
		System(ComponentManager& componentManager)
			:m_componentManager(componentManager) { }

		System(const System&) = delete;
		System(System&&) = delete;
		System operator=(const System&) = delete;
		System operator=(System&&) = delete;

		/** Binds the input Entity if it's m_componentMask matches the m_componentMask set in the System subclass constructor.
		@param entity The Entity which should be bound
		@return bool TRUE: Entity is either already bound or bound during this call of System::Bind(). FALSE: Entity::m_componentMask isn't compatible with System::m_componentMask
		*/
		virtual bool Bind(Entity& entity)
		{
			for (int i = 0; i < MAXCOMPONENTS; ++i)
			{
				if (m_componentMask.test(i))
				{
					if (!entity.m_componentMask.test(i))
					{
						return false;
					}
				}
			}

			if (m_entities.Contains(entity.m_id))
				return true;

			m_entities.Add(entity.m_id, std::move(entity));

			return true;
		}

		/** Unbinds the input Entity.
		@param entity The Entity which should be unbound
		@return bool TRUE: Entity was unbound. FALSE: Entity wasn't bound
		*/
		virtual bool UnBind(const Entity& entity)
		{
			if (!m_entities.Contains(entity.m_id))
				return false;

			m_entities.Remove(entity.m_id);

			return true;
		}

		/** Pure virtual method which should be overridden by the inheriting subclass.
		@return void
		*/
		virtual void Update() = 0;

		size_t numEntitiesBound() const { return m_entities.numObjects(); }
	};
}