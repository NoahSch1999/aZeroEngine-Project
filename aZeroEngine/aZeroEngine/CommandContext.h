#pragma once
#include "CommandList.h"
#include "CommandAllocator.h"
#include "CommandQueue.h"
#include <queue>
#include <set>
#include <list>
#include <iostream>

class CommandContext
{
	friend class CommandManager;

protected:
	CommandList list;
	CommandAllocator allocator;

private:
	void ResetAllocator() 
	{ 
		allocator.Reset(); 
	}

public:
	CommandContext(ID3D12Device* _device, D3D12_COMMAND_LIST_TYPE _commandType)
	{
		allocator.Init(_device, _commandType);
		list.Init(_device, allocator);
	}


	~CommandContext() = default;

	void SetDescriptorHeaps(UINT _numHeaps, ID3D12DescriptorHeap* const* _heaps)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetDescriptorHeaps(_numHeaps, _heaps);
	}

	CommandList& GetList() { return list; }

	void Reset() 
	{ 
		list.ResetGraphic(allocator);
	}

	void Close() 
	{ 
		list.CloseGraphic();
	}
};

class GraphicsCommandContext : public CommandContext
{
private:

public:
	GraphicsCommandContext(ID3D12Device* _device)
		:CommandContext(_device, D3D12_COMMAND_LIST_TYPE_DIRECT)
	{

	}

	void DrawInstanced(UINT _vertexCount, UINT _instanceCount, UINT _startVertexIndex, UINT _startInstanceLocation)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->DrawInstanced(_vertexCount, _instanceCount, _startVertexIndex, _startInstanceLocation);
	}

	void SetRootSignature(ID3D12RootSignature* const _rootSignature)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRootSignature(_rootSignature);
	}

	void SetShaderResourceView(UINT _index, const D3D12_GPU_VIRTUAL_ADDRESS _virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRootShaderResourceView(_index, _virtualAddress);
	}

	void SetConstantBufferView(UINT _index, const D3D12_GPU_VIRTUAL_ADDRESS _virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRootConstantBufferView(_index, _virtualAddress);
	}
	void SetUnorderedAccessView(UINT _index, const D3D12_GPU_VIRTUAL_ADDRESS _virtualAddress)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRootUnorderedAccessView(_index, _virtualAddress);
	}

	void SetDescriptorTable(UINT _index, const D3D12_GPU_DESCRIPTOR_HANDLE _startHandle)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRootDescriptorTable(_index, _startHandle);
	}

	void Set32BitRootConstant(UINT _index, UINT _data, UINT _offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRoot32BitConstant(_index, _data, _offsetIn32BitValues);
	}

	void Set32BitRootConstants(UINT _index, UINT _numValues, const void* _data, UINT _offsetIn32BitValues)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetGraphicsRoot32BitConstants(_index, _numValues, _data, _offsetIn32BitValues);
	}

	void SetRSViewports(UINT _numViewports, const D3D12_VIEWPORT* const _viewports)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->RSSetViewports(_numViewports, _viewports);
	}

	void SetRSScizzorRects(UINT _numRects, const D3D12_RECT* const _scizzorRects)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->RSSetScissorRects(_numRects, _scizzorRects);
	}

	void SetPipelineState(ID3D12PipelineState* const _pipelineState)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->SetPipelineState(_pipelineState);
	}

	void SetIAIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* _view)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->IASetIndexBuffer(_view);
	}

	void SetIAVertexBuffers(UINT _startSlot, UINT _numViews, const D3D12_VERTEX_BUFFER_VIEW* const _views)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->IASetVertexBuffers(_startSlot, _numViews, _views);
	}

	void SetIAPrimiteTopology(const D3D12_PRIMITIVE_TOPOLOGY _topologyType)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->IASetPrimitiveTopology(_topologyType);
	}

	void SetOMRenderTargets(UINT _numRenderTargets, const D3D12_CPU_DESCRIPTOR_HANDLE* const _renderTargetsDesriptors, 
		bool _consectiveRange, const D3D12_CPU_DESCRIPTOR_HANDLE* const _depthStencilDescriptor)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->OMSetRenderTargets(_numRenderTargets, _renderTargetsDesriptors, 
			_consectiveRange, _depthStencilDescriptor);
	}

	void ClearRenderTargetView(RenderTarget& _renderTarget)
	{
		_renderTarget.Clear(list);
	}

	void ClearDepthStencilView(DepthStencil& _depthStencil)
	{
		_depthStencil.Clear(list);
	}

	void CopyTextureToBuffer(ID3D12Device* const _device, const D3D12_RESOURCE_DESC& _srcTextureDesc,
		ID3D12Resource* const _srcTextureResource, ID3D12Resource* const _readbackDest,
		D3D12_RESOURCE_STATES _beforeState, D3D12_RESOURCE_STATES _afterState)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[1];
		UINT numRows = 0;
		UINT64 rowSize = 0;
		UINT64 totalSize = 0;

		_device->GetCopyableFootprints(&_srcTextureDesc, 0, 1, 0, footPrint, &numRows, &rowSize, &totalSize);

		D3D12_TEXTURE_COPY_LOCATION dest; // Readback
		dest.pResource = _readbackDest;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = 0;
		dest.PlacedFootprint = footPrint[0];

		D3D12_TEXTURE_COPY_LOCATION source; // GPUOnly
		source.pResource = _srcTextureResource;
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = 0;

		/*if (_beforeState != D3D12_RESOURCE_STATE_COMMON)
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_srcTextureResource, _beforeState, D3D12_RESOURCE_STATE_COMMON);
			static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->ResourceBarrier(1, &barrier);
		}*/

		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->CopyTextureRegion(&dest, 0, 0, 0, &source, nullptr);

		//barrier = CD3DX12_RESOURCE_BARRIER::Transition(_srcTextureResource, D3D12_RESOURCE_STATE_COPY_SOURCE, _afterState);
		//static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->ResourceBarrier(1, &barrier);
	}
};

