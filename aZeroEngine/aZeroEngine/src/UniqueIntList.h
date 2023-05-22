#pragma once
#include <set>

/** @brief A list of unique integer values which can be lent and returned.
*/
class UniqueIntList
{
private:
	std::set<int> m_keys;
	int m_maxKeys = 0;
	int m_incStep = 100;
	int m_currentlyLent = 0;

public:

	/* Initializes the UniqueIntList object.
	@param startNumKeys The number of unique integers for the UniqueIntList to start with. Goes from the range 0...._startNumKeys.
	@param incStep The number of unique integers to increase with whenever all the current keys are already lent.
	*/
	UniqueIntList(int startNumKeys = 100, int incStep = 100)
		:m_maxKeys(startNumKeys), m_incStep(incStep)
	{
		for (int i = 0; i < m_maxKeys; i++)
		{
			m_keys.emplace(i);
		}
	}

	/* Initializes the UniqueIntList object.
	@param startNumKeys The number of unique integers for the UniqueIntList to start with. Goes from the range 0...._startNumKeys.
	@param incStep The number of unique integers to increase with whenever all the current keys are already lent.
	@return void
	*/
	void init(int startNumKeys = 100, int incStep = 100)
	{
		m_maxKeys = startNumKeys;
		m_incStep = incStep;

		for (int i = 0; i < m_maxKeys; i++)
		{
			m_keys.emplace(i);
		}
	}

	/* Gets a unique integer value that preferably should be given back using UniqueIntList::ReturnKey() later.
	@return int The unique key.
	*/
	int lendKey()
	{
		if (m_keys.size() == 0)
		{
			for (int i = m_maxKeys; i < m_maxKeys + m_incStep; i++)
			{
				m_keys.emplace(i);
			}
		}

		m_currentlyLent++;

		return m_keys.extract(m_keys.begin()).value();
	}

	/* Returns the key to the UniqueIntList object to enable it to be reused using for example UniqueIntList::LendKey().
	@param key The key to return. This is set to -1 to specify that it has been returned.
	@return void
	*/
	void returnKey(int& key)
	{
		if (m_keys.count(key) == 0)
		{
			m_keys.emplace(key);
			m_currentlyLent--;
		}

		key = -1;
	}

	int getNumCurrentlyLent() const { return m_currentlyLent; }
};