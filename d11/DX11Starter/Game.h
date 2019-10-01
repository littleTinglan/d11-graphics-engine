#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Light.h"
#include "pch.h"
#include <vector>

#define M_PI 3.141592653589

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();

	//Render Target Array
	ID3D11RenderTargetView* RTarray[2];

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	SimplePixelShader* pShader2;
	SimplePixelShader* dShader;
	SimplePixelShader* blurShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//entity
	std::vector<Entity*> entities;
	Entity* myEnt1;
	Entity* myEnt2;
	Entity* myEnt3;
	Entity* myEnt4;
	Entity* myEnt5;
	Entity* ground;
	Entity* myEnt6;
	Entity* trees;

	//Meshes
	std::vector<Mesh*> meshes;
	Mesh* myMesh1;
	Mesh* myMesh2;
	Mesh* myMesh3;

	//camera
	Camera* myCam;
	float zNear = 0.1f;
	float zFar = 100.0f;
	XMFLOAT4 dofPara = XMFLOAT4(2,8,20,15);
	//x <-- near blur depth   
	//y <-- focal plane depth
	//z <-- far blur depth
	//w <-- blurriness cutoff constant for objects behind the focal plane

	//material
	Material* material;
	Material* mat2;
	Material* mat3;
	Material* blurMat;

	//light
	DirectionalLight light;
	PointLight light2;
	SpotLight light3;
	SpotLight light4;
	SpotLight light5;

	//texture
	ID3D11ShaderResourceView* rockSRV;
	ID3D11ShaderResourceView* rockNormal;
	ID3D11ShaderResourceView* floorSRV;
	ID3D11SamplerState* sampler;
	ID3D11SamplerState* bloomSampler;
	ID3D11SamplerState* dofSampler;

	//Skybox stuff---------------------
	ID3D11ShaderResourceView* skySRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;

	ID3D11RasterizerState* skyRasterState;
	ID3D11DepthStencilState* skyDepthState;

	//post processing
	//Bloom-----------------------------
	//Render Targets and Shader Resource Views
	ID3D11RenderTargetView* ppRTV; //render to a texture <--- before pp
	ID3D11ShaderResourceView* ppSRV; //to sample from the same texture <--- original
	
	ID3D11RenderTargetView* extractBrightRTV; //RRT for extracting bright pixels
	ID3D11ShaderResourceView* extractSRV; 

	ID3D11RenderTargetView* blurRTV; // RRT for Gaussian blur
	ID3D11ShaderResourceView* blurSRV;

	//shaders
	SimpleVertexShader* ppVS;
	SimplePixelShader* ppExtract;
	SimplePixelShader* ppBlur; //does 2 passes
	SimplePixelShader* addBlend;

	//Depth of Field ----------------------------------

	//shaders
	ID3D11RenderTargetView* depthRTV;
	ID3D11ShaderResourceView* depthSRV;

	ID3D11RenderTargetView* dofBlurRTV;
	ID3D11ShaderResourceView* dofBlurSRV;

	ID3D11RenderTargetView* dofRTV;
	ID3D11ShaderResourceView* dofSRV;

	SimplePixelShader* dofPS; //post process
	
};
