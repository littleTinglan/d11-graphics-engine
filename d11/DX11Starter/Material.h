#pragma once
#include "SimpleShader.h"

//represents a set of vertex and pixel shaders
//needed to be updated in Entity
class Material
{
public:
	SimplePixelShader* pShader;
	SimpleVertexShader* vShader;

	Material();
	~Material();
	//material only has a color as output
	Material(SimpleVertexShader* v, SimplePixelShader* p);
	//w/ diffuse map
	Material(SimpleVertexShader* v, SimplePixelShader* p, ID3D11ShaderResourceView* srv, ID3D11SamplerState* s);
	//w/ diffuse & normal maps
	Material(SimpleVertexShader* v, SimplePixelShader* p, ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* srvN, ID3D11SamplerState* s);


	//getter
	SimplePixelShader* getPixelShader() { return pShader; }
	SimpleVertexShader* getVertexShader() { return vShader; }
	ID3D11ShaderResourceView* getSRV() { return mySRV; }
	ID3D11ShaderResourceView* getNormalSRV() { return myNormalSRV; }
	ID3D11SamplerState* getSampler() { return sampler; }
	//setter
	void setSRV(ID3D11ShaderResourceView* t) { myNormalSRV = t; }

	//texture related
	ID3D11ShaderResourceView* mySRV;
	ID3D11ShaderResourceView* myNormalSRV;
	ID3D11SamplerState* sampler;
};

