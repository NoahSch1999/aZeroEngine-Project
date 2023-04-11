#include "Scene.h"

Scene::~Scene()
{
	if (resourceEngine)
	{
		for (auto& ent : entities.GetObjects())
		{
			lSystem->RemoveLight(ent);

			Transform* tf = ecs->GetComponentManager().GetComponent<Transform>(ent);
			if (tf)
			{
				resourceEngine->RemoveResource(tf->GetBuffer());
			}

			ecs->ObliterateEntity(ent);
		}
	}
}

Scene::Scene(Scene&& _other) noexcept
{
	ecs = _other.ecs;
	vbCache = _other.vbCache;
	textureCache = _other.textureCache;
	mManager = _other.mManager;
	resourceEngine = _other.resourceEngine;
	name = _other.name;
	lSystem = _other.lSystem;
	entities = _other.entities;
	entityIdToName = _other.entityIdToName;
	entityNameToId = _other.entityNameToId;
	_other.resourceEngine = nullptr;
}

void Scene::Save(const std::string& _fileDirectory, const std::string& _fileName)
{
	name = _fileName;

	int size = (int)entities.GetObjects().size();

	std::ofstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios_base::trunc | std::ios::out | std::ios::binary);

	bool yes = true;
	bool no = false;

	file.write((char*)&size, sizeof(int));
	for (auto& entity : entities.GetObjects())
	{
		const std::string name = entityIdToName.at(entity.id);
		Helper::WriteToFile(file, name);

		for (int i = 0; i < MAXCOMPONENTS; i++)
		{
			if (entity.componentMask.test(i))
			{
				file.write((char*)&yes, sizeof(bool));
			}
			else
			{
				file.write((char*)&no, sizeof(bool));
			}
		}

		Transform* tf = ecs->GetComponentManager().GetComponent<Transform>(entity);
		file.write((char*)&tf->GetTranslation(), sizeof(Vector3));
		file.write((char*)&tf->GetRotation(), sizeof(Vector3));
		file.write((char*)&tf->GetScale(), sizeof(Vector3));

		if (entity.componentMask.test(COMPONENTENUM::MESH))
		{
			Mesh* mesh = ecs->GetComponentManager().GetComponent<Mesh>(entity);
			std::string fileName = vbCache->GetResource(mesh->GetID())->GetName();
			Helper::WriteToFile(file, fileName);
			file.write((char*)&mesh->castShadows, sizeof(bool));
			file.write((char*)&mesh->receiveShadows, sizeof(float));
		}

		if (entity.componentMask.test(COMPONENTENUM::MATERIAL))
		{
			MaterialComponent* matComp = ecs->GetComponentManager().GetComponent<MaterialComponent>(entity);

			if (matComp->type == MATERIALTYPE::PHONG)
			{
				file.write((char*)&matComp->type, sizeof(int));

				PhongMaterial* phong = mManager->GetMaterial<PhongMaterial>(matComp->materialID);
				Helper::WriteToFile(file, phong->GetName());

				phong->Save("..\\materials\\", *textureCache);
			}
			else if (matComp->type == MATERIALTYPE::PBR)
			{
				file.write((char*)&matComp->type, sizeof(int));

				PBRMaterial* pbrMat = mManager->GetMaterial<PBRMaterial>(matComp->materialID);
				Helper::WriteToFile(file, pbrMat->GetName());

				pbrMat->Save("..\\materials\\", *textureCache);
			}
		}

		if (entity.componentMask.test(COMPONENTENUM::PLIGHT))
		{
			PointLight* light = lSystem->GetLightManager()->GetLight<PointLight>(ecs->GetComponentManager().GetComponent<PointLightComponent>(entity)->id);
			file.write((char*)light, sizeof(PointLight));
		}

		if (entity.componentMask.test(COMPONENTENUM::DLIGHT))
		{
			DirectionalLight* light = lSystem->GetLightManager()->GetLight<DirectionalLight>(ecs->GetComponentManager().GetComponent<DirectionalLightComponent>(entity)->id);
			file.write((char*)light, sizeof(DirectionalLight));
		}
	}

	file.close();
}

