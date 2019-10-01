#pragma once
#include "Vertex.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <iostream>
#include <fstream>
#include <vector>

//hold geometry data (vertives & indices)
//should both create and use the buffer
class Mesh 
{
public:
	//default constructor
	Mesh();
	Mesh(Vertex vert[], int vertC, //vertex
		unsigned int index[], int indiceCount, //index
		ID3D11Device* d);
	Mesh(char* file, ID3D11Device* device);
	//destructor
	~Mesh(); //calls Release() for both buffers

	//attributes
	//Vertex vertices[]; 
	//why is this not allowed? "Incomplete type is not allowed"
	//but would allow me to do this in Private
	Vertex* vertices;
	unsigned int* indices;

	//geter
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();

private:
	//buffers
	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	int vertCount; // how many indices are in the index buffer
	int indCount;

	ID3D11Device* d;
	//functions
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	void Init(); //this will create the buffers 

};

