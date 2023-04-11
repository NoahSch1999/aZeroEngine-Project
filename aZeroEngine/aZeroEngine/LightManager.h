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
	Matrix VPMatrix = Matrix::Identity;
	Vector3 direction = Vector3(0.f, 0.f, 1.f);
	float pad = 0;
	Vector3 color = Vector3(1.f, 1.f, 1.f);
	float intensity = 1.f;
};

struct NumLights
{
	int numPointLights = 0;
	int numSpotLights = 0;
};

// TODO - CLEANUP CLASS AND DOCUMENT IT ETC...
class LightManager
{
private:

public:
	NumLights numLightsData;
	ConstantBuffer numLightsCB;
	StructuredList<PointLight>pLightList;
	ResourceEngine& resourceEngine;

	ConstantBuffer dLightBuffer;
	DirectionalLight dLightData;

	bool pLigthDirty = false;
	bool numLightDirty = false;

	LightManager(ResourceEngine& _resourceEngine)
		:resourceEngine(_resourceEngine)
	{

	}

	LightManager(ResourceEngine& _resourceEngine, int _numPLights = 100)
		:resourceEngine(_resourceEngine)
	{
		Init(_numPLights);
	}

	void ShutDown()
	{
		resourceEngine.RemoveResource(pLightList.dataBuffer);
		resourceEngine.RemoveResource(numLightsCB);
		resourceEngine.RemoveResource(dLightBuffer);
	}

	void Init(int _numPLights = 100)
	{
		pLightList.Init(resourceEngine, _numPLights, true, true);
		resourceEngine.CreateResource(numLightsCB, (void*)&numLightsData, sizeof(NumLights), true, true);
		resourceEngine.CreateResource(dLightBuffer, (void*)&dLightData, sizeof(DirectionalLight), true, true);
#ifdef _DEBUG
		pLightList.dataBuffer.GetGPUOnlyResource()->SetName(L"LightManager Point Light List Main Resource");
		pLightList.dataBuffer.GetUploadResource()->SetName(L"LightManager Point Light List Interm Resource");

		dLightBuffer.GetGPUOnlyResource()->SetName(L"LightManager Directional Light CB Main Resource");
		dLightBuffer.GetUploadResource()->SetName(L"LightManager Directional Light CB Interm Resource");

		numLightsCB.GetGPUOnlyResource()->SetName(L"LightManager Num Lights Main Resource");
		numLightsCB.GetUploadResource()->SetName(L"LightManager Num Lights Interm Resource");
#endif // DEBUG
	}

	void Update()
	{
		if (pLigthDirty)
		{
			resourceEngine.Update(pLightList.dataBuffer, (void*)pLightList.data.GetObjects().data());
			pLigthDirty = false;
		}
		if (numLightDirty)
		{
			resourceEngine.Update(numLightsCB, (void*)&numLightsData);
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

		return nullptr;
	}

	template<typename T>
	void AddLight(int& _id, const T& _light)
	{
		if constexpr (std::is_same_v<T, PointLight>)
		{
			_id = pLightList.AddElement(resourceEngine, _light, resourceEngine.GetFrameIndex());
			numLightsData.numPointLights++;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T>
	void RemoveLight(T& _lightComponent)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if (!pLightList.RemoveElement(resourceEngine, _lightComponent.id, resourceEngine.GetFrameIndex()))
				return;

			numLightsData.numPointLights--;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T, typename D>
	void UpdateLight(const T& _lightComponent, const D& _data)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if constexpr (std::is_same_v<D, PointLight>)
			{
				pLightList.UpdateElement(resourceEngine, _data, _lightComponent.id, resourceEngine.GetFrameIndex());
				pLigthDirty = true;
				return;
			}
			return;
		}
	}

	void CleanseLights(int _frameIndex)
	{
		pLightList.Cleanse();
		numLightsData.numPointLights = 0;
		numLightDirty = true;
	}
};