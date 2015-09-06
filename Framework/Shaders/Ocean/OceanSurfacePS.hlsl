//==============================================================================
// Ocean Surface Pixel Shader
//
// Add Perlin Noise and Tessendorf's FFT later
//==============================================================================
// History
//
// -Created on 5/30/2015 by Dustin Watson
// -Updated 8/5/2015 by John Klima
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
// Globals
//

Texture2D g_texDisplacement		: register(t0); // FFT wave displacement map in VS (wave displacement from ocean heightfield)
Texture2D g_texPerlin			: register(t1); // Perlin wave displacement
Texture2D g_texGradient			: register(t2); // FFT wave gradient map in PS
Texture2D g_WaveMap0 	  		: register(t3); // First Normal map
Texture2D g_WaveMap1 	  		: register(t4); // Second Normal map
Texture2D g_ReflectionMap 		: register(t5);
Texture2D g_RefractionMap 		: register(t6);
Texture2D g_FresnelMap			: register(t7);
TextureCube g_texReflectCube 	: register(t8);// A small skybox cube texture for reflection

SamplerState g_perlinSampler	: register(s0);
SamplerState g_gradientSampler	: register(s1);
SamplerState g_fresnelSampler	: register(s2);
SamplerState g_cubeSampler		: register(s3);
SamplerState g_mirrorSampler	: register(s4);

//======================================================================================================

//
// Pixel Shader
//

struct PixelInput
{
	float4 position					: SV_POSITION;
	float2 uv						: TEXCOORD0;
	float3 LocalPos	                : TEXCOORD1;
	float4 reflectionMapSamplingPos : TEXCOORD2;
	float4 refractionMapSamplingPos : TEXCOORD3;
	float4 position3D               : TEXCOORD4;
	float3 viewDirection            : TEXCOORD5;
};