class CopyCommandContext : public CommandContext
{
private:

public:
	CopyCommandContext(ID3D12Device* _device)
		:CommandContext(_device, D3D12_COMMAND_LIST_TYPE_COPY)
	{

	}

	void UpdateSubresource(ID3D12Resource* const _dest, ID3D12Resource* const _src, UINT64 _srcOffset, UINT64 _rowPitch, UINT64 _slicePitch, const void* const _data)
	{
		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _data;
		sData.RowPitch = _rowPitch;
		sData.SlicePitch = _slicePitch;
		UpdateSubresources(static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList()), _dest, _src, 0, 0, 1, &sData);
	}

	void CopyBufferRegion(ID3D12Resource* const _destBuffer, UINT64 _destOffset, ID3D12Resource* const _srcBuffer, UINT64 _srcOffset, UINT64 _numBytes)
	{
		static_cast<ID3D12GraphicsCommandList*>(list.GetBaseList())->CopyBufferRegion(_destBuffer, _destOffset, _srcBuffer, _srcOffset, _numBytes);
	}
};



class ComputeCommandContext : public CommandContext
{
private:

public:
	ComputeCommandContext(ID3D12Device* _device)
		:CommandContext(_device, D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{

	}
};

class CommandManager
{
private:
	std::list<std::shared_ptr<GraphicsCommandContext>> freeGraphicsContexts;
	std::list<std::shared_ptr<GraphicsCommandContext>> usedGraphicsContexts;

	std::list<std::shared_ptr<CopyCommandContext>> freeCopyContexts;
	std::list<std::shared_ptr<CopyCommandContext>> usedCopyContexts;

	std::list<std::shared_ptr<ComputeCommandContext>> freeComputeContexts;
	std::list<std::shared_ptr<ComputeCommandContext>> usedComputeContexts;

