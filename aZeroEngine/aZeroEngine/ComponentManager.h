#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "SystemManager.h"

namespace aZeroECS
{
	class ComponentManager;

	struct ComponentArrayBase
	{
		ComponentArrayBase() = default;
		virtual ~ComponentArrayBase() = default;

		/** Pure virtual which is overriden by ComponentArray to allow MappedVector::Remove() without having to specify template parameters.
		@param entity The Entity to remove
		@return void
		*/
		virtual void RemoveComponent(Entity entity) = 0;
	};

	template<typename T>
	class ComponentArray : public ComponentArrayBase
	{
	private:
		MappedVector<T> m_components;

	public:
		ComponentArray() = default;

		ComponentArray(const ComponentArray&) = delete;
		ComponentArray(ComponentArray&&) = delete;
		ComponentArray operator=(const ComponentArray&) = delete;
		ComponentArray operator=(ComponentArray&&) = delete;

		void AddComponent(Entity entity, T&& component)
		{
			m_components.Add(entity.m_id, std::move(component));
		}

		/** Returns a pointer to the component of type T for the input Entity.
		* This method returns nullptr if the input Entity doesn't have a component of type T.
		@param entity The Entity to get the component for
		@return T* const
		*/
		T* const GetComponent(Entity entity)
		{
			return m_components.GetByID(entity.m_id);
		}

		/** Returns a reference to the component of type T for the input Entity.
		@param entity The Entity to get the component for
		@return T&
		*/
		T& const GetComponentFast(Entity entity)
		{
			return m_components.GetByIDFast(entity.m_id);
		}

		/** Removes the component for the input Entity.
		@param entity The Entity to remove
		@return void
		*/
		virtual void RemoveComponent(Entity entity) override
		{
			m_components.Remove(entity.m_id);
		}

		/** Returns number of components within the ComponentArray.
		@return int
		*/
		int numComponents() const { return m_components.numObjects(); }
	};

	/** @brief Used to store, manage, and register components.
	*/
	class ComponentManager
	{
	private:
		std::unordered_map<std::type_index, std::unique_ptr<ComponentArrayBase>> m_componentArrayMap;
		std::unordered_map<std::type_index, short> m_typeToBitflag;
		SystemManager& m_systemManager;

	public:
		ComponentManager(SystemManager& systemManager)
			:m_systemManager(systemManager) { }

		ComponentManager(const ComponentManager&) = delete;
		ComponentManager(ComponentManager&&) = delete;
		ComponentManager operator=(const ComponentManager&) = delete;
		ComponentManager operator=(ComponentManager&&) = delete;

		/** Registers a new component type of type T for the ComponentManager.
		* This should be called once for each component that the ComponentManager should support.
		@return void
		*/
		template<typename T>
		void RegisterComponent()
		{
			const std::type_index typeIndex = std::type_index(typeid(T));

			m_typeToBitflag.emplace(typeIndex, m_componentArrayMap.size());
			m_componentArrayMap.emplace(typeIndex, std::make_unique<ComponentArray<T>>());
		}

		/** Checks whether or not the input Entity currently has a component of type T.
		@param entity The Entity to check
		@return bool TRUE: The Entity has a component of type T, FALSE: The Entity doesn't have a component of type T
		*/
		template<typename T>
		bool HasComponent(const Entity& entity) const
		{
			return entity.m_componentMask.test(static_cast<size_t>(m_typeToBitflag.at(std::type_index(typeid(T)))));
		}

		/** Adds a component of type T to the input Entity.
		@param entity The Entity to add the component to
		@return void
		*/
		template<typename T>
		void AddComponent(Entity& entity)
		{
			entity.m_componentMask.set(static_cast<size_t>(m_typeToBitflag.at(std::type_index(typeid(T)))), true);

			ComponentArray<T>* const componentArray = static_cast<ComponentArray<T>*>(m_componentArrayMap.at(std::type_index(typeid(T))).get());

			componentArray->AddComponent(entity, std::move(T()));

			m_systemManager.EntityUpdated(entity);
		}

