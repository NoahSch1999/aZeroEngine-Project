#include "Scene.h"

Scene::Scene(Scene&& _other) noexcept
{
	m_componentManager = _other.m_componentManager;
	m_entityManager = _other.m_entityManager;
	modelCache = _other.modelCache;
	mManager = _other.mManager;
	name = _other.name;
	lSystem = _other.lSystem;
	entities = _other.entities;
	entityIdToName = _other.entityIdToName;
	entityNameToId = _other.entityNameToId;
	_other.loaded = false;
}

Scene::~Scene()
{
	ShutDown();
}

void Scene::ShutDown()
{
	if (loaded)
	{
		for (auto& ent : entities.GetObjects())
		{
			lSystem->RemoveLight(ent);

			m_entityManager->RemoveEntity(ent);
		}
	}

	loaded = false;
}

void Scene::Save(const std::string& _fileDirectory, const std::string& _fileName, Texture2DCache* _textureCache)
{
	name = _fileName;

	int size = (int)entities.GetObjects().size();

	std::ofstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios_base::trunc | std::ios::out | std::ios::binary);

	bool yes = true;
	bool no = false;

	file.write((char*)&size, sizeof(int));
	for (auto& entity : entities.GetObjects())
	{
		const std::string name = entityIdToName.at(entity.m_id);
		Helper::WriteToFile(file, name);

		for (int i = 0; i < aZeroECS::MAXCOMPONENTS; i++)
		{
			if (entity.m_componentMask.test(i))
			{
				file.write((char*)&yes, sizeof(bool));
			}
			else
			{
				file.write((char*)&no, sizeof(bool));
			}
		}

		Transform* tf = m_componentManager->GetComponent<Transform>(entity);
		file.write((char*)&tf->GetTranslation(), sizeof(DXM::Vector3));
		file.write((char*)&tf->GetRotation(), sizeof(DXM::Vector3));
		file.write((char*)&tf->GetScale(), sizeof(DXM::Vector3));

		if (m_componentManager->HasComponent<Mesh>(entity))
		{
			Mesh* mesh = m_componentManager->GetComponent<Mesh>(entity);
			std::string fileName = modelCache->GetResource(mesh->GetID())->getMeshName();
			Helper::WriteToFile(file, fileName);
			file.write((char*)&mesh->castShadows, sizeof(bool));
			file.write((char*)&mesh->receiveShadows, sizeof(float));
		}

		if (m_componentManager->HasComponent<MaterialComponent>(entity))
		{
			MaterialComponent* matComp = m_componentManager->GetComponent<MaterialComponent>(entity);

			if (matComp->type == MATERIALTYPE::PBR)
			{
				file.write((char*)&matComp->type, sizeof(int));

				PBRMaterial* pbrMat = mManager->GetMaterial<PBRMaterial>(matComp->materialID);
				if (pbrMat)
				{
					Helper::WriteToFile(file, pbrMat->GetName());
				}
				else
				{
					pbrMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");
					Helper::WriteToFile(file, pbrMat->GetName());
				}

				pbrMat->Save("..\\materials\\", *_textureCache);
			}
		}

		if (m_componentManager->HasComponent<PointLightComponent>(entity))
		{
			PointLight* light = lSystem->GetLightManager()->GetLight<PointLight>(m_componentManager->GetComponent<PointLightComponent>(entity)->id);
			file.write((char*)light, sizeof(PointLight));
		}

		if (m_componentManager->HasComponent<DirectionalLightComponent>(entity))
		{
			DirectionalLight* light = lSystem->GetLightManager()->GetLight<DirectionalLight>(m_componentManager->GetComponent<DirectionalLightComponent>(entity)->id);
			file.write((char*)light, sizeof(DirectionalLight));
		}
	}

	file.close();
}

