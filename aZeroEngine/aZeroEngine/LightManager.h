#pragma once
#include "ECSBase.h"
#include "StructuredList.h"

struct PointLight
{
	Vector3 color = Vector3(1.f, 1.f, 1.f);
	float intensity = 1.f;
	Vector3 position = Vector3(0.f, 0.f, 0.f);
	float range = 10.f;
};

struct DirectionalLight
{
	Vector3 direction = Vector3(0.f, -1.f, 0.f);
	float intensity = 1.f;
	Vector3 color = Vector3(1.f, 1.f, 1.f);
	float pad = 0;
};

struct NumLights
{
	int numDirectionalLights = 0;
	int numPointLights = 0;
	int numSpotLights = 0;
};

class LightManager
{
private:

public:
	NumLights numLightsData;
	ConstantBuffer numLightsCB;
	StructuredList<PointLight>pLightList;
	StructuredList<DirectionalLight>dLightList;
	ResourceEngine& resourceEngine;

	bool pLigthDirty = false;
	bool dLigthDirty = false;
	bool numLightDirty = false;

	LightManager(ResourceEngine& _resourceEngine)
		:resourceEngine(_resourceEngine)
	{

	}

	void ShutDown()
	{
		resourceEngine.RemoveResource(pLightList.dataBuffer);
		resourceEngine.RemoveResource(dLightList.dataBuffer);
		resourceEngine.RemoveResource(numLightsCB);
	}

	void Init(ID3D12Device* _device)
	{
		pLightList.Init(_device, resourceEngine, 100, true, true);
		dLightList.Init(_device, resourceEngine, 1, true, true);
		resourceEngine.CreateResource(_device, numLightsCB, (void*)&numLightsData, sizeof(NumLights), true, true);

#ifdef _DEBUG
		numLightsCB.GetMainResource()->SetName(L"Num Lights Main Resource");
		numLightsCB.GetIntermediateResource()->SetName(L"Num Lights Inermediate Resource");
#endif // DEBUG
	}

	void Update(int _frameIndex)
	{
		if (pLigthDirty)
		{
			resourceEngine.Update(pLightList.dataBuffer, (void*)pLightList.data.GetObjects().data(), _frameIndex);
			pLigthDirty = false;
		}
		if (dLigthDirty)
		{
			resourceEngine.Update(dLightList.dataBuffer, (void*)dLightList.data.GetObjects().data(), _frameIndex);
			dLigthDirty = false;
		}
		if (numLightDirty)
		{
			resourceEngine.Update(numLightsCB, (void*)&numLightsData, _frameIndex);
			numLightDirty = false;
		}
	}

	template<typename T>
	T* GetLight(int _id)
	{
		if constexpr (std::is_same_v<T, PointLight>)
		{
			if(pLightList.data.Exists(_id))
				return pLightList.data.GetObjectByID(_id);
		}
		else if constexpr (std::is_same_v<T, DirectionalLight>)
		{
			if (dLightList.data.Exists(_id))
				return dLightList.data.GetObjectByID(_id);
		}

		return nullptr;
	}

	template<typename T>
	void AddLight(ID3D12Device* _device, int& _id, T& _light, int _frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLight>)
		{
			_id = pLightList.AddElement(resourceEngine, _light, _frameIndex);
			numLightsData.numPointLights++;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
		else if constexpr (std::is_same_v<T, DirectionalLight>)
		{
			_id = dLightList.AddElement(resourceEngine, _light, _frameIndex);
			numLightsData.numDirectionalLights++;
			dLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T>
	void RemoveLight(T& _lightComponent, int _frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if (!pLightList.RemoveElement(resourceEngine, _lightComponent.id, _frameIndex))
				return;

			numLightsData.numPointLights--;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
		else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
		{
			if (!dLightList.RemoveElement(resourceEngine, _lightComponent.id, _frameIndex))
				return;

			numLightsData.numDirectionalLights--;
			dLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T, typename D>
	void UpdateLight(const T& _lightComponent, const D& _data, int _frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if constexpr (std::is_same_v<D, PointLight>)
			{
				pLightList.UpdateElement(resourceEngine, _data, _lightComponent.id, _frameIndex);
				pLigthDirty = true;
				return;
			}
			return;
		}
		else if constexpr (std::is_same_v<T, DirectionalLightComponent>)
		{
			if constexpr (std::is_same_v<D, DirectionalLight>)
			{
				dLightList.UpdateElement(resourceEngine, _data, _lightComponent.id, _frameIndex);
				dLigthDirty = true;
				return;
			}
		}
	}

	void CleanseLights(int _frameIndex)
	{
		pLightList.Cleanse();
		dLightList.Cleanse();
		numLightsData.numPointLights = 0;
		numLightsData.numDirectionalLights = 0;
		numLightDirty = true;
	}
};