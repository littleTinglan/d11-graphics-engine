#include "Game.h"
#include "Vertex.h"
// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	dShader = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete dShader;
	delete pShader2;
	delete blurShader;
	delete ppVS;
	delete ppExtract;
	delete ppBlur;
	delete addBlend;
	delete skyVS;
	delete skyPS;
	delete dofPS;
	
	for (auto& m : meshes)delete m;
	for (auto& e : entities)delete e;

	delete myCam;

	delete material;
	delete mat2;
	delete mat3;
	delete blurMat;

	rockSRV->Release();
	floorSRV->Release();
	rockNormal->Release();
	sampler->Release();
	bloomSampler->Release();
	dofSampler->Release();
	skyRasterState->Release();
	skyDepthState->Release();

	ppRTV->Release();
	ppSRV->Release();
	extractBrightRTV->Release();
	extractSRV->Release();
	blurRTV->Release();
	blurSRV->Release();
	skySRV->Release();
	depthRTV->Release();
	depthSRV->Release();
	dofRTV->Release();
	dofBlurRTV->Release();
	dofBlurSRV->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later

	myCam = new Camera(width, height, zNear, zFar);
	myCam->Start();

	LoadShaders();

	//load textures
	CreateWICTextureFromFile(device, context, L"Textures/gray.jpg", 0, &floorSRV);
	CreateWICTextureFromFile(device, context, L"Textures/rock.jpg", 0, &rockSRV);
	CreateWICTextureFromFile(device, context, L"Textures/rockNormals.jpg", 0, &rockNormal);
	//CreateWICTextureFromFile(device, context, L"Textures/skybox.png", 0, &skyboxSRV);

	CreateDDSTextureFromFile(
		device,
		L"Textures/nightSkybox.dds",
		0,
		&skySRV);

	// Manually create a sampler state
	D3D11_SAMPLER_DESC samplerDesc = {}; // Zero out the struct memory
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	// Sampler for bloom
	D3D11_SAMPLER_DESC samplerDesc1 = {}; // Zero out the struct memory
	samplerDesc1.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc1.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc1.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc1.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc1.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc1, &bloomSampler);

	// Sampler for dof
	D3D11_SAMPLER_DESC samplerDesc2 = {}; // Zero out the struct memory
	samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc2, &dofSampler);

	//skybox sampler state
	D3D11_RASTERIZER_DESC rd = {}; // Remember to zero it out!
	rd.CullMode = D3D11_CULL_FRONT;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.DepthClipEnable = true;
	device->CreateRasterizerState(&rd, &skyRasterState);

	// Depth state for accepting pixels with depth EQUAL to existing depth
	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	material = new Material(vertexShader, pixelShader, rockSRV, rockNormal, sampler);
	mat2 = new Material(vertexShader, dShader);
	mat3 = new Material(vertexShader, pShader2, floorSRV, sampler);
	blurMat = new Material(vertexShader, blurShader, rockSRV, rockNormal, sampler);
	CreateMatrices();

	CreateBasicGeometry();

	//set mouse pos
	prevMousePos.x = width / 2;
	prevMousePos.y = height / 2;
//Lights------------------
	light = { XMFLOAT4(0.1,0.1,0.1,1.0),                  //ambient color
			  XMFLOAT4(0.2, 0.2, 0.2, 1),           //diffuse color
			  XMFLOAT3(1, -1, 0), 1.0 };                  //direction
	light2 = { XMFLOAT4(0.1, 0.1, 0.1, 1.0),              //ambient color
			   XMFLOAT4(0.4, 0.478, 0.980, 1),          //diffuse color               
			   XMFLOAT3(5, -2 , -5), 1.0 };               //light position
	light3 = { XMFLOAT4(0.01, 0.01, 0.01, 1.0),           //ambient
			   XMFLOAT4(0.8, 0.160, 0.074, 1.0),          //Diffuse            
			   XMFLOAT3(-1.0, -0.5, 0),                        //Direction
			   30.0f,                                     //cone
			   XMFLOAT3(7, 2, 0),                        //Light Pos
			   120.0f };                                    //Range
	light4 = { XMFLOAT4(0.01, 0.01, 0.01, 1.0),           //ambient
		XMFLOAT4(0.992, 0.882, 0.227, 1.0),          //Diffuse            
		XMFLOAT3(1, -0.2, 0),                        //Direction
		30.0f,                                     //cone
		XMFLOAT3(-7.0f, 2.0f, 0.0f),              //Light Pos
		80.0f };                                    //Range
	light5 = { XMFLOAT4(0.01, 0.01, 0.01, 1.0),           //ambient
		XMFLOAT4(0.8, 0.8, 0.8, 1.0),          //Diffuse            
		XMFLOAT3(0, -1, 1),                        //Direction
		40.0f,                                     //cone
		XMFLOAT3(0.0f, 30.0f, -30.0f),              //Light Pos
		100.0f };                                    //Range
