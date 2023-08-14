#pragma once
#include "UploadBuffer.h"
#include <memory>

template<typename T>
class StructuredList
{
private:

public:
	int maxElements;
	std::deque<int> freeIDs;
	std::deque<int> usedIDs;

	UploadBuffer<T> dataBuffer;

	MappedVector<T>data;

	StructuredList() = default;

	void Init(ID3D12Device* device, ResourceRecycler& trashcan, int _maxElements, bool _dynamic = false, bool _trippleBuffered = false)
	{
		maxElements = _maxElements;

		for (int i = 0; i < maxElements; i++)
		{
			freeIDs.push_back(i);
		}

		UploadBufferInitSettings initSettings;
		UploadBufferSettings settings;
		settings.m_numElements = maxElements;
		settings.m_numSubresources = 3;
		dataBuffer = std::move(UploadBuffer<T>(device, initSettings, settings, trashcan));
	}

	int AddElement(T& _data, int _frameIndex)
	{
		if (freeIDs.empty())
			return -1;

		int ID = freeIDs.front();
		freeIDs.pop_front();

		usedIDs.push_back(ID);

		data.Add(ID, std::move(_data));

		return ID;
	}

	bool RemoveElement(int& _ID, int _frameIndex)
	{
		if (data.Contains(_ID))
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

	void UpdateElement(const T& _data, int _ID, int _frameIndex)
	{
		if (data.Contains(_ID))
		{
			*data.GetByID(_ID) = _data;
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
