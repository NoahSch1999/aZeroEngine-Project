#pragma once
#include "Scene.h"

class SceneManager
{
private:
	std::unordered_map<std::string, std::weak_ptr<Scene>>scenes;

	ResourceEngine* resourceEngine = nullptr;

	ECS* ecs = nullptr;

	VertexBufferCache* vbCache = nullptr;

	MaterialManager* mManager = nullptr;

	Texture2DCache* textureCache = nullptr;

	DescriptorManager* dManager = nullptr;

	std::shared_ptr<LightSystem> lSystem = nullptr;

	ID3D12Device* device = nullptr;

public:

	SceneManager(ECS& _ecs, ResourceEngine& _resourceEngine, MaterialManager* _mManager, DescriptorManager& _dManager,
		VertexBufferCache& _vbCache, Texture2DCache& _textureCache, std::shared_ptr<LightSystem> _lSystem, UINT& _frameIndex, ID3D12Device* _device)
		:ecs(&_ecs), resourceEngine(&_resourceEngine), dManager(&_dManager), vbCache(&_vbCache), textureCache(&_textureCache),
		lSystem(_lSystem), mManager(_mManager), device(_device)
	{
	}

	std::shared_ptr<Scene> NewScene(const std::string& _name);

	std::shared_ptr<Scene> LoadScene(const std::string& _fileDirectory, const std::string& _filename);

	std::shared_ptr<Scene> GetScene(const std::string& _name);

	bool RenameScene(std::shared_ptr<Scene> _sceneHandle, const std::string& _newName);

	void RemoveScene(std::shared_ptr<Scene>& _sceneHandle);
};