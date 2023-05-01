#pragma once
#include "ECSBase.h"
#include "StructuredList.h"
#include "UploadBuffer.h"

struct PointLight
{
	Vector3 color = Vector3(1.f, 1.f, 1.f);
	float intensity = 1.f;
	Vector3 position = Vector3(0.f, 0.f, 0.f);
	float range = 10.f;
	PointLight() = default;
	PointLight(const PointLight& _other)
	{
		color = _other.color;
		intensity = _other.intensity;
		position = _other.position;
		range = _other.range;
	}

	PointLight operator=(const PointLight& _other)
	{
		color = _other.color;
		intensity = _other.intensity;
		position = _other.position;
		range = _other.range;
		return *this;
	}

	PointLight(PointLight&& _other) noexcept
	{
		color = _other.color;
		intensity = _other.intensity;
		position = _other.position;
		range = _other.range;
	}

	PointLight operator=(PointLight&& _other) noexcept
	{
		color = _other.color;
		intensity = _other.intensity;
		position = _other.position;
		range = _other.range;
		return *this;
	}
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
	std::unique_ptr<UploadBuffer<NumLights>> numLightsCB;
	StructuredList<PointLight>pLightList;

	std::unique_ptr<UploadBuffer<DirectionalLight>> dLightBuffer;
	DirectionalLight dLightData;

	bool pLigthDirty = false;
	bool numLightDirty = false;

	LightManager() = default;

	LightManager(ID3D12Device* _device, ResourceTrashcan& _trashcan, int _numPLights = 100)
	{
		Init(_device, _trashcan, _numPLights);
	}

	void Init(ID3D12Device* _device, ResourceTrashcan& _trashcan, int _numPLights = 100)
	{
		pLightList.Init(_device, _trashcan, _numPLights, true, true);

		UploadBufferInitSettings initSettings;
		UploadBufferSettings settings;
		settings.m_numElements = 1;
		settings.m_numSubresources = 3;
		numLightsCB = std::make_unique<UploadBuffer<NumLights>>(_device, initSettings, settings, _trashcan);
		dLightBuffer = std::make_unique<UploadBuffer<DirectionalLight>>(_device, initSettings, settings, _trashcan);
	}

	void Update(ID3D12GraphicsCommandList* _cmdList, UINT _frameIndex)
	{
		if (pLigthDirty)
		{
			pLightList.dataBuffer.Update(_cmdList, _frameIndex, pLightList.data.GetObjects().data());
			pLigthDirty = false;
		}
		if (numLightDirty)
		{
			numLightsCB->Update(_cmdList, _frameIndex, numLightsData, 0);
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
	void AddLight(int& _id, T& _light, UINT frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLight>)
		{
			_id = pLightList.AddElement(_light, frameIndex);
			numLightsData.numPointLights++;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T>
	void RemoveLight(T& _lightComponent, UINT frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if (!pLightList.RemoveElement(_lightComponent.id, frameIndex))
				return;

			numLightsData.numPointLights--;
			pLigthDirty = true;
			numLightDirty = true;
			return;
		}
	}

	template<typename T, typename D>
	void UpdateLight(const T& _lightComponent, const D& _data, UINT frameIndex)
	{
		if constexpr (std::is_same_v<T, PointLightComponent>)
		{
			if constexpr (std::is_same_v<D, PointLight>)
			{
				pLightList.UpdateElement(_data, _lightComponent.id, frameIndex);
				pLigthDirty = true;
				return;
			}
			return;
		}
	}

	void CleanseLights()
	{
		pLightList.Cleanse();
		numLightsData.numPointLights = 0;
		numLightDirty = true;
	}
};