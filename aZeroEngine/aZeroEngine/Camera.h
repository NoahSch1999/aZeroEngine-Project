#pragma once
#include "D3D12Include.h"
#include "VertexDefinitions.h"
#include "ConstantBuffer.h"
#include "ResourceEngine.h"

class Camera
{
private:
	#define FORWARD Vector3(0,0,1)
	#define VECRIGHT Vector3(1,0,0)
	#define UP Vector3(0,1,0)

	// Math
	Matrix view;
	Matrix proj;
	
	Vector3 position = Vector3::Zero;
	Vector3 forward = FORWARD;
	Vector3 right = VECRIGHT;

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
	Vector2 sensitivity;

	// Buffer
	ConstantBuffer buffer;
	std::string name;
	ResourceEngine* resourceEngine = nullptr;

	bool active = true;

public:

	ConstantBuffer& GetBuffer() { return buffer; }
	Vector3& GetPosition() { return position; }
	Vector3& GetForward() { return forward; }
	Matrix& GetView() { return view; }
	Matrix& GetProj() { return proj; }
	bool Active() { return active; }
	void ToggleActive(bool _active)
	{
		active = _active;
	}

	Camera(ResourceEngine& _resourceEngine, 
		float _fov, uint32_t _aspectRatio, float _nearPlane = 0.1f, float _farPlane = 1000.f, Vector2 _sensitivity = Vector2(0.005f, 0.005f), const std::string& _name = "")
		:fov(_fov), nearPlane(_nearPlane), farPlane(_farPlane), sensitivity(_sensitivity), name(_name), resourceEngine(&_resourceEngine)
	{
		// Init Math
		view = Matrix::CreateLookAt(position, forward, UP);
		proj = Matrix::CreatePerspectiveFieldOfView(_fov, _aspectRatio, _nearPlane, _farPlane);

		// Init buffer
		Matrix init = view * proj;
		_resourceEngine.CreateResource(buffer, (void*)&init, sizeof(Matrix), true, true);


#ifdef _DEBUG
		buffer.GetGPUOnlyResource()->SetName(L"Camera Main Resource");
		buffer.GetUploadResource()->SetName(L"Camera Intermediate Resource");
#endif // _DEBUG
	}

	~Camera()
	{
		resourceEngine->RemoveResource(buffer);
	}

	void SetSensitivity(float _sensitivity) { sensitivity.x = _sensitivity; sensitivity.y = _sensitivity; }
	void SetSensitivity(Vector2 _sensitivity) { sensitivity = _sensitivity; }
	Vector2 GetSensitivity() const { return sensitivity; }

	void SetFov(float _fov) { fov = _fov; }
	float GetFov() const { return fov; }

	std::string GetName() const { return name; }

	void Update(float _aspectRatio)
	{
		Matrix vecRotMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);

		forward = Vector3::Transform(FORWARD, vecRotMatrix);
		forward.Normalize();

		right = Vector3::Transform(VECRIGHT, vecRotMatrix);
		right.Normalize();

		Vector3 camTarget = forward + position;

		view = Matrix::CreateLookAt(position, camTarget, UP);
		proj = Matrix::CreatePerspectiveFieldOfView(fov, _aspectRatio, nearPlane, farPlane);

		Matrix mat = view * proj;
		resourceEngine->Update(buffer, (void*)&mat);
	}

	void Update(double _deltaTime, float _aspectRatio)
	{
		Matrix vecRotMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);

		forward = Vector3::Transform(FORWARD, vecRotMatrix);
		forward.Normalize();

		right = Vector3::Transform(VECRIGHT, vecRotMatrix);
		right.Normalize();

		Vector3 camTarget = forward + position;

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
			Vector2 mouseDir = InputManager::GetMouseFrameDirection();
			yaw -= mouseDir.x * sensitivity.x;
			pitch += mouseDir.y * sensitivity.y;
			dirty = true;
		}
			
		if (dirty)
		{
			view = Matrix::CreateLookAt(position, camTarget, UP);
			proj = Matrix::CreatePerspectiveFieldOfView(fov, _aspectRatio, nearPlane, farPlane);

			Matrix mat = view * proj;
			resourceEngine->Update(buffer, (void*)&mat);
		}
	}
};