		/** Adds a component of type T to the input Entity.
		* Initiates the new component with the input data.
		@param entity The Entity to add the component to
		@param data Initial data for the component
		@return void
		*/
		template<typename T>
		void AddComponent(Entity& entity, T&& data)
		{
			entity.m_componentMask.set(static_cast<size_t>(m_typeToBitflag.at(std::type_index(typeid(T)))), true);

			ComponentArray<T>* const componentArray = static_cast<ComponentArray<T>*>(m_componentArrayMap.at(std::type_index(typeid(T))).get());

			componentArray->AddComponent(entity, std::move(data));

			m_systemManager.EntityUpdated(entity);
		}

		/** Returns a pointer to the component of type T for the input Entity.
		* Consider using ComponentManager::GetComponentArray<T>() in conjunctions with ComponentArray::GetComponent() or ComponentArray::GetComponentFast() to avoid additional lookup time per component.
		* This will avoid the additional ComponentArray<T> lookup time which this method has.
		* This method returns nullptr if the input Entity doesn't have a component of type T.
		* TODO: TRY TO AVOID GOING THROUGH THE MAP TO ACCESS THE COMPONENT ARRAY
		@param entity The Entity to get the component for
		@return T* const
		*/
		template<typename T>
		T* const GetComponent(const Entity& entity)
		{
			return static_cast<ComponentArray<T>*>(m_componentArrayMap.at(std::type_index(typeid(T))).get())->GetComponent(entity);
		}

		/** Returns a pointer to the component of type T for the input Entity.
		* Consider using ComponentManager::GetComponentArray<T>() in conjunctions with ComponentArray::GetComponent() or ComponentArray::GetComponentFast() to avoid additional lookup time per component.
		* This will avoid the additional ComponentArray<T> lookup time which this method has.
		* This method will might crash if the Entity doesn't have a component of type T. This avoid unneccessary if-statements. 
		* Consider using ComponentManager::GetComponent() or atleast ComponentManager::HasComponent<T>() if you want to avoid this.
		* TODO: TRY TO AVOID GOING THROUGH THE MAP TO ACCESS THE COMPONENT ARRAY
		@param entity The Entity to get the component for
		@return T&
		*/
		template<typename T>
		T& GetComponentFast(const Entity& entity)
		{
			return static_cast<ComponentArray<T>*>(m_componentArrayMap.at(std::type_index(typeid(T))).get())->GetComponentFast(entity);
		}

		/** Removes the component of type T for the input Entity.
		@param entity The Entity to remove the component for
		@return void
		*/
		template<typename T>
		void RemoveComponent(Entity& entity)
		{
			entity.m_componentMask.set(static_cast<size_t>(m_typeToBitflag.at(std::type_index(typeid(T)))), false);

			ComponentArrayBase* const base = m_componentArrayMap.at(std::type_index(typeid(T))).get();
			base->RemoveComponent(entity);

			m_systemManager.EntityUpdated(entity);
		}

		/** Removes the component of type T matching the input std::type_index for the input Entity.
		@param entity The Entity to remove the component for
		@param typeIndex The std::type_index which matches a registered type T component
		@return void
		*/
		void RemoveComponent(Entity& entity, std::type_index typeIndex)
		{
			entity.m_componentMask.set(static_cast<size_t>(m_typeToBitflag.at(typeIndex)), false);

			ComponentArrayBase* const base = m_componentArrayMap.at(typeIndex).get();
			base->RemoveComponent(entity);

			m_systemManager.EntityUpdated(entity);
		}

		/** Returns a reference to a ComponentArray<T>.
		@return ComponentArray<T>&
		*/
		template<typename T>
		ComponentArray<T>& GetComponentArray()
		{
			return *static_cast<ComponentArray<T>*>(m_componentArrayMap.at(std::type_index(typeid(T))).get());
		}

		/** Returns the std::bitset bit index for the component of type T.
		@return short
		*/
		template<typename T>
		short GetComponentBit() const
		{
			return m_typeToBitflag.at(std::type_index(typeid(T)));
		}

		/** Returns a reference to the map containing the std::type_index and bitflags for all registered components.
		@return std::unordered_map<std::type_index, short>&
		*/
		std::unordered_map<std::type_index, short>& GetBitFlagMap() { return m_typeToBitflag; }
	};
}