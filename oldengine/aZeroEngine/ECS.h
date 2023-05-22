#pragma once
#include "aZeroECS.h"
#include "Components.h"
#include "UniqueIntList.h"

// TO DO: Remove this file and replace with NameMappedVector file and other files...

	template <typename T>
	class NamedMappedVector
	{
	private:
		MappedVector<T> map;

		UniqueIntList idList;

		std::unordered_map<std::string, int>strToID;
		std::unordered_map<int, std::string>IDtoStr;
	public:
		NamedMappedVector(int _startMax, int _incPerEmpty = 100)
			:idList(_startMax, _incPerEmpty)
		{
		}

		/** Adds an element to the last position in the array and creates a string key to access it through NamedSlottedMap::GetObjectByStr()
		@param _key String key which will be used to access the object.
		@param _data Data to copy to the object.
		@return int The ID that can be used to access the element through NamedMappedVector::GetObjectByID(). If an object with _key value already exists, the method returns -1.
		*/
		int Add(const std::string& _key, T&& _data)
		{
			if (strToID.count(_key) > 0)
				return -1;

			int id = idList.lendKey();

			map.Add(id, std::move(_data));

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
			if (!map.Contains(_key))
				return;

			map.Remove(_key);
			strToID.erase(IDtoStr.at(_key));
			IDtoStr.erase(_key);
			idList.returnKey(_key);
		}

		bool Exists(const std::string& _key) const
		{
			if (strToID.count(_key) > 0)
				return true;

			return false;
		}

		bool Exists(int _key) const
		{
			return map.Contains(_key);
		}

		T* GetObjectByKey(const std::string& _key)
		{
			if (strToID.count(_key) == 0)
				return nullptr;

			return map.GetByID(strToID.at(_key));
		}

		T* GetObjectByKey(int _key)
		{
			if (!map.Contains(_key))
				return nullptr;

			return map.GetByID(_key);
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

		UniqueIntList& getIntList() { return idList; }
	};
