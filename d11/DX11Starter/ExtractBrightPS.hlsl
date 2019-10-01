//input
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

//textures and such
Texture2D Pixels        : register(t0);
SamplerState Sampler    : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	//sample color for the current uv
	float4 finalColor = Pixels.Sample(Sampler, input.uv);
	//return finalColor;
	//determine if this pixel is bright or not
	//if it is bright enough, keep it
	//if it is not, make it black
	float brightness = 0.2126f * finalColor.r + 0.7152f * finalColor.g + 0.0722f * finalColor.b;
	//light object, always bloom
	if (finalColor.a == 0)
	{
		return float4(finalColor.rgb, 0);
	}
	else if (finalColor.a == 1)
	{
		//others 
		if (brightness <= 0.72f)
		{
			finalColor.rgb = float3(0, 0, 0);
		}
	}
	//skybox no bloom
	else
	{
		if (brightness <= 0.75f)
		{
			finalColor.rgb = float3(0, 0, 0);
		}
		else
		{
			finalColor.rgb /= 5.0;
		}
	}
	
	return finalColor;
}