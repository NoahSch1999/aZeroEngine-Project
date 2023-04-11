#pragma once
#include "ECSBase.h"
#include "ResourceEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "VertexBufferCache.h"
#include "Camera.h"

class PickingSystem : public ECSystem
{
private:
	ResourceEngine* resourceEngine = nullptr;
	VertexBufferCache* vbCache = nullptr;

	SwapChain* swapChain = nullptr;
	std::weak_ptr<Camera> camera;

	PipelineState pso;
	RootSignature rootSignature;
	InputLayout layout;
	RasterState rasterState;

	std::shared_ptr<DepthStencil> dsv = nullptr;
	std::shared_ptr<RenderTarget> rtv = nullptr;

public:
	PickingSystem(ComponentManager& _componentManager)
		:ECSystem(_componentManager)
	{

	}

	int GetID(int _xPos, int _yPos)
	{
		if (!rtv->GetReadbackData())
			return -1;

		// Return early if _xPos and _yPos are bigger than swapchain width and height
		if (_xPos > rtv->GetDimensions().x || _yPos > rtv->GetDimensions().y
			|| _xPos < 0 || _yPos < 0)
			return -1;

		int index = _xPos * rtv->GetTexelSize() + _yPos * rtv->GetRowPitch();

		int data = 0;

		memcpy(&data, (char*)rtv->GetReadbackData() + index, 4);

		// index in 2D texture (rtv) = _xPos * _yPos

		// id = rtvBuffer[index]

		return data;
	}

	void SetCamera(std::weak_ptr<Camera> _camera) { camera = _camera; }

	std::shared_ptr<RenderTarget> GetPickingRTV() { return rtv; }

	void Init(ID3D12Device* _device, ResourceEngine* _resourceEngine,
		VertexBufferCache* _vbCache, SwapChain* _swapChain)
	{
		resourceEngine = _resourceEngine;
		swapChain = _swapChain;
		vbCache = _vbCache;

		componentMask.set(false);
		componentMask.set(COMPONENTENUM::TRANSFORM, true);
		componentMask.set(COMPONENTENUM::MESH, true);

		rasterState = RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, true);

		RootParameters params;
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // world 0
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // camera 1
		params.AddRootConstants(0, 1, D3D12_SHADER_VISIBILITY_PIXEL); // picking color 2

		rootSignature.Init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

		std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_Picking.cso";
		std::wstring psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/PS_Picking.cso";
#ifdef _DEBUG
		vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Picking.cso";
		psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Picking.cso";
#endif // _DEBUG

		pso.Init(_device, &rootSignature, layout, rasterState, 3, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_D24_UNORM_S8_UINT,
			vsPath, psPath, L"", L"", L"", false);

		rtv = std::make_shared<RenderTarget>();
		resourceEngine->CreateResource(*rtv, swapChain->width, swapChain->height, 4, DXGI_FORMAT_R32_SINT, true, true, Vector4(-1, -1, -1, -1));

		dsv = std::make_shared<DepthStencil>();
		resourceEngine->CreateResource(*dsv, swapChain->width, swapChain->height, false);

#ifdef _DEBUG
		pso.GetPipelineState()->SetName(L"Picking System PSO");
		rootSignature.GetSignature()->SetName(L"Picking System Root Signature");
		rtv->GetGPUOnlyResource()->SetName(L"Picking System RTV");
		dsv->GetGPUOnlyResource()->SetName(L"Picking System DSV");
#endif // DEBUG
	}

	// Inherited via ECSystem
	virtual void Update() override
	{
		// Prep pipeline
		if (!camera.expired())
		{
			rtv->Clear(resourceEngine->renderPassList);
			dsv->Clear(resourceEngine->renderPassList);
			resourceEngine->renderPassList.GetGraphicList()->OMSetRenderTargets(1, &rtv->GetHandle().GetCPUHandleRef(), true, &dsv->GetHandle().GetCPUHandleRef());
			resourceEngine->renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			resourceEngine->renderPassList.GetGraphicList()->RSSetScissorRects(1, &swapChain->scissorRect);
			resourceEngine->renderPassList.GetGraphicList()->RSSetViewports(1, &swapChain->viewport);

			resourceEngine->renderPassList.GetGraphicList()->SetPipelineState(pso.GetPipelineState());
			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootSignature(rootSignature.GetSignature());

			std::shared_ptr<Camera> cam = camera.lock();
			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, cam->GetBuffer().GetGPUAddress());

			for (Entity ent : entityIDMap.GetObjects())
			{
				resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(0, componentManager.GetComponent<Transform>(ent)->GetBuffer().GetGPUAddress());
				resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstant(2, ent.id, 0);
				VertexBuffer* vb = vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID());
				resourceEngine->renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vb->GetView());
				resourceEngine->renderPassList.GetGraphicList()->DrawInstanced(vb->GetNumVertices(), 1, 0, 0);
			}

			resourceEngine->RequestReadback(rtv);
		}
	}
};