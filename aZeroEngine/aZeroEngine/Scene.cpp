#include "Scene.h"

Scene::Scene(Scene&& _other) noexcept
{
	ecs = _other.ecs;
	vbCache = _other.vbCache;
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

			ecs->ObliterateEntity(ent);
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

			if (matComp->type == MATERIALTYPE::PBR)
			{
				file.write((char*)&matComp->type, sizeof(int));

				PBRMaterial* pbrMat = mManager->GetMaterial<PBRMaterial>(matComp->materialID);
				Helper::WriteToFile(file, pbrMat->GetName());

				pbrMat->Save("..\\materials\\", *_textureCache);
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

		std::bitset<MAXCOMPONENTS>tempSet;
		for (int i = 0; i < MAXCOMPONENTS; i++)
		{
			bool tempBo = false;
			file.read((char*)&tempBo, sizeof(bool));
			tempSet.set(i, tempBo);
		}

		Entity& tempEnt = CreateEntity(entityName);

		Transform* tf = ecs->GetComponent<Transform>(tempEnt);
		file.read((char*)&tf->GetTranslation(), sizeof(Vector3));
		file.read((char*)&tf->GetRotation(), sizeof(Vector3));
		file.read((char*)&tf->GetScale(), sizeof(Vector3));

		tf->SetWorldMatrix(tf->GetLocalMatrix());

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

			if (matComp.type == MATERIALTYPE::PBR)
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

std::optional<std::string> Scene::GetEntityName(Entity _entity) const
{
	if (entityIdToName.count(_entity.id) > 0)
		return entityIdToName.at(_entity.id);
	return {};
}

Entity& Scene::CreateEntity(const std::string& _name)
{
	Entity tempEnt = ecs->CreateEntity();
	const std::string name = CheckName(_name);

	entities.Add(tempEnt.id, tempEnt);
	entityIdToName.emplace(tempEnt.id, name);
	entityNameToId.emplace(name, tempEnt.id);

	Entity* entity = entities.GetObjectByID(tempEnt.id);
	AddComponentToEntity<Transform>(*entity, Transform());

	return *entities.GetObjectByID(tempEnt.id);
}

void Scene::DeleteEntity(int _ID)
{
	if (!entities.Exists(_ID))
		return;

	Entity& ent = GetEntity(_ID);

	PointLightComponent* pComp = ecs->GetComponentManager().GetComponent<PointLightComponent>(ent);
	if (pComp)
	{
		lSystem->RemoveLight(ent);
	}

	entityNameToId.erase(entityIdToName.at(ent.id));
	entityIdToName.erase(ent.id);

	ecs->ObliterateEntity(ent);
	entities.Remove(_ID);
}

void Scene::DeleteEntity(const std::string& _name)
{
	if (entities.Exists(entityNameToId.count(_name) == 0))
		return;

	Entity& ent = GetEntity(_name);

	PointLightComponent* pComp = ecs->GetComponentManager().GetComponent<PointLightComponent>(ent);
	if (pComp)
	{
		lSystem->RemoveLight(ent);
	}

	entityNameToId.erase(_name);
	entityIdToName.erase(ent.id);

	ecs->ObliterateEntity(ent);
	entities.Remove(ent.id);
}

void Scene::RenameEntity(const Entity& _entity, const std::string& _newName)
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
