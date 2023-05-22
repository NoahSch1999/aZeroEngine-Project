#pragma once
#include <bitset>

namespace aZeroECS
{
	/** @brief Max number of components supported by aZeroECS
	*/
	const int MAXCOMPONENTS = 32;

	/** @brief Contains a unique ID and a bitmask containing which components the Entity currently has.
	*/
	struct Entity
	{
		Entity(int id)
			:m_id(id) { }

		int m_id = -1;
		std::bitset<MAXCOMPONENTS> m_componentMask;
	};
}