float4 OceanSurfacePS(PixelInput input) : SV_TARGET
{
	// Calculate eye vector.
	float3 eye_vec = g_LocalEye - input.LocalPos;
	float3 eye_dir = normalize(eye_vec);
	
	// --------------- Blend perlin noise for reducing the tiling artifacts
	
	// Blend displacement to avoid tiling artifact
	float dist_2d = length(eye_vec.xz);
	float blend_factor = (PATCH_BLEND_END - dist_2d) / (PATCH_BLEND_END - PATCH_BLEND_BEGIN);
	blend_factor = clamp(blend_factor * blend_factor * blend_factor, 0, 1);
	
	// Compose perlin waves from three octaves
	float2 perlin_tc = input.uv * g_PerlinSize + g_UVBase;
	float2 perlin_tc0 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.x + g_PerlinMovement : 0;
	float2 perlin_tc1 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.y + g_PerlinMovement : 0;
	float2 perlin_tc2 = (blend_factor < 1) ? perlin_tc * g_PerlinOctave.z + g_PerlinMovement : 0;

	float2 perlin_0 = g_texPerlin.Sample(g_perlinSampler, perlin_tc0).xy;
	float2 perlin_1 = g_texPerlin.Sample(g_perlinSampler, perlin_tc1).xy;
	float2 perlin_2 = g_texPerlin.Sample(g_perlinSampler, perlin_tc2).xy;

	float2 perlin = (perlin_0 * g_PerlinGradient.x + perlin_1 * g_PerlinGradient.y + perlin_2 * g_PerlinGradient.z);
	
	// --------------- Water body color
	
	// Texcoord mash optimization: Texcoord of FFT wave is not required when blend_factor > 1
	float2 fft_tc = (blend_factor > 0) ? input.uv : 0;
	
	float2 grad = g_texGradient.Sample(g_gradientSampler, fft_tc).xy;
	grad = lerp(perlin, grad, blend_factor);
	
	float3 normalPerlin = normalize(float3(grad.x, g_TexelLength_x2, grad.y));
	
	// Reflected ray
	float3 reflect_vec = reflect(-eye_dir, normalPerlin);
	
	float cos_angle = dot(normalPerlin, eye_dir);
	
	// A coarse way to handle transmitted light (WaterBodyColor)
	//<JPK> quick hack
	//float3 body_color = float3(0.07f, 0.10f, 0.15f);
	//float3 sky_color = float3(0.38f, 0.45f, 0.56f);
	//float3 bend_param = float3(0.1f, -0.4f, 0.2f);
	//float3 sun_color = float3(1.0f, 1.0f, 0.6f);
	
	// A coarse way to handle transmitted light (WaterBodyColor)
	float3 body_color = g_WaterbodyColor;
	
	// --------------- Reflected color (Gets added to the fresnel term later)
	
	// ramp.x for fresnel term. ramp.y for sky blending
	float4 ramp = g_FresnelMap.Sample(g_fresnelSampler, cos_angle).xyzw;
	
	// A workaround to deal with "indirect reflection vectors" (which are rays requiring multiple
	// reflections to reach the sky).
	if (reflect_vec.y < g_BendParam.x)
	{
		ramp = lerp(ramp, g_BendParam.y, (g_BendParam.x - reflect_vec.y) / (g_BendParam.x - g_BendParam.z));
	}
	
	reflect_vec.y = max(0, reflect_vec.y);
	reflect_vec.y = -reflect_vec.y;
	
	float3 reflectionP = g_texReflectCube.Sample(g_cubeSampler, reflect_vec).xyz;
	
	// Hack bit: making higher contrast
	reflectionP = reflectionP * reflectionP * 2.5f;
	
	// Blend with predefined sky color
	float3 reflected_color = lerp(g_SkyColor, reflectionP, ramp.y);
	
	// Combine waterbody color and reflected color
	float3 water_color = lerp(body_color, reflected_color, ramp.x);
	
	//float cos_spec = clamp(dot(reflect_vec, g_LightDirection), 0, 1);
	//float sun_spot = pow(cos_spec, g_SpecularShininess);
	//water_color += sun_color * sun_spot;
	//float4 final_color = float4(water_color.r, water_color.g, water_color.b, 1);
	
	// ----------------- Actual Scene Reflection and Refraction colors
	
	float4 bumpColor1 = g_WaveMap0.Sample( g_mirrorSampler, input.uv );
	float4 bumpColor2 = g_WaveMap1.Sample( g_mirrorSampler, input.uv );
	float3 normal1 = (bumpColor1.rgb * 2.0f) - 1.0f;
    float3 normal2 = (bumpColor2.rgb * 2.0f) - 1.0f;
	float3 bumpnormal = normalize(normal1);
	
	float2 reflectionCoords;
	float2 refractionCoords;
	
	// Set the reflection color
	reflectionCoords.x = input.reflectionMapSamplingPos.x / input.reflectionMapSamplingPos.w / 2.0f + 0.5f;
	reflectionCoords.y = -input.reflectionMapSamplingPos.y / input.reflectionMapSamplingPos.w / 2.0f + 0.5f;
	
	// Re-position the texture coordinate sampling position by the scaled normal map value to simulate the rippling wave effect.
	float2 reflectTexCoord = reflectionCoords + (bumpnormal.xy * 0.03f);
	float4 reflectiveColor = g_ReflectionMap.Sample( g_mirrorSampler, reflectTexCoord );
	
	// Set the refraction color
	refractionCoords.x = input.refractionMapSamplingPos.x / input.refractionMapSamplingPos.w / 2.0f + 0.5f;
	refractionCoords.y = -input.refractionMapSamplingPos.y / input.refractionMapSamplingPos.w / 2.0f + 0.5f;
	
	// Re-position the texture coordinate sampling position by the scaled normal map value to simulate the rippling wave effect.
	float2 refractTexCoord = refractionCoords + (bumpnormal.xy * 0.03f);
	float4 refractiveColor = g_RefractionMap.Sample( g_mirrorSampler, refractTexCoord );
	
	//
	//Add a water color tint to the refraction.
	//
	
    // Combine the tint with the refraction color.
	//float4(0.0f, 0.8f, 1.0f, 1.0f) is refraction tint
    refractiveColor = saturate(refractiveColor * float4(0.0f, 0.8f, 1.0f, 1.0f));
	
	// --------------- Fresnel term
	
	/*float3 eyeVector = normalize(g_LocalEye - input.position3D);
    float3 normalVector = float3(0,1,0);
    float fresnelTerm = dot(eyeVector, normalVector);    
    float4 final_color = lerp(reflectiveColor, refractiveColor, fresnelTerm);*/
	
	float3 eyeVector = normalize( g_LocalEye - input.position3D );
	float3 normalVector = float3(0,0,1);
	
	float fangle = 1 + dot(eyeVector, normalVector);
	
	fangle = pow(fangle, 5);
	
	float fresnelTerm = 1 / fangle;
	
	fresnelTerm = fresnelTerm * 3;
	fresnelTerm = fresnelTerm < 0 ? 0 : fresnelTerm;
	fresnelTerm = fresnelTerm > 1 ? 1 : fresnelTerm;
	
	float4 final_color = refractiveColor * (1 - fresnelTerm) + reflectiveColor * fresnelTerm * float4(water_color, 1);
	
	// --------------- Sun spots
	
	float cos_spec = clamp(dot(reflect_vec, g_SunDir), 0, 1);
	float sun_spot = pow(cos_spec, g_Shineness);
	final_color += float4(g_SunColor, 1) * sun_spot;
	
	return final_color;
}

float4 OceanSurfaceWireframePS(PixelInput input) : SV_Target
{
    return float4(0.9f, 0.9f, 0.9f, 1);
}