//Lights end------------------------------------

	//pass light to pixel shader
	pixelShader->SetData("light", &light, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &light2, sizeof(PointLight));
	pixelShader->SetData("light3", &light3, sizeof(SpotLight));
	pixelShader->SetData("light4", &light4, sizeof(SpotLight));
	pixelShader->SetData("light5", &light5, sizeof(SpotLight));
	pixelShader->SetFloat("shininess", 64.0f);
	pixelShader->SetFloat3("camPos", myCam->getCamPos());

	pShader2->SetData("light", &light, sizeof(DirectionalLight));
	pShader2->SetData("light2", &light2, sizeof(PointLight));
	pShader2->SetData("light3", &light3, sizeof(SpotLight));
	pShader2->SetData("light4", &light4, sizeof(SpotLight));
	pShader2->SetData("light5", &light5, sizeof(SpotLight));
	pShader2->SetFloat3("camPos", myCam->getCamPos());

	blurShader->SetData("light", &light, sizeof(DirectionalLight));
	blurShader->SetData("light2", &light2, sizeof(PointLight));
	blurShader->SetData("light3", &light3, sizeof(SpotLight));
	blurShader->SetData("light4", &light4, sizeof(SpotLight));
	blurShader->SetData("light5", &light5, sizeof(SpotLight));
	blurShader->SetFloat("shininess", 64.0f);
	blurShader->SetFloat3("camPos", myCam->getCamPos());

	//post processing stuff---------------------------
	//Bloom ------------------------------
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);
	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &ppRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &ppSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();

	ID3D11Texture2D* extractTexture;
	device->CreateTexture2D(&textureDesc, 0, &extractTexture);
	device->CreateRenderTargetView(extractTexture, &rtvDesc, &extractBrightRTV);

	// Create the Shader Resource View
	device->CreateShaderResourceView(extractTexture, &srvDesc, &extractSRV);

	// We don't need the texture reference itself no mo'
	extractTexture->Release();

	ID3D11Texture2D* blurTexture;
	device->CreateTexture2D(&textureDesc, 0, &blurTexture);
	// Create the Render Target View
	device->CreateRenderTargetView(blurTexture, &rtvDesc, &blurRTV);

	// Create the Shader Resource View
	device->CreateShaderResourceView(blurTexture, &srvDesc, &blurSRV);

	// We don't need the texture reference itself no mo'
	blurTexture->Release();

	//Depth of Field --------------------
	ID3D11Texture2D* depthTexture;
	//textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	device->CreateTexture2D(&textureDesc, 0, &depthTexture);
	// Create the Render Target View
	device->CreateRenderTargetView(depthTexture, &rtvDesc, &depthRTV);

	// Create the Shader Resource View
	device->CreateShaderResourceView(depthTexture, &srvDesc, &depthSRV);

	// We don't need the texture reference itself no mo'
	depthTexture->Release();

	ID3D11Texture2D* dofTex;
	device->CreateTexture2D(&textureDesc, 0, &dofTex);
	// Create the Render Target View
	device->CreateRenderTargetView(dofTex, &rtvDesc, &dofRTV);
	// Create the Shader Resource View
	device->CreateShaderResourceView(dofTex, &srvDesc, &dofSRV);
	
	dofTex->Release();

	ID3D11Texture2D* dofBlurTex;
	//textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	device->CreateTexture2D(&textureDesc, 0, &dofBlurTex);
	// Create the Render Target View
	device->CreateRenderTargetView(dofBlurTex, &rtvDesc, &dofBlurRTV);

	// Create the Shader Resource View
	device->CreateShaderResourceView(dofBlurTex, &srvDesc, &dofBlurSRV);

	// We don't need the texture reference itself no mo'
	dofBlurTex->Release();

	//Post Process Finished--------------------------------

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");  //'L' to make it a wide string

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	//have a check here if!=NULL 
	dShader = new SimplePixelShader(device, context);
	dShader->LoadShaderFile(L"defaultPixelShader.cso");

	//have a check here if!=NULL 
	pShader2 = new SimplePixelShader(device, context);
	pShader2->LoadShaderFile(L"PixelShader2.cso");

	//have a check here if!=NULL 
	blurShader = new SimplePixelShader(device, context);
	blurShader->LoadShaderFile(L"BlurShader.cso");

	ppVS = new SimpleVertexShader(device, context);
	ppVS->LoadShaderFile(L"PostProcessVS.cso");

	ppExtract = new SimplePixelShader(device, context);
	ppExtract->LoadShaderFile(L"ExtractBrightPS.cso");

	ppBlur = new SimplePixelShader(device, context);
	ppBlur->LoadShaderFile(L"GaussianBlur.cso");

	addBlend = new SimplePixelShader(device, context);
	addBlend->LoadShaderFile(L"AdditiveBlend.cso");

	// Load the shaders for the sky
	skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	dofPS = new SimplePixelShader(device, context);
	dofPS->LoadShaderFile(L"dofPS.cso");
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		180.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	myMesh1 = new Mesh("Assets/cube.obj", device);
	myMesh2 = new Mesh("Assets/sphere.obj", device);
	myMesh3 = new Mesh("Assets/trees.obj", device);
	meshes.push_back(myMesh1);
	meshes.push_back(myMesh2);
	meshes.push_back(myMesh3);

	myEnt1 = new Entity(myMesh2, context, material);
	//myEnt1 = new Entity(myMesh2, context, mat3);
	myEnt2 = new Entity(myMesh2, context, mat2);
	myEnt3 = new Entity(myMesh2, context, mat2);
	myEnt4 = new Entity(myMesh2, context, mat2);
	myEnt5 = new Entity(myMesh2, context, mat2);
	ground = new Entity(myMesh1, context, mat3);
	myEnt6 = new Entity(myMesh2, context, material);
	trees = new Entity(myMesh3, context, mat3);

	entities.push_back(myEnt1);
	entities.push_back(myEnt2);
	entities.push_back(myEnt3);
	entities.push_back(myEnt4);
	entities.push_back(myEnt5);
	entities.push_back(ground);
	entities.push_back(myEnt6);
	entities.push_back(trees);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	if (GetAsyncKeyState('W') &0x8000)
	{
		myCam->Move(0, 0, 0.05f);
		myCam->Update();
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		myCam->Move(0, 0, -0.05f);
		myCam->Update();
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		myCam->Move(0.05f, 0, 0);
		myCam->Update();
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		myCam->Move(-0.05f, 0, 0);
		myCam->Update();
	}
	if (GetAsyncKeyState('X') & 0x8000)
	{
		myCam->MoveUpDown(0.01f);
		myCam->Update();
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		myCam->MoveUpDown(-0.01f);
		myCam->Update();
	}
	myEnt1->SetScale(5, 5, 5);
	//myEnt1->Update(deltaTime);
	myEnt1->Update();

	myEnt6->SetScale(5, 5, 5);
	myEnt6->SetPos(-5, 0, -10);
	myEnt6->Update();

	trees->SetPos(0, -10, 20);
	trees->Update();

	myEnt2->SetScale(0.5, 0.5, 0.5);
	myEnt2->SetPos(-3, 3, 0);
	myEnt2->Update();
	myEnt3->SetScale(0.5, 0.5, 0.5);
	myEnt3->SetPos(5, -2, -5);
	myEnt3->Update();

	myEnt4->SetScale(0.5, 0.5, 0.5);
	myEnt4->SetPos(7, 2, 0);
	myEnt4->Update();

	myEnt5->SetScale(0.5, 0.5, 0.5);
	myEnt5->SetPos(-5, 2, 0);
	myEnt5->Update();

	ground->SetPos(0, -10, 0);
	ground->SetScale(150, 0.5, 150);
	ground->Update();

	
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	RTarray[0] = ppRTV;
	RTarray[1] = depthRTV;

	//update cam pos
	//pixelShader->SetFloat3("camPos", myCam->getCamPos());
	//pixelShader->CopyAllBufferData();

	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.05f, 0.05f, 0.05f, 0.5f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(ppRTV, color);
	context->ClearRenderTargetView(extractBrightRTV, color);
	context->ClearRenderTargetView(blurRTV, color);
	context->ClearRenderTargetView(depthRTV, color);
	context->ClearRenderTargetView(dofRTV, color);
	context->ClearRenderTargetView(dofBlurRTV, color);

	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->OMSetRenderTargets(2, &RTarray[0], depthStencilView);

	myEnt1->PrepareMaterial(myCam->getView(), myCam->getProj());
	pixelShader->SetFloat3("camPos", myCam->getCamPos());
	pixelShader->SetFloat4("dofPara", dofPara);
	myEnt1->Draw();

	myEnt6->PrepareMaterial(myCam->getView(), myCam->getProj());
	pixelShader->SetFloat3("camPos", myCam->getCamPos());
	pixelShader->SetFloat4("dofPara", dofPara);
	myEnt6->Draw();
	
	dShader->SetFloat4("color", XMFLOAT4(0.992, 0.709, 0.972, 0));
	dShader->SetFloat4("dofPara", dofPara);
	myEnt2->SolidMaterial(myCam->getView(),myCam->getProj());
	myEnt2->Draw();

	dShader->SetFloat4("color", XMFLOAT4(0.305, 0.388, 0.894, 0));
	dShader->SetFloat4("dofPara", dofPara);
	myEnt3->SolidMaterial(myCam->getView(),myCam->getProj());
	myEnt3->Draw();
	
	dShader->SetFloat4("color", XMFLOAT4(0.8, 0.160, 0.074, 0));
	dShader->SetFloat4("dofPara", dofPara);
	myEnt4->SolidMaterial(myCam->getView(),myCam->getProj());
	myEnt4->Draw();

	dShader->SetFloat4("color", XMFLOAT4(1, 0.925, 0.478, 0));
	dShader->SetFloat4("dofPara", dofPara);
	myEnt5->SolidMaterial(myCam->getView(), myCam->getProj());
	myEnt5->Draw();

	pShader2->SetFloat3("camPos", myCam->getCamPos());
	pShader2->SetFloat4("dofPara", dofPara);
	ground->PrepareMat(myCam->getView(), myCam->getProj());
	ground->Draw();

	pShader2->SetFloat3("camPos", myCam->getCamPos());
	pShader2->SetFloat4("dofPara", dofPara);
	trees->PrepareMat(myCam->getView(), myCam->getProj());
	trees->Draw();

