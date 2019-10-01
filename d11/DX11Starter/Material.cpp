#include "Material.h"



Material::Material()
{
}


Material::~Material()
{
}

Material::Material(SimpleVertexShader * v, SimplePixelShader * p)
{
	pShader = p;
	vShader = v;
}

Material::Material(SimpleVertexShader * v, SimplePixelShader * p, ID3D11ShaderResourceView* srv, ID3D11SamplerState* s)
{
	pShader = p;
	vShader = v;
	mySRV = srv;
	sampler = s;
}

Material::Material(SimpleVertexShader * v, SimplePixelShader * p, ID3D11ShaderResourceView * srv, ID3D11ShaderResourceView * srvN, ID3D11SamplerState * s)
{
	pShader = p;
	vShader = v;
	mySRV = srv;
	myNormalSRV = srvN;
	sampler = s;
}
