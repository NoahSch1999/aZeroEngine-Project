#include "ResourceEngine.h"

void ResourceEngine::Init(ID3D12Device* _device)
{
	device = _device;
	descriptorManager.Init(_device, 1000, 1000);
	commandManager = std::make_unique<CommandManager>(device, 15, 15, 15);
}

ResourceEngine::~ResourceEngine()
{
	commandManager->CPUFlush();

	for (auto& resource : trashResources)
	{
		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}
}

void ResourceEngine::RequestReadback(std::shared_ptr<TextureResource> _readbackTexture)
{
	readbackTextures.push_back(_readbackTexture);
}

void ResourceEngine::Execute()
{
	if (frameIndex % 3 == 0)
	{
		commandManager->CPUFlush();

		if (trashResources.size() > 0)
		{
			trashResources.clear();
		}
	}
	else
	{
		commandManager->GPUFlushCopy();
		commandManager->GPUFlushCompute();
		commandManager->GPUFlushGraphics();
	}

	if (readbackTextures.size() > 0)
	{
		D3D12_RANGE emptyRange{ 0,0 };
		for (auto& readbackTexture : readbackTextures)
		{
			D3D12_RANGE readbackBufferRange{ 0, readbackTexture->GetReadbackSize() };

			readbackTexture->GetReadbackResource()->Map(0, &readbackBufferRange, reinterpret_cast<void**>(&readbackTexture->GetReadbackData()));
			readbackTexture->GetReadbackResource()->Unmap(0, &emptyRange);
		}
		readbackTextures.clear();
	}
}