bool Scene::Load(const std::string& _fileDirectory, const std::string& _fileName)
{
	std::ifstream file(_fileDirectory + "/" + _fileName + ".azs", std::ios::in | std::ios::binary);
	if (!file.is_open())
		return false;

	int numEntities = -1;
	file.read((char*)&numEntities, sizeof(int));
	name = _fileName;

	for (int i = 0; i < numEntities; i++)
	{
		std::string entityName;
		Helper::ReadFromFile(file, entityName);

		std::bitset<MAXCOMPONENTS>tempSet;
		for (int i = 0; i < MAXCOMPONENTS; i++)
		{
			bool tempBo = false;
			file.read((char*)&tempBo, sizeof(bool));
			tempSet.set(i, tempBo);
		}

		Entity& tempEnt = CreateEntity(entityName);

		Transform tfTemp;
		file.read((char*)&tfTemp.GetTranslation(), sizeof(Vector3));
		file.read((char*)&tfTemp.GetRotation(), sizeof(Vector3));
		file.read((char*)&tfTemp.GetScale(), sizeof(Vector3));

		resourceEngine->CreateResource(tfTemp.GetBuffer(), (void*)&tfTemp.Compose(), sizeof(Matrix), true, true);
		ecs->RegisterComponent<Transform>(tempEnt);
		ecs->UpdateComponent<Transform>(tempEnt, tfTemp);

		if (tempSet.test(COMPONENTENUM::MESH))
		{
			std::string name;
			Helper::ReadFromFile(file, name);

			Mesh tempMesh;

			if (vbCache->Exists(name))
			{
				tempMesh.SetID(vbCache->GetID(name));
			}
			else
			{
				vbCache->LoadResource(name, "..\\meshes\\");
				tempMesh.SetID(vbCache->GetID(name));
			}

			file.read((char*)&tempMesh.castShadows, sizeof(bool));
			file.read((char*)&tempMesh.receiveShadows, sizeof(float));

			ecs->RegisterComponent<Mesh>(tempEnt);
			ecs->UpdateComponent<Mesh>(tempEnt, tempMesh);
		}

		if (tempSet.test(COMPONENTENUM::MATERIAL))
		{
			MaterialComponent matComp;

			file.read((char*)&matComp.type, sizeof(int));

			std::string matName;
			Helper::ReadFromFile(file, matName);

			if (matComp.type == MATERIALTYPE::PHONG) // can be optimized to remove exists check
			{
				if (mManager->Exists<PhongMaterial>(matName))
				{
					matComp.materialID = mManager->GetReferenceID<PhongMaterial>(matName);
				}
				else
				{
					mManager->LoadMaterial<PhongMaterial>(matName);
					matComp.materialID = mManager->GetReferenceID<PhongMaterial>(matName);
				}
			}
			else if (matComp.type == MATERIALTYPE::PBR)
			{
				if (mManager->Exists<PBRMaterial>(matName))
				{
					matComp.materialID = mManager->GetReferenceID<PBRMaterial>(matName);
				}
				else
				{
					mManager->LoadMaterial<PBRMaterial>( matName);
					matComp.materialID = mManager->GetReferenceID<PBRMaterial>(matName);
				}
			}

			ecs->RegisterComponent<MaterialComponent>(tempEnt);
			ecs->UpdateComponent<MaterialComponent>(tempEnt, matComp);
		}

		if (tempSet.test(COMPONENTENUM::PLIGHT))
		{
			PointLight pLight;
			PointLightComponent pComp;
			file.read((char*)&pLight, sizeof(PointLight));
			lSystem->GetLightManager()->AddLight(pComp.id, pLight);

			ecs->RegisterComponent<PointLightComponent>(tempEnt);
			ecs->UpdateComponent<PointLightComponent>(tempEnt, pComp);
		}

		if (tempSet.test(COMPONENTENUM::DLIGHT))
		{
			DirectionalLight dLight;
			DirectionalLightComponent dComp;
			file.read((char*)&dLight, sizeof(DirectionalLight));
			lSystem->GetLightManager()->AddLight(dComp.id, dLight);

			ecs->RegisterComponent<DirectionalLightComponent>(tempEnt);
			ecs->UpdateComponent<DirectionalLightComponent>(tempEnt, dComp);
		}

		ecs->ForceUpdate(tempEnt); // Change so that the entities arent tried to get bound for each AddComponent and instead is only bound here.
	}

	file.close();
	return true;
}

