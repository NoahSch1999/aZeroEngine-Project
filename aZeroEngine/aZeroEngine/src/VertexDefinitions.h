#pragma once
#include "SimpleMath.h"
namespace DXM = DirectX::SimpleMath;

struct BasicVertex
{
	DXM::Vector3 position;
	DXM::Vector2 uv;
	DXM::Vector3 normal;
	DXM::Vector3 tangent;
};