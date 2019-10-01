

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD0;
	float depth         : DEPTH;        //depth in view space
};

struct PixelOut
{
	float4 color    : SV_TARGET0;
	float4 distance : SV_TARGET1;
};

// Texture-related variables
TextureCube SkyTexture		: register(t0);
SamplerState BasicSampler	: register(s0);

cbuffer externalData : register(b0)
{
	float4 dofPara;
};

// Entry point for this pixel shader
PixelOut main(VertexToPixel input)
{
	PixelOut output;
	float4 color = float4(0,0,0,0.5);
	color = SkyTexture.Sample(BasicSampler, input.uvw);
	color.a = 0.5;
	output.color = color;

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
	//output.distance = float4((f*0.5f + 0.5f), 0, 0, 0);
	output.distance = float4(1, 1, 1, 1);
	return output;
}