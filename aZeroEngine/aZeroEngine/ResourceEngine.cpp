#include "ResourceEngine.h"

void ResourceEngine::Init(ID3D12Device* _device)
{
	device = _device;
	descriptorManager.Init(_device, 1000, 1000);

	directQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
	copyQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

	renderPassAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	renderPassList.Init(_device, renderPassAllocator);

	copyAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY);
	copyList.Init(_device, copyAllocator);

#ifdef _DEBUG
	directQueue.GetQueue()->SetName(L"ResourceEngine Direct Queue");
	copyQueue.GetQueue()->SetName(L"ResourceEngine Copy Queue");

	copyAllocator.GetAllocator()->SetName(L"ResourceEngine Copy Allocator");

	renderPassAllocator.GetAllocator()->SetName(L"ResourceEngine Render Pass Allocator");
	renderPassList.GetBaseList()->SetName(L"ResourceEngine Render Pass Base List");
	renderPassList.GetGraphicList()->SetName(L"ResourceEngine Render Pass Graphics List");

	copyList.GetBaseList()->SetName(L"ResourceEngine Copy Base List");
	copyList.GetGraphicList()->SetName(L"ResourceEngine Copy Graphics List");
#endif // DEBUG
}

ResourceEngine::~ResourceEngine()
{
	directQueue.StallCPU(lastSignal);

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
	//Footprint: It's a way to reinterpret a region of a buffer as if it was a texture, so that you can describe a 
		//   texture copy operation that will read/write to the buffer

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
	UINT numRows = 0;
	UINT64 rowSize = 0;
	UINT64 totalSize = 0;

	D3D12_RESOURCE_DESC desc[1] = { _readbackTexture->GetDesc() };
	device->GetCopyableFootprints(desc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = _readbackTexture->GetReadbackResource().Get();
	dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dest.PlacedFootprint.Offset = 0;
	dest.PlacedFootprint = footPrint[0];

	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = _readbackTexture->GetGPUOnlyResource().Get();
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	_readbackTexture->Transition(renderPassList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	renderPassList.GetGraphicList()->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);

	_readbackTexture->Transition(renderPassList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	readbackTextures.push_back(_readbackTexture);
}

void ResourceEngine::Execute()
{
	UINT64 copySignal = copyQueue.Execute(copyList);

	directQueue.WaitForOther(copyQueue, copySignal);
	UINT64 renderPassSignal = directQueue.Execute(renderPassList);

	if (frameIndex % 3 == 0)
	{
		directQueue.StallCPU(renderPassSignal);

		copyAllocator.Reset();
		renderPassAllocator.Reset();

		if (trashResources.size() > 0)
		{
			trashResources.clear();
		}
	}
	else
	{
		directQueue.WaitForFence(renderPassSignal);

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

	copyList.ResetGraphic(copyAllocator);
	renderPassList.ResetGraphic(renderPassAllocator);
}
