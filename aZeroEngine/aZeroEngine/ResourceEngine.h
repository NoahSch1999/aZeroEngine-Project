#pragma once
#include "ConstantBuffer.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "StructuredBuffer.h"
#include "CommandQueue.h"

class ResourceEngine
{
private:

public:
	CommandQueue directQueue;
	
	CommandAllocator renderPassAllocator;
	CommandList renderPassList;

	CommandQueue copyQueue;
	CommandList copyList;

	CommandAllocator copyAllocator;

	std::vector<ID3D12Resource*> trashResources;
	std::vector<D3D12_RESOURCE_BARRIER> prePassBarriers;
	std::vector<D3D12_RESOURCE_BARRIER> endPassBarriers;
	std::vector<bool*> dirtyFlags;
	UINT64 lastSignal = 0;

	ResourceEngine() = default;

	ResourceEngine(ID3D12Device* _device)
	{
		Init(_device);
	}

	void Init(ID3D12Device* _device)
	{
		directQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
		copyQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

		renderPassAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		renderPassList.Init(_device, renderPassAllocator);

		copyAllocator.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY);
		copyList.Init(_device, copyAllocator);

#ifdef _DEBUG
		directQueue.GetQueue()->SetName(L"Direct Queue");
		copyQueue.GetQueue()->SetName(L"Copy Queue");

		copyAllocator.GetAllocator()->SetName(L"Copy Allocator");

		renderPassAllocator.GetAllocator()->SetName(L"Render Pass Allocator");
		renderPassList.GetBaseList()->SetName(L"Render Pass Base List");
		renderPassList.GetGraphicList()->SetName(L"Render Pass Graphics List");

		copyList.GetBaseList()->SetName(L"Copy Base List");
		copyList.GetGraphicList()->SetName(L"Copy Graphics List");
#endif // DEBUG
	}

	~ResourceEngine() = default;

	void ShutDown()
	{
		directQueue.StallCPU(lastSignal);

		for (auto& resource : trashResources)
		{
			resource->Release();
		}
	}

	// CB
	void CreateResource(ID3D12Device* _device, ConstantBuffer& _resource, void* _data, int _numBytes, bool _dynamic = true, bool _tripple = true)
	{
		_resource.InitBase(_device, copyList, _data, _numBytes, _dynamic, _tripple);
		endPassBarriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(_resource.GetMainResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	// SB
	void CreateResource(ID3D12Device* _device, StructuredBuffer& _resource, void*& _initData, int _numBytes, int _numElements, int _dynamic, int _tripple)
	{
		_resource.InitBase(_device, copyList, _initData, _numBytes, _numElements, _dynamic, _tripple);
		endPassBarriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(_resource.GetMainResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		_resource.SetMainState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// DSV
	void CreateResource(ID3D12Device* _device, DepthStencil& _resource, DescriptorHandle _dsvHandle, UINT _width, UINT _height)
	{
		_resource.InitBase(_device, _dsvHandle, _width, _height);
	}

	// DSV + SRV
	void CreateResource(ID3D12Device* _device, DepthStencil& _resource, DescriptorHandle _dsvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height)
	{
		_resource.InitBase(_device, _dsvHandle, _srvHandle, _width, _height);
	}

	// RTV
	void CreateResource(ID3D12Device* _device, RenderTarget& _resource, DescriptorHandle _rtvHandle, UINT _width, UINT _height, DXGI_FORMAT _format)
	{
		_resource.InitBase(_device, _rtvHandle, _width, _height, _format);
	}

	// RTV + SRV
	void CreateResource(ID3D12Device* _device, RenderTarget& _resource, DescriptorHandle _rtvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height, DXGI_FORMAT _format, const Vector4& _clearColor = Vector4(1.f, 1.f, 1.f, 1.f))
	{
		_resource.InitBase(_device, _rtvHandle, _srvHandle, _width, _height, _format, _clearColor);
	}

	// Texture2D
	void CreateResource(ID3D12Device* _device, Texture2D& _resource, DescriptorHandle _srvHandle, void* _data, UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, const std::string& _name)
	{
		_resource.InitBase(_device, renderPassList, copyList, _srvHandle, _data, _width, _height, _channels, _format, _name);
	}

	// VB
	void CreateResource(ID3D12Device* _device, VertexBuffer& _resource, void* _initData, int _numBytes, int _numElements, const std::string& _name)
	{
		_resource.InitBase(_device, copyList, _initData, _numBytes, _numElements, _name);
	}

	void RemoveResource(BaseResource& _resource)
	{
		if (_resource.GetMainResource())
		{
			trashResources.push_back(_resource.GetMainResource());
			_resource.GetMainResource() = nullptr;
		}

		if (_resource.GetIntermediateResource())
		{
			trashResources.push_back(_resource.GetIntermediateResource());
			_resource.GetIntermediateResource() = nullptr;
		}
	}

	void RemoveResource(Texture2D& _resource)
	{
		if (_resource.GetMainResource())
		{
			trashResources.push_back(_resource.GetMainResource());
			_resource.GetMainResource() = nullptr;
		}
	}

	void RemoveResource(ID3D12Resource* _resource)
	{
		if (_resource)
		{
			trashResources.push_back(_resource);
			_resource = nullptr;
		}
	}

	// CB
	void Update(ConstantBuffer& _resource, void* _data, int _frameIndex)
	{
		UINT64 offset = _resource.GetSizePerSubresource() * _frameIndex;
		memcpy((char*)_resource.GetMappedBuffer() + offset, (char*)_data, _resource.GetSizePerSubresource());

		/*if (_resource.dirty)
			return;

		_resource.dirty = true;
		dirtyFlags.emplace_back(&_resource.dirty);*/

		copyList.GetGraphicList()->CopyBufferRegion(_resource.GetMainResource(), 0, _resource.GetIntermediateResource(), offset, _resource.GetSizePerSubresource());
	}

	// SB Full Update
	void Update(StructuredBuffer& _resource, void* _data, int _frameIndex)
	{
		UINT64 offset = _resource.GetSizePerSubresource() * _frameIndex;
		memcpy((char*)_resource.GetMappedBuffer() + offset, (char*)_data, _resource.GetSizePerSubresource());

		/*if (_resource.dirty)
			return;

		_resource.dirty = true;
		dirtyFlags.emplace_back(&_resource.dirty);*/

		copyList.GetGraphicList()->CopyBufferRegion(_resource.GetMainResource(), 0, _resource.GetIntermediateResource(), offset, _resource.GetSizePerSubresource());
	}

	void Execute(int _frameIndex)
	{
		UINT64 copySignal = copyQueue.Execute(copyList);

		directQueue.WaitForOther(copyQueue, copySignal);
		UINT64 renderPassSignal = directQueue.Execute(renderPassList);

		if (_frameIndex % 3 == 0)
		{
			directQueue.StallCPU(renderPassSignal);

			copyAllocator.Reset();
			copyList.ResetGraphic(copyAllocator);

			renderPassAllocator.Reset();
			renderPassList.ResetGraphic(renderPassAllocator);

			if (trashResources.size() > 0)
			{
				for (auto& resource : trashResources)
				{
					resource->Release();
				}
				trashResources.clear();
			}
		}
		else
		{
			directQueue.WaitForFence(renderPassSignal);
			copyList.ResetGraphic(copyAllocator);
			renderPassList.ResetGraphic(renderPassAllocator);
		}
	}
};