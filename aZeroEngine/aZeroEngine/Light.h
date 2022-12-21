#pragma once
#include "CommandList.h"
#include "HelperFunctions.h"
#include "StructuredBuffer.h"
#include <deque>

struct PointLight
{
	Vector4 position;
	Vector3 color;
	float strenght;
};

struct DirectionalLight
{
	Vector4 direction;
	Vector3 color;
	float strenght;
};

struct SpotLight
{
	Vector4 position;
	Vector3 direction;
	float radius;
	Vector3 color;
	float strenght;
};

struct NumLights
{
	int numDirectionalLights = 0;
	int numPointLights = 0;
	int numSpotLights = 0;
};

template<typename T>
class LightList
{
private:
	int numLights = 0;
	int maxLights;
	std::deque<int>freeIndices;
	std::deque<int>usedIndices;
	StructuredBuffer lightsBuffer;
	StructuredBuffer lightsIndicesBuffer;
public:
	LightList(ID3D12Device* _device, CommandList* _cmdList, int _maxLights)
	{
		maxLights = _maxLights;
		lightsBuffer.Init(_device, _cmdList, maxLights, sizeof(T));
		lightsIndicesBuffer.Init(_device, _cmdList, maxLights, sizeof(int));

		for (int i = 0; i < maxLights; i++)
		{
			freeIndices.push_back(i);
		}
	}

	void AddLight(const T& _light, int& _index);

	void RemoveLight(int _index);

	int GetNumLights() { return usedIndices.size(); }

	StructuredBuffer* GetLightsBufferPtr() { return &lightsBuffer; }
	StructuredBuffer* GetLightsIndicesBufferPtr() { return &lightsIndicesBuffer; }
};

template<typename T>
inline void LightList<T>::AddLight(const T& _light, int& _index)
{
	if (freeIndices.empty())
		return;

	_index = freeIndices.front();
	freeIndices.pop_front();

	usedIndices.push_back(_index);

	int tempSize = usedIndices.size();
	std::vector<int>tempIndices(tempSize);
	for (int i = 0; i < tempSize; i++)
	{
		tempIndices[i] = usedIndices.at(i);
	}

	lightsIndicesBuffer.Update(0, tempIndices.data(), sizeof(int) * tempSize);

	lightsBuffer.Update(_index, (void*)&_light, sizeof(T));
}

template<typename T>
inline void LightList<T>::RemoveLight(int _index)
{
	int tempSize = usedIndices.size();
	for (int i = 0; i < usedIndices.size(); i++)
	{
		if (usedIndices.at(i) == _index)
		{
			freeIndices.push_front(_index);
			usedIndices.erase(usedIndices.begin() + i);

			tempSize--;
			std::vector<int>tempIndices(tempSize);
			for (int i = 0; i < tempSize; i++)
			{
				tempIndices[i] = usedIndices.at(i);
			}

			lightsIndicesBuffer.Update(0, tempIndices.data(), sizeof(int) * tempSize);

			return;
		}
	}
}

class LightManager
{
private:
public:
	ConstantBuffer numLightsCB;
	LightList<DirectionalLight>dLightList;
	LightList<PointLight>pLightList;
	LightList<SpotLight>sLightList;

	NumLights numLights;

	LightManager(ID3D12Device* _device, CommandList* _cmdList, int _maxDirectionalLights, int _maxPointLights, int _maxSpotLights)
		:dLightList(_device, _cmdList, _maxDirectionalLights), 
		pLightList(_device, _cmdList, _maxPointLights),
		sLightList(_device, _cmdList, _maxSpotLights)
	{
		numLightsCB.InitAsDynamic(_device, _cmdList, (void*)&numLights, sizeof(NumLights));
		numLightsCB.resource->SetName(L"CB NUM LIGHTS");
	}

	template<typename T>
	void AddLight(const T& _light, int& _index);
	//{
	//	
	//	//if (freeIndices.empty())
	//	//	return;

	//	//_index = freeIndices.front();
	//	//freeIndices.pop_front();

	//	//usedIndices.push_back(_index);
	//	//
	//	//int tempSize = usedIndices.size();
	//	//std::vector<int>tempIndices(tempSize);
	//	//for (int i = 0; i < tempSize; i++)
	//	//{
	//	//	tempIndices[i] = usedIndices.at(i);
	//	//}

	//	//pointLightIndicesSRV.Update(0, tempIndices.data(), sizeof(int) * tempSize);

	//	//pointLightSRV.Update(_index, (void*)&_pointLight, sizeof(PointLight));

	//	//numLights.numPointLights = tempSize;
	//	//cb.Update((void*)&numLights, sizeof(NumLights));
	//}

	template<typename T>
	void RemoveLight(int _index);
	//{
	//	//int tempSize = usedIndices.size();
	//	//for (int i = 0; i < usedIndices.size(); i++)
	//	//{
	//	//	if (usedIndices.at(i) == _index)
	//	//	{
	//	//		freeIndices.push_front(_index);
	//	//		usedIndices.erase(usedIndices.begin() + i);

	//	//		tempSize--;
	//	//		std::vector<int>tempIndices(tempSize);
	//	//		for (int i = 0; i < tempSize; i++)
	//	//		{
	//	//			tempIndices[i] = usedIndices.at(i);
	//	//		}

	//	//		pointLightIndicesSRV.Update(0, tempIndices.data(), sizeof(int) * tempSize);
	//	//		numLights.numPointLights = tempSize;
	//	//		cb.Update((void*)&numLights, sizeof(NumLights));

	//	//		return;
	//	//	}
	//	//}
	//}

	void UpdateLight(const PointLight& _pointLight, int _index)
	{
		//pointLightSRV.Update(_index, (void*)&_pointLight, sizeof(PointLight));
	}
};

template<typename T>
inline void LightManager::AddLight(const T& _light, int& _index)
{
	if constexpr (std::is_same_v<T, DirectionalLight>)
	{
		dLightList.AddLight(_light, _index);
		numLights.numDirectionalLights = dLightList.GetNumLights();
	}
	else if constexpr (std::is_same_v<T, PointLight>)
	{
		pLightList.AddLight(_light, _index);
		numLights.numPointLights = pLightList.GetNumLights();
	}
	else if constexpr (std::is_same_v<T, SpotLight>)
	{
		sLightList.AddLight(_light, _index);
		numLights.numSpotLights = sLightList.GetNumLights();
	}
	numLightsCB.Update((void*)&numLights, sizeof(NumLights));
}

template<typename T>
inline void LightManager::RemoveLight(int _index)
{
	if constexpr (std::is_same_v<T, DirectionalLight>)
	{
		dLightList.RemoveLight(_index);
		numLights.numDirectionalLights = dLightList.GetNumLights();
	}
	else if constexpr (std::is_same_v<T, PointLight>)
	{
		pLightList.RemoveLight(_index);
		numLights.numPointLights = pLightList.GetNumLights();
	}
	else if constexpr (std::is_same_v<T, SpotLight>)
	{
		sLightList.RemoveLight(_index);
		numLights.numSpotLights = sLightList.GetNumLights();
	}
	numLightsCB.Update((void*)&numLights, sizeof(NumLights));
}
