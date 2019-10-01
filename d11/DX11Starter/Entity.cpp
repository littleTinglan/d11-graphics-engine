#include "Entity.h"



Entity::Entity()
{
}


Entity::Entity(Mesh * myM, ID3D11DeviceContext* c, Material* m)
{
	myMesh = myM;
	context = c;
	myMat = m;
	vShader = myMat->getVertexShader();
	pShader = myMat->getPixelShader();
	//set default value
	XMMATRIX ident = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMat, ident);

	pos = XMFLOAT3(0.0, 0.0, 0.0);
	rot = XMFLOAT3(0.0, 0.0, 0.0);
	scale = XMFLOAT3(1.0, 1.0, 1.0);

}

Entity::~Entity()
{
	// DO NOT DELETE myMesh HERE!!!
	// Because if there are multiple entities sharing the same mesh
	// It will try to delete the mesh multiple times
	// Should be cleanning in the Game.cpp <-- because that's here we used 'new' to create meshes
}


XMFLOAT3 Entity::SetPos(float x, float y, float z)
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
	return pos;
}

XMFLOAT3 Entity::SetRot(float x, float y, float z)
{
	rot.x = x;
	rot.y = y;
	rot.z = z;
	return rot;
}

XMFLOAT3 Entity::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	return scale;
}

void Entity::Update()
{
	//passing in worldMatrix before draw
	XMMATRIX trans = XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX wm = s * rotation*trans;

	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(wm));
}

void Entity::Update(float dt)
{
	rot.y += 0.1*dt;
	//passing in worldMatrix before draw
	XMMATRIX trans = XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX wm = s * rotation*trans;

	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(wm));

}

void Entity::PrepareMaterial(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat)
{
	vShader->SetMatrix4x4("view", viewMat);
	vShader->SetMatrix4x4("projection", projMat);
	pShader->SetSamplerState("basicSampler", myMat->getSampler());
	pShader->SetShaderResourceView("diffuseMap", myMat->getSRV());
	pShader->SetShaderResourceView("normalMap", myMat->getNormalSRV());
	vShader->SetShader();
	pShader->SetShader();
}

void Entity::PrepareMat(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat)
{
	vShader->SetMatrix4x4("view", viewMat);
	vShader->SetMatrix4x4("projection", projMat);
	pShader->SetSamplerState("basicSampler", myMat->getSampler());
	pShader->SetShaderResourceView("diffuseMap", myMat->getSRV());
	vShader->SetShader();
	pShader->SetShader();
}

void Entity::SolidMaterial(XMFLOAT4X4 viewMat, XMFLOAT4X4 projMat)
{
	vShader->SetMatrix4x4("view", viewMat);
	vShader->SetMatrix4x4("projection", projMat);
	vShader->SetShader();
	pShader->SetShader();
}

void Entity::Draw()
{
	//set buffers
	vBuffer = myMesh->GetVertexBuffer();
	UINT stride = sizeof(Vertex);
	UINT  offset = 0;
	
	context->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
	context->IASetIndexBuffer(myMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	
	vShader->SetMatrix4x4("world", worldMat);
	vShader->CopyAllBufferData();
	pShader->CopyAllBufferData();

	//draw
	context->DrawIndexed(myMesh->GetIndexCount(), 0, 0);
}
