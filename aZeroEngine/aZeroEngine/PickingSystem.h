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
		params.AddRootConstants(0, 1, D3D12_SHADER_VISIBILITY_VERTEX); // world 0
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // camera 1
		params.AddRootConstants(0, 1, D3D12_SHADER_VISIBILITY_PIXEL); // picking color 2
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX); // 3 transform buffer

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
		Vector2 scDim = swapChain->GetBackBufferDimensions();
		resourceEngine->CreateResource(*rtv, scDim.x, scDim.y, 4, DXGI_FORMAT_R32_SINT, true, true, Vector4(-1, -1, -1, -1));

		dsv = std::make_shared<DepthStencil>();
		resourceEngine->CreateResource(*dsv, scDim.x, scDim.y, false);

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
			std::shared_ptr<GraphicsCommandContext> context = resourceEngine->commandManager->GetGraphicsContext();
			//rtv->Clear(resourceEngine->renderPassList);
			//dsv->Clear(resourceEngine->renderPassList);
			ID3D12DescriptorHeap* heap[] = { resourceEngine->GetResourceHeap(), resourceEngine->GetSamplerHeap() };
			context->SetDescriptorHeaps(2, heap);
			context->ClearRenderTargetView(*rtv);
			context->ClearDepthStencilView(*dsv);

			context->SetOMRenderTargets(1, &rtv->GetHandle().GetCPUHandleRef(), true, &dsv->GetHandle().GetCPUHandleRef());
			context->SetIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->SetRSScizzorRects(1, &swapChain->GetScissorRect());
			context->SetRSViewports(1, &swapChain->GetViewPort());
			context->SetPipelineState(pso.GetPipelineState());
			context->SetRootSignature(rootSignature.GetSignature());

			std::shared_ptr<Camera> cam = camera.lock();
			context->SetConstantBufferView(1, cam->GetBuffer().GetGPUAddress());
			context->SetShaderResourceView(3, tfAddress);

			/*resourceEngine->renderPassList.GetGraphicList()->OMSetRenderTargets(1, &rtv->GetHandle().GetCPUHandleRef(), true, &dsv->GetHandle().GetCPUHandleRef());
			resourceEngine->renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			resourceEngine->renderPassList.GetGraphicList()->RSSetScissorRects(1, &swapChain->GetScissorRect());
			resourceEngine->renderPassList.GetGraphicList()->RSSetViewports(1, &swapChain->GetViewPort());

			resourceEngine->renderPassList.GetGraphicList()->SetPipelineState(pso.GetPipelineState());
			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootSignature(rootSignature.GetSignature());

			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, cam->GetBuffer().GetGPUAddress());
			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(3, tfAddress);*/

			for (Entity ent : entityIDMap.GetObjects())
			{
				Transform* tf = componentManager.GetComponent<Transform>(ent);
				context->Set32BitRootConstant(0, tf->frameIndex, 0);
				context->Set32BitRootConstant(2, ent.id, 0);
				VertexBuffer* vb = vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID());
				context->SetIAVertexBuffers(0, 1, &vb->GetView());
				context->DrawInstanced(vb->GetNumVertices(), 1, 0, 0);

				/*resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(0, 1, (void*)&tf->frameIndex, 0);
				resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstant(2, ent.id, 0);
				resourceEngine->renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vb->GetView());
				resourceEngine->renderPassList.GetGraphicList()->DrawInstanced(vb->GetNumVertices(), 1, 0, 0);*/
			}
			resourceEngine->commandManager->Execute(context);

			resourceEngine->RequestReadback(rtv);
		}
	}

	D3D12_GPU_VIRTUAL_ADDRESS tfAddress = 0;
};