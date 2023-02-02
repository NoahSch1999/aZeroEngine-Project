#pragma once
#include <iostream>
#include "SwapChain.h"
#include "AppWindow.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "DepthStencil.h"
#include "PipelineState.h"

// TESTING
#include "ConstantBuffer.h"
#include "VertexDefinitions.h"
#include "Sampler.h"
#include "Input.h"
#include "VertexBufferCache.h"
#include "Scene.h"
#include "UserInterface.h"
#include "Texture2DCache.h"
#include "ResourceManager.h"
#include "MaterialManager.h"
#include "Light.h"
#include "Scene.h"

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

	Camera(ID3D12Device* _device, CommandList* _cmdList, UINT _width, UINT _height, HINSTANCE _instance, HWND _handle)
	{
		view = Matrix::CreateLookAt(position, forward, UP);
		proj = Matrix::CreatePerspectiveFieldOfView(fov, (float)_width / (float)_height, 0.1f, 1000.f);
		view.Transpose();
		proj.Transpose();
		buffer = new ConstantBuffer();
		buffer->InitAsDynamic(_device, _cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), L"Camera");

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
			position += right * moveSpeed * _deltaTime;
		}
		if (_input.KeyHeld(DIK_A))
		{
			position -= right * moveSpeed * _deltaTime;
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
			yaw += _input.lastState.lX * 0.001f;
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
			else if(_input.lastWheelValue < 0)
			{
				fov += fovChange * _deltaTime;
			}

			proj = Matrix::CreatePerspectiveFieldOfView(fov, (float)_width / (float)_height, 0.1f, 1000.f);
			proj.Transpose();
		}

		buffer->Update(_cmdList, (void*)&view, sizeof(Matrix) + sizeof(Matrix), _frameIndex);
	}
};

class Graphics
{
private:
	float clearColor[4] = { 0.7f, 0.7f, 0.7f, 1 };
public:
	Graphics(AppWindow* _window, HINSTANCE _instance);
	~Graphics();
	void Initialize(AppWindow* _window, HINSTANCE _instance);
	void Begin();
	void Render(AppWindow* _window);
	void Present();

	void WaitForGPU();

	ID3D12Device* device;
	CommandQueue* directCommandQueue;

	HiddenDescriptorHeap* rtvHeap;
	HiddenDescriptorHeap* dsvHeap;

	// temp
	CommandList directCmdList;
	CommandAllocator* allocator;

	// Presenting and culling
	SwapChain* swapChain;
	RenderTarget* currentBackBuffer;
	int nextSyncSignal = 0;
	int frameIndex;
	int frameCount;

	// DEBUG
	PipelineState pso;
	RootSignature bindlessSignature;

	EditorUI* ui;

	RasterState* rasterState;
	InputLayout layout;

	Sampler* sampler;
	Camera* camera;

	VertexBufferCache* vbCache;
	Texture2DCache* textureCache;
	ResourceManager resourceManager;
	MaterialManager materialManager;
	LightManager* lManager;

	Scene* scene = nullptr;
	ECS* ecs;
};