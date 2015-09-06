//==============================================================================
// Ocean Surface Vertex Shader
//
//==============================================================================
// History
//
// -Created on 5/30/2015 by Dustin Watson
//==============================================================================

#define PATCH_BLEND_BEGIN		800
#define PATCH_BLEND_END			20000

//
// Constants
//

// Shading parameters
cbuffer cbShading : register(b0)
{
	// Water-reflected sky color
	float3		g_SkyColor			: packoffset(c0.x);
	// The color of bottomless water body
	float3		g_WaterbodyColor	: packoffset(c1.x);

	// The strength, direction and color of sun streak
	float		g_Shineness			: packoffset(c1.w);
	float3		g_SunDir			: packoffset(c2.x);
	float3		g_SunColor			: packoffset(c3.x);
	
	// The parameter is used for fixing an artifact
	float3		g_BendParam			: packoffset(c4.x);

	// Perlin noise for distant wave crest
	float		g_PerlinSize		: packoffset(c4.w);
	float3		g_PerlinAmplitude	: packoffset(c5.x);
	float3		g_PerlinOctave		: packoffset(c6.x);
	float3		g_PerlinGradient	: packoffset(c7.x);

	// Constants for calculating texcoord from position
	float		g_TexelLength_x2	: packoffset(c7.w);
	float		g_UVScale			: packoffset(c8.x);
	float		g_UVOffset			: packoffset(c8.y);
};

// Per draw call constants
cbuffer cbChangePerCall : register(b1)
{
	// Transform matrices
	float4x4	g_matLocal;
	float4x4	g_matWorldViewProj;
	float4x4    g_matWorld;

	// Reflection matrix from environment
	float4x4    g_matWorldReflectionViewProj;

	// Misc per draw call constants
	float2		g_UVBase;
	float2		g_PerlinMovement;
	float3		g_LocalEye;

	// The sea level elevation of the ocean
	float       g_SeaLevel;
}

//======================================================================================================

//
// Textures and Samplers
//

Texture2D g_texDisplacement		: register(t0); // FFT wave displacement map in VS (wave displacement from ocean heightfield)
Texture2D g_texPerlin			: register(t1); // Perlin wave displacement

SamplerState g_pointSampler		: register(s0);
SamplerState g_perlinSampler	: register(s1);

//======================================================================================================

//
// Structs
//

struct VertexInput
{
	float3 position  : POSITION;
};

struct PixelInput
{
	float4 position  				: SV_POSITION;
	float2 uv        				: TEXCOORD0;
	float3 LocalPos	                : TEXCOORD1;
	float4 reflectionMapSamplingPos : TEXCOORD2;
	float4 refractionMapSamplingPos : TEXCOORD3;
	float4 position3D               : TEXCOORD4;
	float3 viewDirection            : TEXCOORD5;
};

//======================================================================================================

//
// Vertex Shader
//

PixelInput OceanSurfaceVS(VertexInput input)
{
	PixelInput output;
	
	// Local position
	float4 pos_local = mul(float4(input.position, 1), g_matLocal);
	// UV
	float2 uv_local = pos_local.xz * g_UVScale + g_UVOffset;
	
	// Blend displacement to avoid tiling artifact
	float3 eye_vec = pos_local.xyz - g_LocalEye;
	float dist_2d = length(eye_vec.xz);
	float blend_factor = (PATCH_BLEND_END - dist_2d) / (PATCH_BLEND_END - PATCH_BLEND_BEGIN);
	blend_factor = clamp(blend_factor, 0, 1);
	
	// Add perlin noise to distant patches
	float perlin = 0;
	if (blend_factor < 1)
	{
		float2 perlin_tc = uv_local * g_PerlinSize + g_UVBase;
		float perlin_0 = g_texPerlin.SampleLevel(g_perlinSampler, perlin_tc * g_PerlinOctave.x + g_PerlinMovement, 0).w;
		float perlin_1 = g_texPerlin.SampleLevel(g_perlinSampler, perlin_tc * g_PerlinOctave.y + g_PerlinMovement, 0).w;
		float perlin_2 = g_texPerlin.SampleLevel(g_perlinSampler, perlin_tc * g_PerlinOctave.z + g_PerlinMovement, 0).w;
		
		perlin = perlin_0 * g_PerlinAmplitude.x + perlin_1 * g_PerlinAmplitude.y + perlin_2 * g_PerlinAmplitude.z;
	}
	
	// Displacement map
	float3 displacement = 0;
	if (blend_factor > 0)
	{
		displacement = g_texDisplacement.SampleLevel(g_pointSampler, uv_local, 0).xyz;
	}
	
	displacement = lerp(float3(0, 0, perlin), displacement, blend_factor);
	pos_local.x += displacement.x;
	pos_local.y += displacement.y + g_SeaLevel;
	pos_local.z += displacement.z;
	
	// Transform
	output.position = mul(pos_local, g_matWorldViewProj);
	
	output.LocalPos = pos_local.xyz;
	
	// Calculate refraction and reflection view matrices
	output.reflectionMapSamplingPos = mul(pos_local, g_matWorldReflectionViewProj);
	output.refractionMapSamplingPos = mul(pos_local, g_matWorldViewProj);
	output.position3D = mul(pos_local, g_matWorld);
	
	//
	// Calculate the camera's view direction for fresnel calculations
	//
	
	// Calculate the position of the vertex in the world.
    float3 worldPosition = mul(pos_local, g_matWorld);
	
    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    output.viewDirection = g_LocalEye.xyz - worldPosition.xyz;
	
    // Normalize the viewing direction vector.
    output.viewDirection = normalize(output.viewDirection);
	
	// Pass thru texture coordinate
	output.uv = uv_local;
	
	return output; 
}