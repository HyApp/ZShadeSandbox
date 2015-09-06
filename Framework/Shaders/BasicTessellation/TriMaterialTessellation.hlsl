//==============================================================================
// Tri Material Tessellation Shader
//
//==============================================================================
// History
//
// -Created on 5/12/2015 by Dustin Watson
//==============================================================================
#include "../BasicTessellation/MaterialTessellationCommon.hlsl"
#include "../Basic/Matrix.hlsl"

//======================================================================================================

//
// Hull Shader
//

struct PatchConstOutput
{
	float edges[3]	: SV_TessFactor;
	float inside 	: SV_InsideTessFactor;
};

void HCTessellate(out PatchConstOutput output, float factor)
{
	output.edges[0] = factor;
	output.edges[1] = factor;
	output.edges[2] = factor;
	output.inside = factor;
}

void HCTessellate(out PatchConstOutput output, float3 e0, float3 e1, float3 e2, float3 c)
{
	output.edges[0] = CalcTessFactor(e0);
	output.edges[1] = CalcTessFactor(e1);
	output.edges[2] = CalcTessFactor(e2);
	output.inside = CalcTessFactor(c);
}

PatchConstOutput ConstHS(InputPatch<HullInput, 3> inputPatch, uint patchID : SV_PrimitiveID)
{
	PatchConstOutput output;

	if (g_EnableDistTess == 1)
	{
		float3 WorldPosition0 = inputPatch[0].position;
		float3 WorldPosition1 = inputPatch[1].position;
		float3 WorldPosition2 = inputPatch[2].position;

		// Compute midpoint on edges, and patch center
		float3 e0 = 0.5f * (WorldPosition0 + WorldPosition1);
		float3 e1 = 0.5f * (WorldPosition1 + WorldPosition2);
		float3 e2 = 0.5f * (WorldPosition2 + WorldPosition0);

		float3  c = (WorldPosition0 + WorldPosition1 + WorldPosition2) / 3;

		HCTessellate(output, e0, e1, e2, c);
	}
	else
	{
		HCTessellate(output, g_NoDistTessFactor);
	}

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstHS")]
DomainInput TriMaterialTessellationHS(InputPatch<HullInput, 3> inputPatch, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
	DomainInput output;

	output.position = inputPatch[pointID].position;

	output.normal = inputPatch[pointID].normal;

	output.uv = inputPatch[pointID].uv;
	
	output.instanceIndex = inputPatch[pointID].instanceIndex;
	
	output.instancePos = inputPatch[pointID].instancePos;
	output.instanceRot = inputPatch[pointID].instanceRot;
	output.instanceScl = inputPatch[pointID].instanceScl;
	
	return output;
}

//======================================================================================================

//
// Domain Shader
//

[domain("tri")]
PixelInput TriMaterialTessellationDS(PatchConstOutput input, float3 uvw : SV_DomainLocation, const OutputPatch<DomainInput, 3> inputPatch)
{
	PixelInput output;

	// Find the position of the new vertex
	float3 vPos = uvw.x * inputPatch[0].position + uvw.y * inputPatch[1].position + uvw.z * inputPatch[2].position;

	// Find the normal of the new vertex
	float3 vNormal = uvw.x * inputPatch[0].normal + uvw.y * inputPatch[1].normal + uvw.z * inputPatch[2].normal;

	// Find the texture coordinate of the new vertex
	float2 vTex = uvw.x * inputPatch[0].uv + uvw.y * inputPatch[1].uv + uvw.z * inputPatch[2].uv;
	
	// Find the instance position of the new vertex
	float3 vInstPos = uvw.x * inputPatch[0].instancePos + uvw.y * inputPatch[1].instancePos + uvw.z * inputPatch[2].instancePos;
	// Find the instance rotation of the new vertex
	float3 vInstRot = uvw.x * inputPatch[0].instanceRot + uvw.y * inputPatch[1].instanceRot + uvw.z * inputPatch[2].instanceRot;
	// Find the instance scaling of the new vertex
	float3 vInstScl = uvw.x * inputPatch[0].instanceScl + uvw.y * inputPatch[1].instanceScl + uvw.z * inputPatch[2].instanceScl;
	
	// The instance index does not change so just pass it along
	output.instanceIndex = inputPatch[0].instanceIndex;
	
	// Calculate the new position of the new vertex with WVP matrices
	
	if (g_UsingInstancing == 1)
	{
		if (g_UsingDisplacementMap == 1)
		{
			vInstPos.y = g_DisplacementMap.SampleLevel(g_PointSampler, vTex, 0).r;
		}
		
		vPos.x += vInstPos.x;
		vPos.y += vInstPos.y;
		vPos.z += vInstPos.z;
		
		// If the instance has a separate rotation and scale build the new world matrix
		float4x4 world = CreateWorldMatrix(vInstRot, vInstScl);
		
		output.position = mul(float4(vPos, 1.0), world);
	}
	else
	{
		if (g_UsingDisplacementMap == 1)
		{
			vPos.y = g_DisplacementMap.SampleLevel(g_PointSampler, vTex, 0).r;
		}
		
		output.position = mul(float4(vPos, 1.0f), g_World);
	}
	
	output.positionW = vPos;
	
	output.position = mul(output.position, g_View);
	output.position = mul(output.position, g_Proj);
	
	// Send the texture uv to the pixel shader
	output.uv = vTex;
	
	// Send the normal to the pixel shader
	output.normal = vNormal;
	
	output.clip = dot(output.position, g_ClipPlane);
	output.depth = output.position;
	
	// Logarithmic z buffer
	// DirectX Depth range 0..1
	int C = 1;
	output.pixelDepth = log2(max(1e-6, C * output.position.z + 1)) * 2 / log2(C * g_FarPlane + 1) * output.position.z;
	
	return output;
}

//======================================================================================================