	CommandQueue directQueue;
	CommandQueue copyQueue;
	CommandQueue computeQueue;

	// Might have to have one per queue
	UINT64 lastFenceGraphics = 0;
	UINT64 lastFenceCopy = 0;
	UINT64 lastFenceCompute = 0;

public:

	CommandQueue& GetGraphicsQueue() { return directQueue; }
	CommandQueue& GetCopyQueue() { return copyQueue; }
	CommandQueue& GetComputeQueue() { return computeQueue; }

	CommandManager(ID3D12Device* _device, UINT _numGraphicsContexts, UINT _numCopyContexts, UINT _numComputeContexts)
	{
		std::wstring name = L"";

		directQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
#ifdef _DEBUG
		name = L"Direct Queue";
		directQueue.GetQueue()->SetName(name.c_str());
#endif // _DEBUG

		for (int i = 0; i < _numGraphicsContexts; i++)
		{
			freeGraphicsContexts.push_back(std::make_shared<GraphicsCommandContext>(_device));

#ifdef _DEBUG
			name = L"Graphics ";
			name += L"Allocator " + std::to_wstring(i);
			freeGraphicsContexts.back()->allocator.GetAllocator()->SetName(name.c_str());

			name = L"Graphics ";
			name += L"List " + std::to_wstring(i);
			freeGraphicsContexts.back()->list.GetBaseList()->SetName(name.c_str());
#endif // _DEBUG
		}

		copyQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
#ifdef _DEBUG
		name = L"Copy Queue";
		copyQueue.GetQueue()->SetName(name.c_str());
#endif // _DEBUG

		for (int i = 0; i < _numCopyContexts; i++)
		{
			freeCopyContexts.push_back(std::make_shared<CopyCommandContext>(_device));

#ifdef _DEBUG
			name = L"Copy ";
			name += L"Allocator " + std::to_wstring(i);
			freeCopyContexts.back()->allocator.GetAllocator()->SetName(name.c_str());

			name = L"Copy ";
			name += L"List " + std::to_wstring(i);
			freeCopyContexts.back()->list.GetBaseList()->SetName(name.c_str());
#endif // _DEBUG
		}

		computeQueue.Init(_device, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);
#ifdef _DEBUG
		name = L"Compute Queue";
		computeQueue.GetQueue()->SetName(name.c_str());
#endif // _DEBUG

		for (int i = 0; i < _numComputeContexts; i++)
		{
			freeComputeContexts.push_back(std::make_shared<ComputeCommandContext>(_device));

#ifdef _DEBUG
			name = L"Compute ";
			name += L"Allocator " + std::to_wstring(i);
			freeComputeContexts.back()->allocator.GetAllocator()->SetName(name.c_str());

			name = L"Compute ";
			name += L"List " + std::to_wstring(i);
			freeComputeContexts.back()->list.GetBaseList()->SetName(name.c_str());
#endif // _DEBUG
		}
	}

	CommandManager() = default;
	~CommandManager() = default;

	void GPUFlushGraphics()
	{
		directQueue.WaitForFence(lastFenceGraphics);
	}

	void GPUFlushCopy()
	{
		copyQueue.WaitForFence(lastFenceCopy);
	}

	void GPUFlushCompute()
	{
		computeQueue.WaitForFence(lastFenceCompute);
	}

	void CPUFlush()
	{
		computeQueue.StallCPU(lastFenceCompute);
		copyQueue.StallCPU(lastFenceCopy);
		directQueue.StallCPU(lastFenceGraphics);

		for (std::shared_ptr<GraphicsCommandContext> i : usedGraphicsContexts) {
			i->ResetAllocator();
			i->Reset();
			i->Close();
			freeGraphicsContexts.push_back(i);
		}
		usedGraphicsContexts.clear();

		for (std::shared_ptr<CopyCommandContext> i : usedCopyContexts) {
			i->ResetAllocator();
			i->Reset();
			i->Close();
			freeCopyContexts.push_back(i);
		}
		usedCopyContexts.clear();

		for (std::shared_ptr<ComputeCommandContext> i : usedComputeContexts) {
			i->ResetAllocator();
			i->Reset();
			i->Close();
			freeComputeContexts.push_back(i);
		}
		usedComputeContexts.clear();
	}

