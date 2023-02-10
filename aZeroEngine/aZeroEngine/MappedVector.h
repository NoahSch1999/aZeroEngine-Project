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
	std::unordered_map<std::string, int> strToIndex;
	std::vector<T> objects;
	std::deque<int> freeIndices;
public:
	MappedVector() = default;

	/** Returns a reference to the std::unordered_map that contains the hash keys corresponding to the internal std::vector indices.
	@return std::unordered_map<std::string, int>&
	*/
	std::unordered_map<std::string, int>& GetStringToIndexMap() { return strToIndex; }

	/** Adds the input object to the internal vector and maps the index to the input string value.
	* Doesn't check for already existing objects with the same key. It is up to the developer to check this by for example calling MappedVector::Exists().
	* In case of the key already existing, it will replace so that the key refers to the new object instead, which will make the old object inaccessible by the key value.
	@param _name Key value which gets mapped to the objects' index within the internal vector
	@param _object Object that gets copied to the internal vector
	@return void
	*/
	void Add(const std::string& _name, const T& _object);

	/** Frees up the index/space for the object that matches the input key value
	* Doesn't check if an object with the input key actually exists. It is up to the developer to check this by for example calling MappedVector::Exists().
	* Since it will call std::unordered_map::Erase() on a non-existing hash key, it will throw an exception.
	@param _name Key value which is mapped to the objects index which will get open for reuse.
	@return TRUE: The object has been succesfully removed, FALSE: An object doesn't exist with the specified hash value
	*/
	bool Remove(const std::string& _name);

	/** Returns the index of the object within the internal vector which can be used in conjunction with MappedVector::Get(int).
	@param _name Key value which is mapped with the returned index
	@return void
	*/
	int GetID(const std::string& _name) const { return strToIndex.at(_name); }

	/** Returns a reference to the object within the internal vector.
	* The reference may be invalidated if an object is added or removed by for example calling MappedVector::Add() or MappedVector::Remove().
	@param _name Key value which is mapped with the object returned
	@return void
	*/
	T& Get(const std::string& _name) { return objects[strToIndex.at(_name)]; }

	/** Returns a reference to the object within the internal vector.
	* The reference may be invalidated if an object is added or removed by for example calling MappedVector::Add() or MappedVector::Remove().
	@param _ID Internal vector index to the object returned. Index can be retrieved using MappedVector::Get(int)
	@return void
	*/
	T& Get(int _ID) { return objects[_ID]; }

	/** Returns whether or not the key already exists.
	@param _key Key to check.
	@return bool
	*/
	const bool Exists(const std::string& _key) const { return strToIndex.count(_key) > 0; }

	/** Returns a reference to the internal vector containing the actual objects.
	* The reference may be invalidated if an object is added or removed by for example calling MappedVector::Add() or MappedVector::Remove().
	@return std::vector<T>&
	*/
	std::vector<T>& GetObjects() { return objects; }
};

template<typename T>
inline void MappedVector<T>::Add(const std::string& _name, const T& _object)
{
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
inline bool MappedVector<T>::Remove(const std::string& _name)
{
	auto freeIndexPos = strToIndex.find(_name);
	if (freeIndexPos == strToIndex.end())
		return false;

	freeIndices.push_back(freeIndexPos->second);
	strToIndex.erase(freeIndexPos);

	return true;
}