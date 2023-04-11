#pragma once
#include "StructuredBuffer.h"

template<typename T>
class StructuredList
{
private:

public:
	int maxElements;
	/*int elementSize;*/
	std::deque<int> freeIDs;
	std::deque<int> usedIDs;

	//StructuredBuffer indexBuffer;
	StructuredBuffer dataBuffer;
	SlottedMap<T>data;

	StructuredList() = default;

	void Init(ResourceEngine& _resourceEngine, int _maxElements, bool _dynamic = false, bool _trippleBuffered = false)
	{
		maxElements = _maxElements;

		for (int i = 0; i < maxElements; i++)
		{
			freeIDs.push_back(i);
		}

		if (_dynamic)
		{
			_resourceEngine.CreateResource(dataBuffer, nullptr, sizeof(T) * maxElements, maxElements, true, true);
			
#ifdef _DEBUG
			dataBuffer.GetGPUOnlyResource()->SetName(L"Light Data Buffer Main Resource");
			dataBuffer.GetUploadResource()->SetName(L"Light Data Buffer Interm Resource");
#endif // DEBUG

		}
	}

	int AddElement(ResourceEngine& _resourceEngine, const T& _data, int _frameIndex)
	{
		if (freeIDs.empty())
			return -1;

		int ID = freeIDs.front();
		freeIDs.pop_front();

		usedIDs.push_back(ID);

		data.Add(ID, _data);

		return ID;
	}

	bool RemoveElement(ResourceEngine& _resourceEngine, int& _ID, int _frameIndex)
	{
		if (data.Exists(_ID))
		{
			freeIDs.push_back(_ID);

			int idToRemove = 0;
			for (auto& id : usedIDs)
			{
				if (id == _ID)
					break;
				idToRemove++;
			}

			usedIDs.erase(usedIDs.begin() + idToRemove);
			data.Remove(_ID);
			_ID = -1;

			return true;
		}
		return false;
	}

	void UpdateElement(ResourceEngine& _resourceEngine, const T& _data, int _ID, int _frameIndex)
	{
		if (data.Exists(_ID))
		{
			*data.GetObjectByID(_ID) = _data;
		}
	}

	void Cleanse()
	{
		for (int i = 0; i < usedIDs.size(); i++)
		{
			freeIDs.push_back(usedIDs.at(i));
		}

		usedIDs.clear();
	}
};
