#pragma once
#include "ECSBase.h"
#include "Texture2DCache.h"
#include "DescriptorManager.h"
#include "MaterialManager.h"
#include "VertexBufferCache.h"
#include "Sampler.h"
#include "PipelineState.h"
#include "Input.h"
#include "LightManager.h"
#include "AppWindow.h"


#define FORWARD Vector3(0,0,1)
#define VECRIGHT Vector3(1,0,0)
#define UP Vector3(0,1,0)

struct Camera
{
	Matrix view;
	Matrix proj;
	Vector3 position = Vector3::Zero;
	Vector3 forward = FORWARD;
	Vector3 right = VECRIGHT;

	float maxFov = 3.14f * 0.7f;
	float minFov = 3.14f * 0.05f;
	float fov = 3.14f * 0.2f;
	float fovChange = 100.f;
	float pitch = 0.f;
	float yaw = 0.f;
	float moveSpeed = 4.f;

	ConstantBuffer* buffer;
	DIMOUSESTATE diMouse = DIMOUSESTATE();
	IDirectInputDevice8* diMouseDevice;
	LPDIRECTINPUT8 directInput;

	Camera(ID3D12Device* _device, ResourceEngine& _resourceEngine, AppWindow& _appWindow)
	{
		view = Matrix::CreateLookAt(position, forward, UP);
		Vector2 clientDimensions = _appWindow.GetClientSize();
		proj = Matrix::CreatePerspectiveFieldOfView(fov, (float)clientDimensions.x / (float)clientDimensions.y, 0.1f, 1000.f);
		view.Transpose();
		proj.Transpose();
		buffer = new ConstantBuffer();
		_resourceEngine.CreateResource(_device, *buffer, (void*)&view, sizeof(Matrix) + sizeof(Matrix), true, true);
		//buffer->InitDynamic(_device, &_cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), true, L"Camera Buffer");
#ifdef DEBUG
		buffer->GetMainResource()->SetName(L"Camera Buffer");
#endif // DEBUG


		HRESULT hr = DirectInput8Create(_appWindow.GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
		if (FAILED(hr))
			throw;

		hr = directInput->CreateDevice(GUID_SysMouse, &diMouseDevice, NULL);
		if (FAILED(hr))
			throw;

		hr = diMouseDevice->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr))
			throw;

		hr = diMouseDevice->SetCooperativeLevel(_appWindow.GetHandle(), DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
		if (FAILED(hr))
			throw;

		buffer->GetMainResource()->SetName(L"Camera Main Resource");
		buffer->GetIntermediateResource()->SetName(L"Camera Intermediate Resource");
	}

	void Update(ResourceEngine& _resourceEngine, float _deltaTime, Input& _input, UINT _width, UINT _height, int _frameIndex)
	{
		Matrix vecRotMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);

		forward = Vector3::Transform(FORWARD, vecRotMatrix);
		forward.Normalize();

		right = Vector3::Transform(VECRIGHT, vecRotMatrix);
		right.Normalize();

		Vector3 camTarget = forward + position;

		if (_input.KeyHeld(DIK_W))
		{
			position += forward * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_S))
		{
			position -= forward * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_D))
		{
			position -= right * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_A))
		{
			position += right * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_SPACE))
		{
			position += UP * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_LSHIFT))
		{
			position -= UP * moveSpeed * _deltaTime;
		}
		if (_input.mouseMoved)
		{
			yaw -= _input.lastState.lX * 0.001f;
			pitch += _input.currentState.lY * 0.001f;
		}
		view = Matrix::CreateLookAt(position, camTarget, UP);
		view.Transpose();


		// Make better looking... Quickfix...
		if (_input.mouseWheelMoved)
		{
			if (fov > maxFov)
			{
				fov = maxFov;
			}

			if (fov < minFov)
			{
				fov = minFov;
			}

			if (_input.lastWheelValue > 0)
			{
				fov -= fovChange * _deltaTime;
			}
			else if (_input.lastWheelValue < 0)
			{
				fov += fovChange * _deltaTime;
			}

			proj = Matrix::CreatePerspectiveFieldOfView(fov, (float)_width / (float)_height, 0.1f, 1000.f);
			proj.Transpose();
		}

		_resourceEngine.Update(*buffer, (void*)&view, _frameIndex);

		//buffer->Update(_cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), _frameIndex);
	}
};

class ShadowPassSystem : public ECSystem
{
private:
	PipelineState pso;
	RootSignature rootSig;
	InputLayout layout;
	Sampler defaultSampler;
	DescriptorManager& dManager;
	RasterState solidRaster;

	ResourceEngine& resourceEngine;
	VertexBufferCache& vbCache;
	LightManager& lManager;
	MaterialManager& mManager;
	ECS& ecs;
	float clearColor[4] = { 0.7f, 0.7f, 0.7f, 1 };
public:
	DepthStencil shadowMapTexture;
	Camera* camera;

	Matrix dLightMatrix = Matrix::Identity;

