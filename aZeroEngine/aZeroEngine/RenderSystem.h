#pragma once
#include "ECSBase.h"
#include "Texture2DCache.h"
#include "ResourceManager.h"
#include "MaterialManager.h"
#include "VertexBufferCache.h"
#include "Sampler.h"
#include "PipelineState.h"
#include "Input.h"
#include "Light.h"
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
	DIMOUSESTATE diMouse;
	IDirectInputDevice8* diMouseDevice;
	LPDIRECTINPUT8 directInput;

	Camera(ID3D12Device* _device, CommandList& _cmdList, UINT _width, UINT _height, HINSTANCE _instance, HWND _handle)
	{
		view = Matrix::CreateLookAt(position, forward, UP);
		proj = Matrix::CreatePerspectiveFieldOfView(fov, (float)_width / (float)_height, 0.1f, 1000.f);
		view.Transpose();
		proj.Transpose();
		buffer = new ConstantBuffer();
		buffer->InitDynamic(_device, &_cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), 1, true, L"Camera Buffer");

		HRESULT hr = DirectInput8Create(_instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
		if (FAILED(hr))
			throw;

		hr = directInput->CreateDevice(GUID_SysMouse, &diMouseDevice, NULL);
		if (FAILED(hr))
			throw;

		hr = diMouseDevice->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr))
			throw;

		hr = diMouseDevice->SetCooperativeLevel(_handle, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
		if (FAILED(hr))
			throw;

	}

	void Update(double _deltaTime, Input& _input, UINT _width, UINT _height, CommandList* _cmdList, int _frameIndex)
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

		buffer->Update(_cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), _frameIndex);
	}
};


class BasicRendererSystem : public ECSystem
{
private:
	PipelineState pso;
	RootSignature rootSig;
	InputLayout layout;
	RasterState solidRaster;
	Sampler defaultSampler;
	ResourceManager& rManager;

public:

	Camera camera;

	CommandList& cmdList;
	VertexBufferCache& vbCache;
	LightManager& lManager;
	MaterialManager& mManager;
	ECS& ecs;

	BasicRendererSystem(ID3D12Device* _device, CommandList& _cmdList, ECS& _ecs,
		MaterialManager& _matManager, ResourceManager& _rManager, LightManager& _lManager, VertexBufferCache& _vbCache,
		const SwapChain& _swapChain, AppWindow* _window, HINSTANCE _instance)
		:solidRaster(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE), ecs(_ecs), cmdList(_cmdList), mManager(_matManager), lManager(_lManager), rManager(_rManager), vbCache(_vbCache),
		camera(_device, _cmdList, _window->width, _window->height, _instance, _window->windowHandle)
	{

		componentMask.set(false);
		componentMask.set(0, true);
		componentMask.set(1, true);
		componentMask.set(2, true);

		// Root Signature
		RootParameters params;
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 100, D3D12_SHADER_VISIBILITY_ALL, 0, 1);								// textures 0
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// perdrawconstants 1
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);			// world matrix 2
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);			// camera 3
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// sampler 4
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// point light structs 5
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// point light indices 6
		params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// num lights
		rootSig.Init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

		pso.Init(_device, &rootSig, layout, solidRaster, _swapChain.numBackBuffers, _swapChain.rtvFormat, _swapChain.dsvFormat,
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Basic.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Basic.cso",
			L"", L"", L"");

		defaultSampler.Init(_device, _rManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
	}

	~BasicRendererSystem()
	{

	}

	// Inherited via ECSystem
	virtual void Update() override
	{
		cmdList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList.GetGraphicList()->SetPipelineState(pso.GetPipelineState()); // optimize
		cmdList.GetGraphicList()->SetGraphicsRootSignature(rootSig.GetSignature()); // optimize
		cmdList.GetGraphicList()->SetGraphicsRootDescriptorTable(0, rManager.GetTexture2DStartAddress());
		cmdList.GetGraphicList()->SetGraphicsRootConstantBufferView(3, camera.buffer->GetGPUAddress());
		cmdList.GetGraphicList()->SetGraphicsRootShaderResourceView(5, lManager.pLightList.GetLightsBufferPtr()->GetGPUAddress());
		cmdList.GetGraphicList()->SetGraphicsRootShaderResourceView(6, lManager.pLightList.GetLightsIndicesBufferPtr()->GetGPUAddress());
		cmdList.GetGraphicList()->SetGraphicsRootConstantBufferView(7, lManager.numLightsCB.GetGPUAddress());
		cmdList.GetGraphicList()->SetGraphicsRootDescriptorTable(4, defaultSampler.GetHandle().GetGPUHandle());

		ComponentManager& cManager = ecs.GetComponentManager();
		for (Entity ent : entityIDMap.objects)
		{
			cmdList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache.GetBuffer(cManager.GetComponent<Mesh>(ent)->GetVBIndex())->GetView());
			cmdList.GetGraphicList()->SetGraphicsRootConstantBufferView(2, cManager.GetComponent<Transform>(ent)->GetBuffer().GetGPUAddress());
			cmdList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, mManager.GetMaterial<PhongMaterial>(cManager.GetComponent<MaterialComponent>(ent)->GetMaterialID())->GetGPUAddress());
			cmdList.GetGraphicList()->DrawInstanced(vbCache.GetBuffer(cManager.GetComponent<Mesh>(ent)->GetVBIndex())->GetNumVertices(), 1, 0, 0);
		}
	}
};