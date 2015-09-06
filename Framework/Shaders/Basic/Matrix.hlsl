//==============================================================================
// Matrix Shader Helper
//
//==============================================================================
// History
//
// -Created on 8/25/2015 by Dustin Watson
//==============================================================================

// Default Identity Matrix
float4x4 IdentityMatrix()
{
	float4x4 identity =
	{
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	};

	return identity;
}

// Calculates the rotation matrix based on the position of rotation for the object
float4x4 RotationMatrix(float3 rot)
{
	float4x4 rotateX = {
		1.0, 0.0, 0.0, 0.0,
		0.0, cos(rot.x), -sin(rot.x), 0.0,
		0.0, sin(rot.x), cos(rot.x), 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	float4x4 rotateY = {
		cos(rot.y), 0.0, sin(rot.y), 0.0,
		0.0, 1.0, 0.0, 0.0,
		-sin(rot.y), 0.0, cos(rot.y), 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	float4x4 rotateZ = {
		cos(rot.z), -sin(rot.z), 0.0, 0.0,
		sin(rot.z), cos(rot.z), 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	return mul(rotateX, mul(rotateY, rotateZ));
}

// Calculates the translation matrix based on the position of the object
float4x4 TranslationMatrix(float3 pos)
{
	float4x4 translate = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		pos.x, pos.y, pos.z, 1.0
	};
	
	return translate;
}

// Calculates the scale matrix based on the scale of the object
float4x4 ScaleMatrix(float3 scl)
{
	float4x4 scale = {
		scl.x, 0.0, 0.0, 0.0,
		0.0, scl.y, 0.0, 0.0,
		0, 0.0, scl.z, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	
	return scale;
}

float4x4 CreateWorldMatrix(float3 pos, float3 rot, float3 scl)
{
	float4x4 world = IdentityMatrix();
	world = mul(world, ScaleMatrix(scl));
	world = mul(world, RotationMatrix(rot));
	world = mul(world, TranslationMatrix(pos));
	return transpose(world);
}

float4x4 CreateWorldMatrix(float3 rot, float3 scl)
{
	float4x4 world = IdentityMatrix();
	world = mul(world, ScaleMatrix(scl));
	world = mul(world, RotationMatrix(rot));
	return transpose(world);
}

/*float4x4 CreateWorldMatrix(float3 rot)
{
	float4x4 world = IdentityMatrix();
	
	world = mul(world, ScaleMatrix(float3(1, 1, 1)));
	world = mul(world, RotationMatrix(rot));
	
	return transpose(world);
}

float4x4 CreateWorldMatrix(float3 scl)
{
	float4x4 world = IdentityMatrix();
	
	world = mul(world, ScaleMatrix(scl));
	
	return transpose(world);
}*/