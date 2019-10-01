
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
	//float4 color		: COLOR;
};

//direction light struct
struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float pad;
};
struct PointLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 LightPos;
	float pad;
};
struct SpotLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float cone;
	float3 LightPos;
	float range;
};

//constant buffer
cbuffer externalData : register(b0)
{
	DirectionalLight light;
	//DirectionalLight light2;
	PointLight light2;
	SpotLight light3;
	SpotLight light4;
	SpotLight light5;

	//the camera
	float3 camPos;

	float4 dofPara;
};

//textures
Texture2D diffuseMap : register(t0);
SamplerState basicSampler : register(s0);

struct PixelOut
{
	float4 color    : SV_TARGET0;
	float4 distance : SV_TARGET1;
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

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering

	//inputs
	float3 N = normalize(input.normal);

	//directional light calculation
	float3 lightDir = normalize(-light.Direction);
	float totalLight = dot(N, lightDir);
	totalLight = saturate(totalLight);
	float4 color1 = light.DiffuseColor * totalLight + light.AmbientColor;
	//return color1;

	//point light calculation
	//diffuse
	float3 pLightDir = normalize(input.worldPos - light2.LightPos);
	float NdotL = dot(N, -pLightDir); //diffuse light no attenuation
	NdotL = saturate(NdotL);
	//specular lighting
	float3 dirToCam = normalize(camPos - input.worldPos);
	float3 refl = reflect(pLightDir, N);
	float spec = pow(max(dot(refl, dirToCam), 0), 64);
	//attenuation
	float dis = length(input.worldPos - light2.LightPos);
	float attenuation = 1.0f / (1.0 + 0.1*dis*dis);
	float4 color2 = (light2.DiffuseColor * NdotL) *attenuation;
	//float4 color2 = light2.DiffuseColor * NdotL + light2.AmbientColor;
	//return float4(atten,atten,atten,1);
	//return color2;

	//spot light calculation
	float distance = length(input.worldPos - light3.LightPos);
	float3 sLightDir = normalize(input.worldPos - light3.LightPos);
	float sNdotL = dot(N, -sLightDir); //diffuse light no attenuation
	sNdotL = saturate(sNdotL);
	float atten = saturate(1.0 - distance*distance / (light3.range*light3.range));
	atten *= atten;
	float angleFromCenter = max(dot(-sLightDir, normalize(-light3.Direction)), 0.0f);
	float spotAmount = pow(angleFromCenter, 45.0f - light3.cone);
	float4 color3 = (light3.DiffuseColor * sNdotL * spotAmount + light3.AmbientColor) *atten;

	//spot light calculation
	float distance1 = length(input.worldPos - light4.LightPos);
	float3 sLightDir1 = normalize(input.worldPos - light4.LightPos);
	float sNdotL1 = dot(N, -sLightDir1); //diffuse light no attenuation
	sNdotL1 = saturate(sNdotL1);
	float atten1 = saturate(1.0 - distance1*distance1 / (light4.range*light4.range));
	atten1 *= atten1;
	float angleFromCenter1 = max(dot(-sLightDir1, normalize(-light4.Direction)),0.0f);
	float spotAmount1 = pow(angleFromCenter1, 45.0f - light4.cone);
	float4 color4 = (light4.DiffuseColor * sNdotL1 * spotAmount1 + light4.AmbientColor) *atten1;

	//spot light calculation
	float distance2 = length(input.worldPos - light5.LightPos);
	float3 sLightDir2 = normalize(input.worldPos - light5.LightPos);
	float sNdotL2 = dot(N, -sLightDir2); //diffuse light no attenuation
	sNdotL2 = saturate(sNdotL2);
	float atten2 = saturate(1.0 - distance2*distance2 / (light5.range*light5.range));
	atten2 *= atten2;
	float angleFromCenter2 = max(dot(-sLightDir2, normalize(-light5.Direction)), 0.0f);
	float spotAmount2 = pow(angleFromCenter2, 45.0f - light5.cone);
	float4 color5 = (light5.DiffuseColor * sNdotL2 * spotAmount2 + light5.AmbientColor) *atten2;
	//return color5;
	//return color1 + color2 + color4 + color3 + color5 *0.5;

	float4 textureColor = diffuseMap.Sample(basicSampler, input.uv);

	output.color = float4(textureColor.rgb * (color1 + color2 + color3 + color4 + color5).rgb, 1);

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