cbuffer Data : register (b0)
{
	float pixelSize;
}

// Defines the output data of our vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels           : register(t0); //original texture
Texture2D Pixels2          : register(t1); //blurred texture
Texture2D Pixels3          : register(t2); //depth texture
SamplerState dofSampler    : register(s0);

float2 vMaxCoC = float2(5.0, 10.0);

float4 main(VertexToPixel input) : SV_TARGET
{
	const int NUM_TAPS = 12;
	float2 poisson[NUM_TAPS];
	//POISSON DISC
	poisson[0] = float2(-.326, -.406);
	poisson[1] = float2(-.840, -.074);
	poisson[2] = float2(-.696, .475);
	poisson[3] = float2(-.203, .621);
	poisson[4] = float2(.962, -.195);
	poisson[5] = float2(.473, -.480);
	poisson[6] = float2(.519, .767);
	poisson[7] = float2(.185, -.893);
	poisson[8] = float2(.507, .064);
	poisson[9] = float2(.896, .412);
	poisson[10] = float2(-.322, -.933);
	poisson[11] = float2(-.792, -.598);

	
	//read from the depth texture
	float fDepth = Pixels3.Sample(dofSampler, input.uv).r;

	float4 cOut = Pixels.Sample(dofSampler, input.uv);
	float discRadius, centerDepth;
	centerDepth = cOut.r;
	//convert depth into blur radius
	discRadius = abs(cOut.a * vMaxCoC.y - vMaxCoC.x);
	cOut = 0;

	for (int t = 0; t < NUM_TAPS; t++)
	{
		//new texture coord
		float2 newUV = input.uv + pixelSize * poisson[t] * discRadius;
		//pixel color for the scene before pp
		float4 sceneTex = Pixels.Sample(dofSampler, newUV);
		//pixel color for the blurred scene
		float4 blurTex = Pixels2.Sample(dofSampler, newUV);
		//mix the blur texture and original texture
		float tapBlur = abs(sceneTex.a*2.0 - 1.0);
		float4 tap = lerp(sceneTex, blurTex, tapBlur);

		//"smart" blur ignores taps that are closer than the center tap and in focus
		tap.a = (tap.a >= centerDepth) ? 1.0 : abs(tap.a *2.0 - 1.0);

		cOut.rgb += tap.rgb * tap.a;
		cOut.a += tap.a;
	}
	float4 blurColor = (cOut / cOut.a);

	float4 oriTex = Pixels.Sample(dofSampler, input.uv);
	float4 blurTex = Pixels2.Sample(dofSampler, input.uv);

	//blend?
	float alpha = Pixels3.Sample(dofSampler, input.uv).r; //1 --- blur
														  //0 --- no blur
	float4 finalColor;
	finalColor = float4(blurColor.rgb * alpha + oriTex.rgb * (1 - alpha), 1.0);

	//return float4(alpha, alpha, alpha, 1);
	return finalColor;

}