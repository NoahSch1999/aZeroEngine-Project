#pragma once
#include "LightManager.h"

class LightSystem : public ECSystem
{
private:
	std::shared_ptr<LightManager> lightManager = nullptr;
	ID3D12Device* device = nullptr;
	UINT m_frameIndex = 0;

public:

	LightSystem() = default;
	
	LightSystem(ComponentManager& _componentManager)
		:ECSystem(_componentManager)
	{
		
	}

	void Init(ID3D12Device* device, ResourceTrashcan& trashcan)
	{
		lightManager = std::make_shared<LightManager>(device, trashcan, 1000);

		// Signature Setup
		componentMask.set(false);
		componentMask.set(COMPONENTENUM::PLIGHT, true);
	}

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
		// Point shadow allocation management
		//PointLight* p = lightManager->GetLight<PointLight>(_component.id);
		//if (p.shadowMapIndex != _data.shadowMapIndex) // if changed
		//{
		//	// the light didnt have a shadow map and the new should have one -> so create a shadow map
		//	if (_data.shadowMapIndex >= 0 && p.shadowMapIndex < 0) // create new shadow map and asign the index to shadowMapIndex and update the light with that
		//	{
		//		// add to array of lights that should be used in shadow pass of rendersystem
		//	}
		//	// the light had a shadow map but the new shouldnt have one -> delete the shadow map
		//	else if (_data.shadowMapIndex < 0 && p.shadowMapIndex >= 0) // remove shadow map and asign -1 to the shadowMapIndex and update the light with that
		//	{
		//		// remove from array of lights that should be used in shadow pass of rendersystem
		//	}
		//}

		if(_component.id != -1)
			lightManager->UpdateLight(_component, _data, m_frameIndex);
	}

	void RemoveLight(const Entity& _entity)
	{
		if (entityIDMap.Exists(_entity.id))
		{
			PointLightComponent* pLight = componentManager.GetComponent<PointLightComponent>(_entity);
			if(pLight)
				lightManager->RemoveLight(*pLight, m_frameIndex);
		}
	}

	virtual bool Bind(Entity& _entity)
	{
		if (ECSystem::Bind(_entity))
		{
			PointLightComponent* pLight = componentManager.GetComponent<PointLightComponent>(_entity);
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

	virtual bool UnBind(Entity& _entity)
	{
		/*if (entityIDMap.Exists(_entity.id))
		{
			PointLightComponent* pLight = componentManager.GetComponent<PointLightComponent>(_entity);
			lightManager->RemoveLight(*pLight);
		}*/

		return ECSystem::UnBind(_entity);
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