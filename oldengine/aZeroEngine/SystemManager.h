#pragma once
#include "System.h"

namespace aZeroECS
{
	/** @brief Registers System subclasses for aZeroECS.
	*/
	class SystemManager
	{
		/*friend class ComponentManager;*/

	private:
		std::unordered_map<std::type_index, std::shared_ptr<System>> m_registeredSystems;

	public:
		SystemManager() = default;

		SystemManager(const SystemManager&) = delete;
		SystemManager(SystemManager&&) = delete;
		SystemManager operator=(const SystemManager&) = delete;
		SystemManager operator=(SystemManager&&) = delete;

		/** Registers the input System subclass and returns a std::shared_ptr to the shared memory block.
		@param componentManager The ComponentManager which the System subclass can use to access components
		@param args An arbitrary number of arguments which will be passed to the constructor of System subclass after componentManager
		@return std::shared_ptr<T>
		*/
		template<typename T, typename...Args>
		std::shared_ptr<T> RegisterSystem(ComponentManager& componentManager, Args&&...args)
		{
			std::shared_ptr<T> newSystem = std::make_shared<T>(componentManager, args...);

			m_registeredSystems.emplace(std::type_index(typeid(T)), newSystem);

			return newSystem;
		}

		/** Checks if the Entity should be bound or unbound from a registered System subclass depending on it's component bitmask.
		* Binds the Entity to a registered System subclass if it has the neccessary components.
		* Unbinds the Entity from a registered System subclass if it doesn't have the neccessary components.
		@param entity The Entity which should be checked
		@return void
		*/
		void EntityUpdated(Entity& entity)
		{
			for (auto& system : m_registeredSystems)
			{
				const bool bound = system.second->Bind(entity);
				if (!bound)
				{
					system.second->UnBind(entity);
				}
			}
		}
	};
}