bool Scene::Load(ID3D12Device* device, GraphicsContextHandle& context, UINT frameIndex, const std::string& _fileDirectory, const std::string& _fileName)
{
	std::ifstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios::in | std::ios::binary);

	if (!file.is_open())
		return false;
	else
	{
		if (loaded)
		{
			ShutDown();
		}

		loaded = true;
	}

	int numEntities = -1;
	file.read((char*)&numEntities, sizeof(int));
	name = _fileName;

	for (int i = 0; i < numEntities; i++)
	{
		std::string entityName;
		Helper::ReadFromFile(file, entityName);

		std::bitset<aZeroECS::MAXCOMPONENTS>tempSet;
		for (int i = 0; i < aZeroECS::MAXCOMPONENTS; i++)
		{
			bool tempBo = false;
			file.read((char*)&tempBo, sizeof(bool));
			tempSet.set(i, tempBo);
		}

		aZeroECS::Entity& tempEnt = CreateEntity(entityName);

		Transform* tf = m_componentManager->GetComponent<Transform>(tempEnt);
		file.read((char*)&tf->GetTranslation(), sizeof(DXM::Vector3));
		file.read((char*)&tf->GetRotation(), sizeof(DXM::Vector3));
		file.read((char*)&tf->GetScale(), sizeof(DXM::Vector3));

		tf->SetWorldMatrix(tf->GetLocalMatrix());

		if (tempSet.test(m_componentManager->GetComponentBit<Mesh>()))
		{
			std::string name;
			Helper::ReadFromFile(file, name);

			Mesh tempMesh;

			if (modelCache->Exists(name))
			{
				tempMesh.SetID(modelCache->GetID(name));
			}
			else
			{
				modelCache->LoadAZModel(device, context, frameIndex, name, "..\\meshes\\");
				tempMesh.SetID(modelCache->GetID(name));
			}

			file.read((char*)&tempMesh.castShadows, sizeof(bool));
			file.read((char*)&tempMesh.receiveShadows, sizeof(float));

			m_componentManager->AddComponent<Mesh>(tempEnt, std::move(tempMesh));
		}

		if (tempSet.test(m_componentManager->GetComponentBit<MaterialComponent>()))
		{
			MaterialComponent matComp;

			file.read((char*)&matComp.type, sizeof(int));

			std::string matName;
			Helper::ReadFromFile(file, matName);

			if (matComp.type == MATERIALTYPE::PBR)
			{
				if (mManager->Exists<PBRMaterial>(matName))
				{
					matComp.materialID = mManager->GetReferenceID<PBRMaterial>(matName);
				}
				else
				{
					mManager->LoadMaterial<PBRMaterial>(device, context, frameIndex, matName);
					matComp.materialID = mManager->GetReferenceID<PBRMaterial>(matName);
				}
			}

			m_componentManager->AddComponent<MaterialComponent>(tempEnt, std::move(matComp));
		}

		if (tempSet.test(m_componentManager->GetComponentBit<PointLightComponent>()))
		{
			PointLight pLight;
			file.read((char*)&pLight, sizeof(PointLight));

			m_componentManager->AddComponent<PointLightComponent>(tempEnt);
			PointLightComponent* pComp = m_componentManager->GetComponent<PointLightComponent>(tempEnt);
			lSystem->GetLightManager()->UpdateLight(*pComp, pLight, frameIndex);
		}

		// TO REMOVE
		if (tempSet.test(m_componentManager->GetComponentBit<DirectionalLightComponent>()))
		{
			DirectionalLight dLight;
			file.read((char*)&dLight, sizeof(DirectionalLight));
		}

		//ecs->ForceUpdate(tempEnt); // Change so that the entities arent tried to get bound for each AddComponent and instead is only bound here.
	}
	aZeroECS::ComponentArray<MaterialComponent>& arr = m_componentManager->GetComponentArray<MaterialComponent>();
	file.close();

	return true;
}

std::optional<std::string> Scene::GetEntityName(aZeroECS::Entity _entity) const
{
	if (entityIdToName.count(_entity.m_id) > 0)
		return entityIdToName.at(_entity.m_id);
	return {};
}

aZeroECS::Entity& Scene::CreateEntity(const std::string& _name)
{
	aZeroECS::Entity tempEnt = m_entityManager->CreateEntity();
	const std::string name = CheckName(_name);

	entities.Add(tempEnt.m_id, std::move(tempEnt));
	entityIdToName.emplace(tempEnt.m_id, name);
	entityNameToId.emplace(name, tempEnt.m_id);

	aZeroECS::Entity* entity = entities.GetByID(tempEnt.m_id);
	Transform tf;
	AddComponentToEntity<Transform>(*entity, std::move(tf));

	return *entities.GetByID(tempEnt.m_id);
}

void Scene::DeleteEntity(int _ID)
{
	if (!entities.Contains(_ID))
		return;

	aZeroECS::Entity& ent = GetEntity(_ID);

	PointLightComponent* pComp = m_componentManager->GetComponent<PointLightComponent>(ent);
	if (pComp)
	{
		lSystem->RemoveLight(ent);
	}

	entityNameToId.erase(entityIdToName.at(ent.m_id));
	entityIdToName.erase(ent.m_id);

	int id = ent.m_id;
	m_entityManager->RemoveEntity(ent);
	entities.Remove(id);
}

void Scene::DeleteEntity(const std::string& _name)
{
	if (entities.Contains(entityNameToId.count(_name) == 0))
		return;

	aZeroECS::Entity& ent = GetEntity(_name);

	PointLightComponent* pComp = m_componentManager->GetComponent<PointLightComponent>(ent);
	if (pComp)
	{
		lSystem->RemoveLight(ent);
	}

	entityNameToId.erase(_name);
	entityIdToName.erase(ent.m_id);

	int id = ent.m_id;
	m_entityManager->RemoveEntity(ent);
	entities.Remove(id);
}

void Scene::RenameEntity(const aZeroECS::Entity& _entity, const std::string& _newName)
{
	if (entityNameToId.count(_newName) > 0)
		return;

	std::optional<std::string> oldName = GetEntityName(_entity);

	if (!oldName.has_value())
		return;

	entityNameToId.emplace(_newName, entityNameToId.at(oldName.value()));
	entityNameToId.erase(oldName.value());

	entityIdToName.at(entityNameToId.at(_newName)) = _newName;
}

std::string Scene::CheckName(const std::string _name) const
{
	if (entityNameToId.count(_name) > 0)
	{
		return CheckName(_name + ".");
	}
	return std::string(_name);
}
