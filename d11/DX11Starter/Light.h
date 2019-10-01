#pragma once
#include <DirectXMath.h>
using namespace DirectX;

//----------------
// A struct for lights
//---------------

struct DirectionalLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;
	float pad;
};
struct PointLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 LightPos;
	float range;
};
struct SpotLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;
	float cone;
	XMFLOAT3 LightPos;
	float range;
};