	// Graphics
	void WaitForCopy()
	{
		directQueue.WaitForOther(copyQueue, lastFenceCopy);
	}

	void WaitForCopy(std::shared_ptr<GraphicsCommandContext> _context, UINT64 _waitForSignal)
	{
		directQueue.WaitForOther(copyQueue, _waitForSignal);
	}

	void WaitForCompute(std::shared_ptr<GraphicsCommandContext> _context, UINT64 _waitForSignal)
	{
		directQueue.WaitForOther(computeQueue, _waitForSignal);
	}

	// Copy
	void WaitForGraphics(std::shared_ptr<CopyCommandContext> _context, UINT64 _waitForSignal)
	{
		copyQueue.WaitForOther(directQueue, _waitForSignal);
	}

	void WaitForCompute(std::shared_ptr<CopyCommandContext> _context, UINT64 _waitForSignal)
	{
		copyQueue.WaitForOther(computeQueue, _waitForSignal);
	}

	// Compute
	void WaitForGraphics(std::shared_ptr<ComputeCommandContext> _context, UINT64 _waitForSignal)
	{
		computeQueue.WaitForOther(directQueue, _waitForSignal);
	}

	void WaitForCopy(std::shared_ptr<ComputeCommandContext> _context, UINT64 _waitForSignal)
	{
		computeQueue.WaitForOther(copyQueue, _waitForSignal);
	}

	std::shared_ptr<GraphicsCommandContext> GetGraphicsContext()
	{
		if (freeGraphicsContexts.size() == 0)
		{
			return nullptr;
		}
		else
		{
			std::shared_ptr<GraphicsCommandContext> retContext = freeGraphicsContexts.front();
			retContext->Reset();
			freeGraphicsContexts.pop_front();

			return retContext;
		}
	}

	std::shared_ptr<CopyCommandContext> GetCopyContext()
	{
		if (freeGraphicsContexts.size() == 0)
		{
			return nullptr;
		}
		else
		{
			std::shared_ptr<CopyCommandContext> retContext = freeCopyContexts.front();
			retContext->Reset();
			freeCopyContexts.pop_front();

			return retContext;
		}
	}

	std::shared_ptr<ComputeCommandContext> GetComputeContext()
	{
		if (freeGraphicsContexts.size() == 0)
		{
			return nullptr;
		}
		else
		{
			std::shared_ptr<ComputeCommandContext> retContext = freeComputeContexts.front();
			retContext->Reset();
			freeComputeContexts.pop_front();

			return retContext;
		}
	}

	UINT64 Execute(std::shared_ptr<GraphicsCommandContext>& _context)
	{
		_context->Close();
		UINT64 fenceValue = directQueue.Execute(_context->list);
		usedGraphicsContexts.push_back(_context);

		_context = nullptr;

		lastFenceGraphics = fenceValue;

		return fenceValue;
	}

	UINT64 Execute(std::shared_ptr<CopyCommandContext>& _context)
	{
		_context->Close();
		UINT64 fenceValue = copyQueue.Execute(_context->list);
		usedCopyContexts.push_back(_context);

		_context = nullptr;

		lastFenceCopy = fenceValue;

		return fenceValue;
	}

	UINT64 Execute(std::shared_ptr<ComputeCommandContext>& _context)
	{
		_context->Close();
		UINT64 fenceValue = computeQueue.Execute(_context->list);
		usedComputeContexts.push_back(_context);

		_context = nullptr;

		lastFenceCompute = fenceValue;

		return fenceValue;
	}

};