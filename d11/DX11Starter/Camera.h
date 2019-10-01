#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "DXCore.h"
#include  <Windows.h>
#define M_PI 3.141592653589

using namespace DirectX;
inline XMVECTOR mathFV(XMFLOAT3& vec)
{
	return XMLoadFloat3(&vec);
}
inline XMFLOAT3 mathVF(XMVECTOR& vec)
{
	XMFLOAT3 var;
	XMStoreFloat3(&var, vec);
	return var;
}
inline XMFLOAT4X4 mathMF(XMMATRIX& m)
{
	XMFLOAT4X4 var;
	XMStoreFloat4x4(&var, m);
	return var;
}
inline XMMATRIX mathFM(XMFLOAT4X4& val)
{
	return XMLoadFloat4x4(&val);
}

//behave as fps camera
//updates view & projection matrces
//pitch - around x    yaw -  around y
class Camera
{
	XMFLOAT4X4 viewMat, projMat; //view matrix and projection matrix
	//for lookto view matrix
	XMFLOAT3 camPos, dir, up;
	XMFLOAT4 rotQuat;
	float xRot, yRot;
	
	//need the window info for perspective matrix
	int width, height;  //window width, height
	float angle,		//field of view
		  zNear,	//near clip
		  zFar;		//far clip
	

public:
	Camera();
	~Camera();
	//customized constructor
	Camera(int w, int h, float n, float f);

	//getter
	float getAngle() { return angle; }
	float getNear() { return zNear; }
	float getFar() { return zFar; }
	XMFLOAT3& getCamPos() { return camPos; }
	XMFLOAT3 getUp() { return mathVF(mathFV(up) - mathFV(camPos)); }
	XMFLOAT4X4 getView() { return viewMat; }
	XMFLOAT4X4 getProj() { return projMat; }

	//setter
	void MoveUpDown(float s);

	void OnResize(int x, int y);
	void Start();

	//update the cam
	void Move(float x, float y, float z);
	void Turn(float x, float y);
	void Update();
	
};

