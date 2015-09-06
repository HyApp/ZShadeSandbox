//==============================================================================
// Texture Shader
//
//==============================================================================
// History
//
// -Created in 2013 by Dustin Watson
//==============================================================================

//
// Constants
//

cbuffer cbPerFrame : register(b0)
{
	float4 g_ClipPlane;
	float2 padding;
	int	   g_FlipTextureH;
	int	   g_FlipTextureV;
};

cbuffer cbPerObject : register(b1)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
};

//======================================================================================================

//
// Textures and samplers
//

Texture2D g_ShaderTexture : register(t0);

SamplerState g_LinearSampler : register(s0);

//======================================================================================================

//
// Structs
//

struct VertexInput
{
    float4 position : POSITION;
    float2 uv		: TEXCOORD0;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv		: TEXCOORD0;
	float clip		: TEXCOORD1;
};

//======================================================================================================

//
// Vertex Shader
//

PixelInput TextureVS(VertexInput input)
{
    PixelInput output;
    
	input.position.w = 1.0f;
    
    output.position = mul(input.position, g_World);
    output.position = mul(output.position, g_View);
    output.position = mul(output.position, g_Proj);

    output.uv = input.uv;
    
	output.clip = dot(output.position, g_ClipPlane);

    return output;
}

//======================================================================================================

//
// Pixel Shader
//

float4 TexturePS(PixelInput input) : SV_Target
{
    float4 textureColor;
    
	// 1 - u: flips the texture horizontally
	if (g_FlipTextureH == 1)
		input.uv.x = 1 - input.uv.x;
	
	// 1 - v: flips the texture vertically
	if (g_FlipTextureV == 1)
		input.uv.y = 1 - input.uv.y;
	
    //Sample the pixel color from the texture using the sampler at this texture
    //coordinate location
    textureColor = g_ShaderTexture.Sample(g_LinearSampler, input.uv);
    
	clip( input.clip < 0.0 ? -1 : 1 );

	return textureColor;
}

float4 TextureWireframePS(PixelInput input) : SV_Target
{
	clip( input.clip < 0.0 ? -1 : 1 );
	
    return float4(0.9f, 0.9f, 0.9f, 1);
}
