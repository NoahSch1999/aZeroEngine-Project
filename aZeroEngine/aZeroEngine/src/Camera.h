#pragma once
#include "UploadBuffer.h"

class Camera
{
private:
	#define FORWARD DXM::Vector3(0,0,1)
	#define VECRIGHT DXM::Vector3(1,0,0)
	#define UP DXM::Vector3(0,1,0)

	// Math
	DXM::Matrix view;
	DXM::Matrix proj;
	
	DXM::Vector3 position = DXM::Vector3::Zero;
	DXM::Vector3 forward = FORWARD;
	DXM::Vector3 right = VECRIGHT;

	// Settings
	float maxFov = 3.14f * 0.7f;
	float minFov = 3.14f * 0.05f;
	float fov = 3.14f * 0.2f;
	float fovChange = 100.f;
	float pitch = 0.f;
	float yaw = 0.f;
	float moveSpeed = 4.f;
	float nearPlane;
	float farPlane;
	DXM::Vector2 sensitivity;

	// Buffer
	std::unique_ptr<UploadBuffer<DXM::Matrix>> buffer;
	std::string name;

	bool active = true;

public:

	UploadBuffer<DXM::Matrix>* GetBuffer() { return buffer.get(); }
	DXM::Vector3& GetPosition() { return position; }
	DXM::Vector3& GetForward() { return forward; }
	DXM::Matrix& GetView() { return view; }
	DXM::Matrix& GetProj() { return proj; }
	bool Active() { return active; }
	void ToggleActive(bool _active)
	{
		active = _active;
	}

	Camera(ID3D12Device* device, ResourceTrashcan& trashcan, float _fov, uint32_t _aspectRatio, float _nearPlane = 0.1f, 
		float _farPlane = 1000.f, DXM::Vector2 _sensitivity = DXM::Vector2(0.005f, 0.005f), const std::string& _name = "")
		:fov(_fov), nearPlane(_nearPlane), farPlane(_farPlane), sensitivity(_sensitivity), name(_name)
	{
		// Init Math
		view = DXM::Matrix::CreateLookAt(position, forward, UP);
		proj = DXM::Matrix::CreatePerspectiveFieldOfView(_fov, _aspectRatio, _nearPlane, _farPlane);

		// Init buffer
		DXM::Matrix init = view * proj;

		UploadBufferInitSettings initSettings;
		UploadBufferSettings settings;
		settings.m_numElements = 1;
		settings.m_numSubresources = 3;
		buffer = std::make_unique<UploadBuffer<DXM::Matrix>>(device, initSettings, settings, trashcan);
	}

	~Camera()
	{

	}

	void SetSensitivity(float _sensitivity) { sensitivity.x = _sensitivity; sensitivity.y = _sensitivity; }
	void SetSensitivity(DXM::Vector2 _sensitivity) { sensitivity = _sensitivity; }
	DXM::Vector2 GetSensitivity() const { return sensitivity; }

	void SetFov(float _fov) { fov = _fov; }
	float GetFov() const { return fov; }

	std::string GetName() const { return name; }

	void Update(ID3D12GraphicsCommandList* _cmdList, UINT _frameIndex, float _aspectRatio)
	{
		DXM::Matrix vecRotMatrix = DXM::Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);

		forward = DXM::Vector3::Transform(FORWARD, vecRotMatrix);
		forward.Normalize();

		right = DXM::Vector3::Transform(VECRIGHT, vecRotMatrix);
		right.Normalize();

		DXM::Vector3 camTarget = forward + position;

		view = DXM::Matrix::CreateLookAt(position, camTarget, UP);
		proj = DXM::Matrix::CreatePerspectiveFieldOfView(fov, _aspectRatio, nearPlane, farPlane);

		DXM::Matrix mat = view * proj;
		buffer->update(_cmdList, _frameIndex, mat, 0);
	}

	void Update(double _deltaTime, float _aspectRatio, ID3D12GraphicsCommandList* _cmdList, UINT _frameIndex)
	{
		DXM::Matrix vecRotMatrix = DXM::Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);

		forward = DXM::Vector3::Transform(FORWARD, vecRotMatrix);
		forward.Normalize();

		right = DXM::Vector3::Transform(VECRIGHT, vecRotMatrix);
		right.Normalize();

		DXM::Vector3 camTarget = forward + position;

		bool dirty = false;

		if (InputManager::KeyHeld('W'))
		{
			position += forward * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::KeyHeld('S'))
		{
			position -= forward * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::KeyHeld('D'))
		{
			position -= right * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::KeyHeld('A'))
		{
			position += right * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::KeyHeld(VK_SPACE))
		{
			position += UP * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::KeyHeld(VK_SHIFT))
		{
			position -= UP * moveSpeed * _deltaTime;
			dirty = true;
		}
		if (InputManager::MouseMoved())
		{
			DXM::Vector2 mouseDir = InputManager::GetMouseFrameDirection();
			yaw -= mouseDir.x * sensitivity.x;
			pitch += mouseDir.y * sensitivity.y;
			dirty = true;
		}
			
		if (dirty)
		{
			view = DXM::Matrix::CreateLookAt(position, camTarget, UP);
			proj = DXM::Matrix::CreatePerspectiveFieldOfView(fov, _aspectRatio, nearPlane, farPlane);

			DXM::Matrix mat = view * proj;
			buffer->update(_cmdList, _frameIndex, mat, 0);
		}
	}
};