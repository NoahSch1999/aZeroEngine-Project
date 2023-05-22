#pragma once
#include "VertexDefinitions.h"
#include "reactphysics3d/reactphysics3d.h"

class Transform
{
private:
	DXM::Vector3 translation = DXM::Vector3::Zero;
	DXM::Vector3 rotation = DXM::Vector3::Zero;
	DXM::Vector3 scale = DXM::Vector3(1.f, 1.f, 1.f);

	DXM::Matrix worldMatrix = DXM::Matrix::Identity;
	DXM::Matrix localMatrix = DXM::Matrix::Identity;
public:
	UINT m_frameResourceIndex = 0;

	DXM::Vector3& GetTranslation() { return translation; }
	DXM::Vector3& GetRotation() { return rotation; }
	DXM::Vector3& GetScale() { return scale; }

	DXM::Matrix GetLocalMatrix() const
	{
		return  DXM::Matrix::CreateScale(scale) * DXM::Matrix::CreateFromYawPitchRoll(rotation) * DXM::Matrix::CreateTranslation(translation);
	}

	void SetWorldMatrix(const DXM::Matrix& _matrix) { worldMatrix = _matrix; }
	DXM::Matrix GetWorldMatrix() const { return worldMatrix; }

	Transform() = default;
};

class Mesh
{
private:
	int vbID;
public:
	Mesh() = default;
	Mesh(int _vbID) :vbID(_vbID) {};
	void SetID(int _vbID) { vbID = _vbID; }
	int GetID() { return vbID; }

	bool castShadows = false;
	int receiveShadows = 1;
};

enum MATERIALTYPE { PBR };

struct MaterialComponent
{
public:
	MaterialComponent() = default;
	MaterialComponent(int _materialID) :materialID(_materialID) {};

	int materialID = -1;
	MATERIALTYPE type = MATERIALTYPE::PBR;
};

// Light component
struct PointLightComponent
{
	PointLightComponent() = default;
	int id = -1;
};

struct DirectionalLightComponent
{
	DirectionalLightComponent() = default;
	int id = -1;
};

struct RigidBody
{
	RigidBody() = default;
	reactphysics3d::RigidBody* m_body = nullptr;
	/*reactphysics3d::BodyType m_type = reactphysics3d::BodyType::DYNAMIC;*/
	/*reactphysics3d::Vector3 m_force = reactphysics3d::Vector3(0.f, 0.f, 0.f);
	reactphysics3d::Vector3 m_centerOfMass = reactphysics3d::Vector3(0.f, 0.f, 0.f);
	float m_mass = 0.f;
	bool m_enableGravity = true;
	bool m_enableSimulation = true;*/

};