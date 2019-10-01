#pragma once

#include <DirectXMath.h>
using namespace DirectX;

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The position of the vertex
	XMFLOAT2 UV;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	//DirectX::XMFLOAT4 Color;        // The color of the vertex
};
