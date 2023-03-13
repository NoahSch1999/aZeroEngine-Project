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

	void Init(ID3D12Device* _device, ResourceEngine& _resourceEngine, int _maxElements, bool _dynamic = false, bool _trippleBuffered = false)
	{
		maxElements = _maxElements;

		for (int i = 0; i < maxElements; i++)
		{
			freeIDs.push_back(i);
		}

		if (_dynamic)
		{
			void* temp = nullptr;
			//_resourceEngine.CreateResource(_device, indexBuffer, temp, sizeof(int) * maxElements, maxElements, true, true);
			////indexBuffer.InitDynamic(_device, &_cmdList, nullptr, sizeof(int) * maxElements, maxElements, _trippleBuffered, L"");
			//if (temp)
			//{
			//	delete temp;
			//	temp = nullptr;
			//}
			_resourceEngine.CreateResource(_device, dataBuffer, temp, sizeof(T) * maxElements, maxElements, true, true);
			if (temp)
			{
				delete temp;
				temp = nullptr;
			}
			//dataBuffer.InitDynamic(_device, &_cmdList, nullptr, sizeof(T) * maxElements, maxElements, _trippleBuffered, L"");
			
#ifdef _DEBUG
			//indexBuffer.GetMainResource()->SetName(L"Light Index Buffer Main Resource");
			//indexBuffer.GetIntermediateResource()->SetName(L"Light Index Buffer Interm Resource");
			dataBuffer.GetMainResource()->SetName(L"Light Data Buffer Main Resource");
			dataBuffer.GetIntermediateResource()->SetName(L"Light Data Buffer Interm Resource");
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

		/*int numUsed = (int)usedIDs.size();
		std::vector<int>IDsToUpload(numUsed);

		for (int i = 0; i < numUsed; i++)
		{
			IDsToUpload[i] = usedIDs.at(i);
		}*/
		data.Insert(ID, _data);
		//_resourcEngine.Update(indexBuffer, IDsToUpload.data(), _frameIndex);
		//_resourceEngine.Update(dataBuffer, (void*)data.GetObjects().data(), _frameIndex);
		//indexBuffer.Update(_cmdList, 0, IDsToUpload.data(), sizeof(int) * maxElements, _frameIndex);
		//dataBuffer.Update(_cmdList, ID, (void*)&_data, sizeof(T), _frameIndex);

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
			//_resourceEngine.Update(dataBuffer, (void*)data.GetObjects().data(), 0);

			return true;
		}
		return false;

		//for (int i = 0; i < usedIDs.size(); i++)
		//{
		//	if (usedIDs.at(i) == _ID)
		//	{
		//		freeIDs.push_back(_ID);
		//		usedIDs.erase(usedIDs.begin() + i);

		//		data.Remove(_ID);
		//		_ID = -1;
		//		_resourceEngine.Update(dataBuffer, (void*)data.GetObjects().data(), 0);

		//		//int numUsed = usedIDs.size();
		//		//std::vector<int>IDsToUpload(numUsed);
		//		//for (int i = 0; i < numUsed; i++)
		//		//{
		//		//	IDsToUpload[i] = usedIDs.at(i);
		//		//}

		//		//if (IDsToUpload.size() != 0)
		//		//{
		//		//	_resourceEngine.Update(indexBuffer, IDsToUpload.data(), _frameIndex);
		//		//	//indexBuffer.Update(_cmdList, 0, IDsToUpload.data(), sizeof(int) * maxElements, _frameIndex);
		//		//}

		//		return true;
		//	}
		//}

		//return false;
	}

	void UpdateElement(ResourceEngine& _resourceEngine, const T& _data, int _ID, int _frameIndex)
	{
		if (data.Exists(_ID))
		{
			*data.GetObjectByID(_ID) = _data;
		}
		
		//dataBuffer.Update(_cmdList, _ID, (void*)&_data, sizeof(T), _frameIndex);
	}

	void Cleanse()
	{
		for (int i = 0; i < usedIDs.size(); i++)
		{
			freeIDs.push_back(usedIDs.at(i));
		}

		usedIDs.clear();

		data.Clear();
	}
};
