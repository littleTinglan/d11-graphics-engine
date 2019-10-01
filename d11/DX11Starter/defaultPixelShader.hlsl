
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD;
	float3 normal       : NORMAL;
	float3 tangent      : TANGENT;
	float3 worldPos     : POSITION;
	float depth         : DEPTH;        //depth in view space
};

struct PixelOut
{
	float4 color    : SV_TARGET0;
	float4 distance : SV_TARGET1;
};

//constant buffer
cbuffer externalData : register(b0)
{
	float4 color;

	float4 dofPara;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
PixelOut main(VertexToPixel input)
{
	PixelOut output;

	output.color = float4(color.rgb, 0);

	float f = 0.0f;
	if (input.depth < dofPara.y)
	{
		f = (input.depth - dofPara.y) / (dofPara.y - dofPara.x);
	}
	else
	{
		f = (input.depth - dofPara.y) / (dofPara.z - dofPara.y);
		f = clamp(f, 0, dofPara.w);
	}
	output.distance = float4((f*0.5f + 0.5f), 0, 0, 0);
	//float f = 1 - (input.position.z / input.position.w);
	//output.distance = float4(f, 0, 0, 0);

	return output;
}