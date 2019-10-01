#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include "SimpleShader.h"
#include "Material.h"

using namespace DirectX;
class Entity
{
	//fields
	//need world matrix, pos, rot, scale, pointer to Mesh
	XMFLOAT4X4 worldMat;
	XMFLOAT3 pos, rot, scale;
	Mesh* myMesh;
	ID3D11Buffer* vBuffer;
	ID3D11DeviceContext* context;
	SimpleVertexShader* vShader;
	SimplePixelShader* pShader;
	Material* myMat;

public:
	//constructor
	Entity();
	//customlize constructor
	//should set default values for the matrix, transformation
	Entity(Mesh* myM, ID3D11DeviceContext* c, Material* m);
	~Entity();

	//geter & seter
	XMFLOAT4X4 GetMatrix() { return worldMat; };

	XMFLOAT3 SetPos(float x, float y, float z);
	XMFLOAT3 SetRot(float x, float y, float z);
	XMFLOAT3 SetScale(float x, float y, float z);

	//methods
	//should use the delta time instead of frame rate
	//cuz the frame rate can change when the program runs
	void Update();
	void Update(float dt); //should offset pos
	// MoveForward()

	void PrepareMaterial(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat);
	void PrepareMat(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat);
	void SolidMaterial(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat);
	//set buffers, draw commands
	// NOTE: more advanced engine might need a Renderer class
	// that makes decisions about what to render and when
	void Draw();

private:



};

