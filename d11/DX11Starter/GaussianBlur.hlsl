cbuffer Data : register (b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
}

//input to pixel shader
//match the vs
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

//Textures
Texture2D Pixels       :   register(t0);
Texture2D Pixels2      :   register(t1);
SamplerState Sampler   :   register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	//keep track of total color
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;

	//blur <---- 2 passes
	for (int y = -blurAmount; y <= blurAmount; y++)
	{
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			float2 uv = input.uv + float2(x*pixelWidth, y*pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}
	totalColor = totalColor / numSamples;
	//totalColor += Pixels2.Sample(Sampler, input.uv);
	return totalColor;
}