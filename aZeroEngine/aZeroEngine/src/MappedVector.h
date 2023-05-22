#pragma once
#include <vector>
#include <unordered_map>

/** @brief Stores objects inside a vector and uses a unique int ID to return a specific object.
* The vector will always be packed and can be iterated linearly.
*/
template<typename T>
class MappedVector
{
private:
	std::unordered_map<int, int> m_idToIndex;
	std::unordered_map<int, int> m_indexToId;
	std::vector<T> m_objects;

public:

	/** Allocates a specified amount of memory for the vector.
	@param startSize Number of elements that the vector should allocate capacity for
	*/
	MappedVector(int startSize = 100)
	{
		m_objects.reserve(startSize);
	}

	/** Stores the input value inside the vector and makes it accessible through the input id.
	@param id Unique id to store the value for
	@param value Initial value for the object
	@return void
	*/
	void Add(int id, T&& value)
	{
		const int index = m_objects.size();
		m_objects.emplace_back(std::move(value));
		m_idToIndex.emplace(id, index);
		m_indexToId[index] = id;
	}

	/** Free's up the input id from the MappedVector and removes the object inside the vector.
	@param id Unique id which should have it's value removed
	@return void
	*/
	void Remove(int id)
	{
		const int index = m_idToIndex.at(id);
		m_idToIndex.erase(id);

		const int indexOfLast = m_objects.size() - 1;

		if (indexOfLast == index)
		{
			m_indexToId.erase(index);
			m_objects.erase(m_objects.begin() + index);
			return;
		}

		m_objects[index] = std::move(m_objects[indexOfLast]);

		const int idOfLast = m_indexToId.at(indexOfLast);
		m_idToIndex.at(idOfLast) = index;

		m_indexToId.at(index) = idOfLast;
		m_indexToId.erase(indexOfLast);
		m_objects.erase(m_objects.begin() + indexOfLast);
	}

	/** Shrinks the vector to fit perfectly around the current number of elements.
	@return void
	*/
	void ShrinkToFit()
	{
		m_objects.shrink_to_fit();
	}

	/** Expands the vectors memory.
	@param numElements Number of elements to expand the memory with
	@return void
	*/
	void Expand(int numElements)
	{
		m_objects.reserve(m_objects.size() + numElements);
	}

	/** Checks whether or not an object is stored for the input id.
	@param id Unique id which should be checked
	@return bool TRUE: An object with the input id exists, FALSE: No object with the input id exists
	*/
	bool Contains(int id) const
	{
		return m_idToIndex.contains(id);
	}

	/** Returns a pointer to the element for the input id.
	* This method returns nullptr if the MappedVector doesn't contain an element for the input id.
	@param id The id to retrieve the element with.
	@return T* const
	*/
	T* const GetByID(int id)
	{
		std::unordered_map<int, int>::const_iterator it = m_idToIndex.find(id);
		if (it != m_idToIndex.end())
			return &m_objects[it->second];

		return nullptr;
	}

	/** Returns a reference to the element for the input id.
	* Doesn't check whether or not the element is within the MappedVector.
	@param id The id to retrieve the element with.
	@return T&
	*/
	T& GetByIDFast(int id)
	{
		return m_objects[m_idToIndex.at(id)];
	}

	/** Returns a reference to the vector.
	* To iterate over it in a safe manner, use structure binding with an iterator or loop using the std::vector::size().
	@return std::vector<T>&
	*/
	std::vector<T>& GetObjects()
	{
		return m_objects;
	}

	size_t numObjects() const { return m_objects.size(); }
};