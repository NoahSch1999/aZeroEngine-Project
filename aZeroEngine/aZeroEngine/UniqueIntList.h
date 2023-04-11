#pragma once
#include <set>

/** @brief A list of unique integer values which can be lent and returned.
*/
class UniqueIntList
{
private:
	std::set<int> keys;
	int maxKeys = 0;
	int incStep = 100;
public:

	/* Initializes the UniqueIntList object.
	@param _startNumKeys The number of unique integers for the UniqueIntList to start with. Goes from the range 0...._startNumKeys.
	@param _incStep The number of unique integers to increase with whenever all the current keys are already lent.
	*/
	UniqueIntList(int _startNumKeys = 100, int _incStep = 100)
		:maxKeys(_startNumKeys), incStep(_incStep)
	{
		for (int i = 0; i < maxKeys; i++)
		{
			keys.emplace(i);
		}
	}

	/* Initializes the UniqueIntList object.
	@param _startNumKeys The number of unique integers for the UniqueIntList to start with. Goes from the range 0...._startNumKeys.
	@param _incStep The number of unique integers to increase with whenever all the current keys are already lent.
	@return void
	*/
	void Init(int _startNumKeys = 100, int _incStep = 100)
	{
		maxKeys = _startNumKeys;
		incStep = _incStep;

		for (int i = 0; i < maxKeys; i++)
		{
			keys.emplace(i);
		}
	}

	/* Gets a unique integer value that preferably should be given back using UniqueIntList::ReturnKey() later.
	@return int The unique key.
	*/
	int LendKey()
	{
		if (keys.size() == 0)
		{
			for (int i = maxKeys; i < maxKeys + incStep; i++)
			{
				keys.emplace(i);
			}
		}

		return keys.extract(keys.begin()).value();
	}

	/* Returns the key to the UniqueIntList object to enable it to be reused using for example UniqueIntList::LendKey().
	@param _key The key to return. This is set to -1 to specify that it has been returned.
	@return void
	*/
	void ReturnKey(int& _key)
	{
		if (keys.count(_key) == 0)
		{
			keys.emplace(_key);
		}
		_key = -1;
	}
};