	ShadowPassSystem(ID3D12Device* _device, ResourceEngine& _resourceEngine, ECS& _ecs,
		MaterialManager& _matManager, DescriptorManager& _dManager, LightManager& _lManager, VertexBufferCache& _vbCache,
		HINSTANCE _instance, AppWindow& _appWindow)
		:solidRaster(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE), ecs(_ecs), resourceEngine(_resourceEngine), mManager(_matManager), lManager(_lManager), dManager(_dManager), vbCache(_vbCache)
	{
		componentMask.set(false);
		componentMask.set(COMPONENTENUM::TRANSFORM, true);
		componentMask.set(COMPONENTENUM::MESH, true);

		RootParameters params;
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world 0
		params.AddRootConstants(1, 16, D3D12_SHADER_VISIBILITY_VERTEX); // Light 1
		rootSig.Init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

		pso.Init(_device, &rootSig, layout, solidRaster, _appWindow.GetSwapChain().numBackBuffers, _appWindow.GetBBFormat(), _appWindow.GetDSVFormat(),
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_ShadowPass.cso", L"",
			L"", L"", L"");

		defaultSampler.Init(_device, _dManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

		_resourceEngine.CreateResource(_device, shadowMapTexture, _dManager.GetDSVDescriptor(), _dManager.GetPassDescriptor(), _appWindow.GetClientSize().x, _appWindow.GetClientSize().y);
		//shadowMapTexture.Init(_device, _dManager.GetDSVDescriptor(), _cmdList, _appWindow.GetClientSize().x, _appWindow.GetClientSize().y, _dManager.GetPassDescriptor());
//#ifdef DEBUG
		shadowMapTexture.GetMainResource()->SetName(L"shadow map");
//#endif // DEBUG
		//shadowMapTexture.TransitionMain(_cmdList.GetGraphicList(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	~ShadowPassSystem()
	{
		resourceEngine.RemoveResource(shadowMapTexture);
	}

	// Inherited via ECSystem
	virtual void Update() override {
	
		resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(0, NULL, true, &shadowMapTexture.GetHandle().GetCPUHandleRef());
		resourceEngine.renderPassList.GetGraphicList()->ClearDepthStencilView(shadowMapTexture.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

		resourceEngine.renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		resourceEngine.renderPassList.GetGraphicList()->SetPipelineState(pso.GetPipelineState());
		resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootSignature(rootSig.GetSignature());

		Matrix lightMatrix = camera->proj * camera->view;
		float near_plane = 1.0f, far_plane = 10.f;
		Matrix lProj = Matrix::CreateOrthographic(10.0f, 10.0f, near_plane, far_plane);
		Matrix lView = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
		lightMatrix = lProj * lView;

		resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(1, 16, (void*)&lightMatrix, 0);

		ComponentManager& cManager = ecs.GetComponentManager();
		for (Entity ent : entityIDMap.GetObjects())
		{
			Transform*x = cManager.GetComponent<Transform>(ent);
			resourceEngine.renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetView());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(0, x->GetBuffer().GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->DrawInstanced(vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
		}

		D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(shadowMapTexture.GetMainResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);
	};

	void End()
	{
		D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(shadowMapTexture.GetMainResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);
	}
};

class BasicRendererSystem : public ECSystem
{
private:
	PipelineState pso;
	RootSignature rootSig;
	DescriptorManager& dManager;

	PipelineState pbrPso;
	RootSignature pbrRootSig;

public:
	RasterState solidRaster;
	Sampler defaultSampler;
	InputLayout layout;

	Camera camera;

	RenderTarget* currentBackBuffer;
	DepthStencil* dsv;

	ResourceEngine& resourceEngine;
	VertexBufferCache& vbCache;
	LightManager& lManager;
	MaterialManager& mManager;
	ECS& ecs;

	DepthStencil* shadowMap;

	bool pbr = true;

	BasicRendererSystem(ID3D12Device* _device, ResourceEngine& _resourceEngine, ECS& _ecs,
		MaterialManager& _matManager, DescriptorManager& _dManager, LightManager& _lManager, VertexBufferCache& _vbCache,
		HINSTANCE _instance, AppWindow& _appWindow)
		:solidRaster(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE), ecs(_ecs), resourceEngine(_resourceEngine), mManager(_matManager), lManager(_lManager), dManager(_dManager), vbCache(_vbCache),
		camera(_device, _resourceEngine, _appWindow)
	{

		componentMask.set(false);
		componentMask.set(COMPONENTENUM::TRANSFORM, true);
		componentMask.set(COMPONENTENUM::MESH, true);
		componentMask.set(COMPONENTENUM::MATERIAL, true);

		// Root Signature
		RootParameters params;
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 100, D3D12_SHADER_VISIBILITY_ALL, 0, 1);								// textures 0
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// perdrawconstants 1
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);			// world matrix 2
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);											// camera 3
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// sampler 4
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// point light structs 5
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// num lights 6

		params.AddRootDescriptor(2, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// dLights 7
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// Shadow Map 8
		params.AddRootConstants(2, 16, D3D12_SHADER_VISIBILITY_VERTEX); // Light 9
		params.AddRootConstants(2, 4, D3D12_SHADER_VISIBILITY_PIXEL);											// camera 10
		params.AddRootConstants(3, 1, D3D12_SHADER_VISIBILITY_PIXEL);											// mesh draw constants 11

		rootSig.Init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

		pso.Init(_device, &rootSig, layout, solidRaster, _appWindow.GetSwapChain().numBackBuffers, _appWindow.GetBBFormat(), _appWindow.GetDSVFormat(),
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Basic.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Basic.cso",
			L"", L"", L"");

		defaultSampler.Init(_device, _dManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

		RootParameters paramsTwo;
		paramsTwo.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 100, D3D12_SHADER_VISIBILITY_ALL, 0, 1); // textures 0
		paramsTwo.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world matrix 1
		paramsTwo.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);	// camera 2
		paramsTwo.AddRootConstants(2, 8, D3D12_SHADER_VISIBILITY_PIXEL); // Camera ps 3
		paramsTwo.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL); // sampler 4
		paramsTwo.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0); // numlights 5
		paramsTwo.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// point light structs 6
		paramsTwo.AddRootDescriptor(2, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// directional light structs 7
		paramsTwo.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL); // PBR Mat 8

