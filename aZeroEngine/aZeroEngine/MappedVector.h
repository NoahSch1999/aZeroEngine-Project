#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <deque>

template<typename T>
/** @brief A vector where all indices are mapped with a std::string key value.
* To get an object within the internal vector, a user can call either MappedVector::Get(std::string) or MappedVector::Get(int)
*/
class MappedVector
{
private:
	std::unordered_map<std::string, int>strToIndex;
	std::vector<T>objects;
	std::deque<int>freeIndices;
public:
	MappedVector() = default;

	std::unordered_map<std::string, int>& GetStringToIndexMap() { return strToIndex; }

	/** Adds the input object to the internal vector and maps the index to the input string value.
	@param _name Key value which gets mapped to the objects' index within the internal vector
	@param _object Object that gets copied to the internal vector
	@return void
	*/
	void Add(const std::string& _name, const T& _object);
	/** Frees up the index/space for the object that matches the input key value
	@param _name Key value which is mapped to the objects index which will get open for reuse.
	@return void
	*/
	void Remove(const std::string& _name);

	/** Returns the index of the object within the internal vector which can be used in conjunction with MappedVector::Get(int).
	@param _name Key value which is mapped with the returned index
	@return void
	*/
	int GetID(const std::string& _name);

	/** Returns a reference to the object within the internal vector.
	@param _name Key value which is mapped with the object returned
	@return void
	*/
	T& Get(const std::string& _name);

	/** Returns a reference to the object within the internal vector.
	@param _ID Internal vector index to the object returned. Index can be retrieved using MappedVector::Get(int)
	@return void
	*/
	T& Get(int _ID);
};

template<typename T>
inline void MappedVector<T>::Add(const std::string& _name, const T& _object)
{
	if (strToIndex.count(_name) > 0)
		return;

	int newIndex = -1;
	if (freeIndices.empty())
	{
		newIndex = (int)objects.size();
		objects.emplace_back(_object);
		strToIndex.emplace(_name, newIndex);
	}
	else
	{
		newIndex = freeIndices.front();
		freeIndices.pop_front();
		objects[newIndex] = _object;
		strToIndex.emplace(_name, newIndex);
	}
}

template<typename T>
inline void MappedVector<T>::Remove(const std::string& _name)
{
	if (strToIndex.count(_name) == 0)
		return;

	int freeIndex = strToIndex.at(_name);
	strToIndex.erase(_name);
	freeIndices.push_back(freeIndex);
}

template<typename T>
inline T& MappedVector<T>::Get(const std::string& _name)
{
	return objects[strToIndex.at(_name)];
}

template<typename T>
inline int MappedVector<T>::GetID(const std::string& _name)
{
	return strToIndex.at(_name);
}

template<typename T>
inline T& MappedVector<T>::Get(int _ID)
{
	return objects[_ID];
}
