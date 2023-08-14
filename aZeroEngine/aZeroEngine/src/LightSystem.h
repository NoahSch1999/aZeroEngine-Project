#pragma once
#include "LightManager.h"
#include <memory>

class LightSystem : public aZeroECS::System
{
private:
	std::shared_ptr<LightManager> lightManager = nullptr;
	ID3D12Device* device = nullptr;
	UINT m_frameIndex = 0;

public:

	LightSystem() = default;
	
	LightSystem(aZeroECS::ComponentManager& _componentManager, ID3D12Device* device, ResourceRecycler& trashcan)
		:System(_componentManager)
	{
		lightManager = std::make_shared<LightManager>(device, trashcan, 1000);
		m_componentMask.set(m_componentManager.GetComponentBit<PointLightComponent>());
	}

	//void Init()
	//{
	//	

	//	// Signature Setup
	//	
	//}

	void BeginFrame(UINT frameIndex)
	{
		m_frameIndex = frameIndex;
	}

	std::shared_ptr<LightManager>GetLightManager() { return lightManager; }

	PointLight* GetLight(PointLightComponent _component)
	{
		return lightManager->GetLight<PointLight>(_component.id);
	}

	void UpdateLight(PointLightComponent _component, const PointLight& _data)
	{
		if(_component.id != -1)
			lightManager->UpdateLight(_component, _data, m_frameIndex);
	}

	void RemoveLight(const aZeroECS::Entity& _entity)
	{
		if (m_entities.Contains(_entity.m_id))
		{
			PointLightComponent* pLight = m_componentManager.GetComponent<PointLightComponent>(_entity);
			if(pLight)
				lightManager->RemoveLight(*pLight, m_frameIndex);
		}
	}

	virtual bool Bind(aZeroECS::Entity& _entity)
	{
		if (aZeroECS::System::Bind(_entity))
		{
			PointLightComponent* pLight = m_componentManager.GetComponent<PointLightComponent>(_entity);
			if (pLight->id == -1)
			{
				PointLight pLightData;
				int id = -1;
				lightManager->AddLight(id, pLightData, m_frameIndex);
				pLight->id = id;
			}
			return true;
		}

		return false;
	}

	virtual bool UnBind(const aZeroECS::Entity& _entity)
	{
		return aZeroECS::System::UnBind(_entity);
	}

	// Inherited via ECSystem
	virtual void Update() override
	{	
		throw;
	}

	void Update(ID3D12GraphicsCommandList* _cmdList)
	{
		lightManager->Update(_cmdList, m_frameIndex);
	}
};