		pbrRootSig.Init(_device, &paramsTwo, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);
		pbrPso.Init(_device, &pbrRootSig, layout, solidRaster, _appWindow.GetSwapChain().numBackBuffers, _appWindow.GetBBFormat(), _appWindow.GetDSVFormat(),
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_PBR.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_PBR.cso",
			L"", L"", L"");
	}

	~BasicRendererSystem()
	{
		resourceEngine.RemoveResource(*camera.buffer);
		delete camera.buffer;
	}

	// Inherited via ECSystem
	virtual void Update() override
	{
		resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &dsv->GetHandle().GetCPUHandleRef());
		resourceEngine.renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (pbr)
		{
			resourceEngine.renderPassList.GetGraphicList()->SetPipelineState(pbrPso.GetPipelineState());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootSignature(pbrRootSig.GetSignature());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootDescriptorTable(0, dManager.GetTexture2DStartAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(2, camera.buffer->GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(3, 4, (void*)&camera.forward, 0);
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(3, 4, (void*)&camera.position, 4);
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootDescriptorTable(4, defaultSampler.GetHandle().GetGPUHandle());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(5, lManager.numLightsCB.GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(6, lManager.pLightList.dataBuffer.GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(7, lManager.dLightList.dataBuffer.GetGPUAddress());
			ComponentManager& cManager = ecs.GetComponentManager();
			for (Entity ent : entityIDMap.GetObjects())
			{
				resourceEngine.renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetView());
				resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, cManager.GetComponent<Transform>(ent)->GetBuffer().GetGPUAddress());
				resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(8, mManager.GetMaterial<PBRMaterial>(cManager.GetComponent<MaterialComponent>(ent)->materialID)->GetGPUAddress());
				resourceEngine.renderPassList.GetGraphicList()->DrawInstanced(vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
			}
		}
		else
		{
			Matrix lightMatrix = Matrix::Identity;
			float near_plane = 1.0f, far_plane = 10.f;
			Matrix lProj = Matrix::CreateOrthographic(10.0f, 10.0f, near_plane, far_plane);
			Matrix lView = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
			lightMatrix = lProj * lView;

			resourceEngine.renderPassList.GetGraphicList()->SetPipelineState(pso.GetPipelineState());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootSignature(rootSig.GetSignature());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootDescriptorTable(0, dManager.GetTexture2DStartAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(3, camera.buffer->GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(5, lManager.pLightList.dataBuffer.GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(6, lManager.numLightsCB.GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(7, lManager.dLightList.dataBuffer.GetGPUAddress());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootDescriptorTable(4, defaultSampler.GetHandle().GetGPUHandle());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootDescriptorTable(8, shadowMap->GetSrvHandle().GetGPUHandle());
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(9, 16, (void*)&lightMatrix, 0);

			Vector4 camDir = Vector4(camera.forward.x, camera.forward.y, camera.forward.z, 1.f);
			//Vector4 camPos = Vector4(camera.forward.x, camera.forward.y, camera.forward.z, 1.f);
			resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(10, 4, &camDir, 0);

			ComponentManager& cManager = ecs.GetComponentManager();
			for (Entity ent : entityIDMap.GetObjects())
			{
				resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(11, 1, &cManager.GetComponent<Mesh>(ent)->receiveShadows, 0);
				resourceEngine.renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetView());
				resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(2, cManager.GetComponent<Transform>(ent)->GetBuffer().GetGPUAddress());
				resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, mManager.GetMaterial<PhongMaterial>(cManager.GetComponent<MaterialComponent>(ent)->materialID)->GetGPUAddress());
				resourceEngine.renderPassList.GetGraphicList()->DrawInstanced(vbCache.GetResource(cManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
			}
		}
	}
};