//set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

//skybox ---------------------------
// Draw the sky LAST - Ideally, we've set this up so that it
// only keeps pixels that haven't been "drawn to" yet (ones that
// have a depth of 1.0)
	ID3D11Buffer* skyVB = myMesh1->GetVertexBuffer();
	ID3D11Buffer* skyIB = myMesh1->GetIndexBuffer();

	// Set the buffers
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	//skyVS->SetMatrix4x4("world", );
	skyVS->SetMatrix4x4("view", myCam->getView());
	skyVS->SetMatrix4x4("projection", myCam->getProj());
	skyVS->CopyAllBufferData();
	skyVS->SetShader();

	skyPS->SetFloat4("dofPara", dofPara);
	skyPS->SetShaderResourceView("SkyTexture", skySRV);
	skyPS->SetSamplerState("BasicSampler", sampler);
	skyPS->SetShader();

	// Set up the render state options
	context->RSSetState(skyRasterState);
	context->OMSetDepthStencilState(skyDepthState, 0);
	// Finally do the actual drawing
	context->DrawIndexed(myMesh1->GetIndexCount(), 0, 0);
	// Reset any states we've changed for the next frame!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

//Post Process--------------------
	
	//post process
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->OMSetRenderTargets(1, &extractBrightRTV, depthStencilView);
	//Extract Bright Pixels--------------------
	//set to post processing shaders
	ppVS->SetShader();
	ppExtract->SetShader();
	ppExtract->SetShaderResourceView("Pixels", ppSRV);
	ppExtract->SetSamplerState("Sampler", sampler);
	ppExtract->CopyAllBufferData();

	//unbind vert/index buffer
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	ppExtract->SetShaderResourceView("Pixels", 0);


	//Blur-----------------
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->OMSetRenderTargets(1, &blurRTV, 0);
	ppVS->SetShader();
	ppBlur->SetShader();
	ppBlur->SetShaderResourceView("Pixels", extractSRV);
	ppBlur->SetShaderResourceView("Pixels2", ppSRV);
	ppBlur->SetSamplerState("Sampler", bloomSampler);
	ppBlur->SetInt("blurAmount", 15);
	ppBlur->SetFloat("pixelWidth", 1.0f / width);
	ppBlur->SetFloat("pixelHeight", 1.0f / height);
	ppBlur->CopyAllBufferData();
	//unbind vert/index buffer
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	ppBlur->SetShaderResourceView("Pixels", 0);
	ppBlur->SetShaderResourceView("Pixels2", 0);

	//Additive Blend-------------------------
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->OMSetRenderTargets(1, &dofRTV, 0);
	ppVS->SetShader();
	addBlend->SetShader();
	addBlend->SetShaderResourceView("Original", ppSRV);
	addBlend->SetShaderResourceView("Blurred", blurSRV);
	addBlend->SetSamplerState("Sampler", bloomSampler);
	addBlend->CopyAllBufferData();
	//unbind vert/index buffer
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	addBlend->SetShaderResourceView("Original", 0);
	addBlend->SetShaderResourceView("Blurred", 0);

	//DOF-----------------------------------------
	//Blur-----------------
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->OMSetRenderTargets(1, &dofBlurRTV, 0);
	ppVS->SetShader();
	ppBlur->SetShader();
	ppBlur->SetShaderResourceView("Pixels", dofSRV);
	ppBlur->SetShaderResourceView("Pixels2", ppSRV);
	ppBlur->SetSamplerState("Sampler", bloomSampler);
	ppBlur->SetInt("blurAmount", 10);
	ppBlur->SetFloat("pixelWidth", 1.0f / width);
	ppBlur->SetFloat("pixelHeight", 1.0f / height);
	ppBlur->CopyAllBufferData();
	//unbind vert/index buffer
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	ppBlur->SetShaderResourceView("Pixels", 0);
	ppBlur->SetShaderResourceView("Pixels2", 0);

	//Blur using depth buffer
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	ppVS->SetShader();
	dofPS->SetShader();
	dofPS->SetFloat("pixelSize", 1.0f / (width*height));
	dofPS->SetShaderResourceView("Pixels", dofSRV);
	dofPS->SetShaderResourceView("Pixels2", dofBlurSRV);
	dofPS->SetShaderResourceView("Pixels3", depthSRV);
	dofPS->SetSamplerState("dofSampler", dofSampler);
	dofPS->CopyAllBufferData();
	//unbind vert/index buffer
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	dofPS->SetShaderResourceView("Pixels", 0);
	dofPS->SetShaderResourceView("Pixels2", 0);
	dofPS->SetShaderResourceView("Pixels3", 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	
	// Add any custom code here...
	float dx = (prevMousePos.y - y)*0.005;
	float dy = -(prevMousePos.x - x)*0.005;
	
	myCam->Turn(dx, dy);
	myCam->Update();

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion