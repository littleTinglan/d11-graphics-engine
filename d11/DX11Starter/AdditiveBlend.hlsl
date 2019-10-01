//input to pixel shader
//match the vs
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Original     :   register(t0);
Texture2D Blurred      :   register(t1);
SamplerState Sampler   :   register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = float4(0,0,0,1);
	color = Original.Sample(Sampler, input.uv);
	color += Blurred.Sample(Sampler, input.uv);
	return color;
}