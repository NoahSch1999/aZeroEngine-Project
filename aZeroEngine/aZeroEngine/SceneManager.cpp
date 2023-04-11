#include "SceneManager.h"

std::shared_ptr<Scene> SceneManager::NewScene(const std::string& _name)
{
	if (scenes.count(_name) > 0)
		return nullptr;

	std::shared_ptr<Scene> newScene = std::make_shared<Scene>(ecs, vbCache, mManager, textureCache, lSystem.get(), device, resourceEngine);
	newScene->SetName(_name);
	scenes.emplace(newScene->GetName(), newScene);

	return newScene;
}

std::shared_ptr<Scene> SceneManager::LoadScene( const std::string& _fileDirectory, const std::string& _filename)
{
	if (scenes.count(_filename) > 0)
		return nullptr;

	std::shared_ptr<Scene> newScene = std::make_shared<Scene>(ecs, vbCache, mManager, textureCache, lSystem.get(), device, resourceEngine);
	newScene->Load(_fileDirectory, _filename);
	scenes.emplace(newScene->GetName(), newScene);
	return newScene;
}

std::shared_ptr<Scene> SceneManager::GetScene(const std::string& _name)
{
	if (scenes.count(_name) == 0)
		return nullptr;

	return scenes.at(_name).lock();
}

bool SceneManager::RenameScene(std::shared_ptr<Scene> _sceneHandle, const std::string& _newName)
{
	if (scenes.count(_sceneHandle->GetName()) == 0)
		return false;

	if (scenes.count(_newName) > 0)
		return false;

	scenes.erase(_sceneHandle->GetName());
	scenes.emplace(_newName, _sceneHandle);
	_sceneHandle->SetName(_newName);

	return true;
}

void SceneManager::RemoveScene(std::shared_ptr<Scene>& _sceneHandle)
{
	if (!_sceneHandle)
		return;

	if (scenes.count(_sceneHandle->GetName()) == 0)
		return;

	scenes.erase(_sceneHandle->GetName());
	_sceneHandle.reset();
	_sceneHandle = nullptr;
}