int Scene::FindRec(int _num)
{
	if (entityNameToId.count("Entity_" + std::to_string(_num)) > 0)
	{
		_num += 1;
		_num = FindRec(_num);
	}

	return _num;
}

std::string Scene::GetEntityName(Entity _entity) const
{
	if (entityIdToName.count(_entity.id) > 0)
		return entityIdToName.at(_entity.id);
	return "";
}

Entity& Scene::CreateEntity()
{
	Entity tempEnt = ecs->GetEntityManager().CreateEntity();
	const std::string name = CheckName("Entity_" + std::to_string(tempEnt.id));
	entities.Insert(tempEnt.id, tempEnt);
	entityNameToId.emplace(name, tempEnt.id);
	entityIdToName.emplace(tempEnt.id, name);
	Entity* entity = entities.GetObjectByID(tempEnt.id);
	std::wstring wName;
	wName.assign(name.begin(), name.end());
	Transform tfTemp;
	resourceEngine->CreateResource(tfTemp.GetBuffer(), (void*)&tfTemp.Compose(), sizeof(Matrix), true, true);
	AddComponentToEntity<Transform>(*entity, tfTemp);

	return *entity;
}

Entity& Scene::CreateEntity(const std::string& _name)
{
	Entity tempEnt = ecs->GetEntityManager().CreateEntity();
	const std::string name = CheckName(_name);
	entities.Insert(tempEnt.id, tempEnt);
	Entity* entity = entities.GetObjectByID(tempEnt.id);
	std::wstring wName;
	wName.assign(name.begin(), name.end());
	entityIdToName.emplace(tempEnt.id, name);
	entityNameToId.emplace(name, tempEnt.id);

	return *entities.GetObjectByID(tempEnt.id);
}

void Scene::DeleteEntity(const std::string& _name)
{
	if (entities.Exists(entityNameToId.count(_name) == 0))
		return;

	Entity& ent = GetEntity(_name);
	Transform* tf = ecs->GetComponentManager().GetComponent<Transform>(ent);
	if (tf)
	{
		resourceEngine->RemoveResource(tf->GetBuffer());
	}
	ecs->ObliterateEntity(ent);
	entities.Remove(entityNameToId.at(_name));
}

void Scene::DeleteEntity(int _ID)
{
	Entity& ent = GetEntity(_ID);
	Transform* tf = ecs->GetComponentManager().GetComponent<Transform>(ent);
	if (tf)
	{
		resourceEngine->RemoveResource(tf->GetBuffer());
	}

	PointLightComponent* pComp = ecs->GetComponentManager().GetComponent<PointLightComponent>(ent);
	if (pComp)
	{
		lSystem->RemoveLight(ent);
	}

	ecs->ObliterateEntity(ent);
	entities.Remove(_ID);
}

void Scene::RenameEntity(const std::string& _oldName, const std::string& _newName)
{
	if (entityNameToId.count(_newName) > 0)
		return;

	entityNameToId.emplace(_newName, entityNameToId.at(_oldName));
	entityNameToId.erase(_oldName);

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

Entity& Scene::GetEntity(const std::string& _name)
{
	return *entities.GetObjectByID(entityNameToId.at(_name));
}

Entity& Scene::GetEntity(int _ID)
{
	return *entities.GetObjectByID(_ID);
}

bool Scene::EntityExists(const std::string& _name) const
{
	return entityNameToId.count(_name) > 0;
}
