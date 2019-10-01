#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

Camera::Camera(int w, int h, float n, float f)
{
	width = w;
	height = h;
	zNear = n;
	zFar = f;
	xRot = 0;
	yRot = 0;
	XMStoreFloat4(&rotQuat, XMQuaternionIdentity());

	//set up camera
	camPos = XMFLOAT3(0, 0, -15);
	dir = XMFLOAT3(0, 0, 1);
	up = XMFLOAT3(0, 1, 0);
	angle = 0.25 * M_PI;
}


void Camera::MoveUpDown(float s)
{
	camPos.y += s;
}

void Camera::OnResize(int x, int y)
{
	width = x;
	height = y;
	//set up projection matrix
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		angle,
		(float)width / height,
		zNear,
		zFar
	);
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(P));
}

void Camera::Start()
{
	//set up view matrix
	XMMATRIX V = XMMatrixLookToLH(
		XMLoadFloat3(&camPos),
		XMLoadFloat3(&dir),
		XMLoadFloat3(&up)
	);
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(V));

	//set up projection matrix
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		angle,
		(float)width / height,
		zNear,
		zFar
	);
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(P));
}

void Camera::Move(float x, float y, float z)
{
	XMFLOAT3 dt = XMFLOAT3(x, y, z);
	XMVECTOR d = XMLoadFloat3(&dt);
	XMVECTOR rotQ = XMLoadFloat4(&rotQuat);
	XMStoreFloat3(&dt, XMVector3Rotate(d, rotQ));
	camPos.x += dt.x;
	camPos.y += dt.y;
	camPos.z += dt.z;
}

void Camera::Turn(float x, float y)
{
	xRot += x;
	yRot += y;
}

void Camera::Update()
{
	XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYaw(xRot, yRot, 0);
	XMStoreFloat4(&rotQuat, rotationQuat);
	XMVECTOR newDir = XMVectorSet(0, 0, 1, 0);//XMLoadFloat3(&dir);
	//apply the roation to direction
	newDir = XMVector3Rotate(newDir, rotationQuat);
	XMMATRIX newView = XMMatrixLookToLH(XMLoadFloat3(&camPos), newDir, XMLoadFloat3(&up));
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(newView));
}

