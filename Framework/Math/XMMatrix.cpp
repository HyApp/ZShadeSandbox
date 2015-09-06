#include "XMMatrix.h"
#include "XMMath3.h"
#include "XMMath4.h"
using ZShadeSandboxMath::XMMatrix;
//==============================================================================================================================
//==============================================================================================================================
XMMatrix::XMMatrix()
{
	ZeroOut();
}
//==============================================================================================================================
XMMatrix::XMMatrix(float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44)
{
	Update
	(
		_11, _12, _13, _14,
		_21, _22, _23, _24,
		_31, _32, _33, _34,
		_41, _42, _43, _44
	);
	
	UpdateFields();
}
//==============================================================================================================================
XMMatrix::XMMatrix(const XMMATRIX& mat)
{
	__matrix = mat;
}
//==============================================================================================================================
void XMMatrix::Update(int x, int y, float value)
{
	Update(((y * 15) + x), value);
}
//==============================================================================================================================
void XMMatrix::Update(int index, float value)
{
	_m[index] = value;

	// Need to rebuild the matrix data and update the fields
	
	Update(
		_m[0],  _m[1],  _m[2],  _m[3],
		_m[4],  _m[5],  _m[6],  _m[7],
		_m[8],  _m[9],  _m[10], _m[11],
		_m[12], _m[13], _m[14], _m[15]
	);
	
	UpdateFields();
}
//==============================================================================================================================
void XMMatrix::Update(float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44)
{
	XMVECTOR r0 = XMVectorSet(_11, _12, _13, _14);
	XMVECTOR r1 = XMVectorSet(_21, _22, _23, _24);
	XMVECTOR r2 = XMVectorSet(_31, _32, _33, _34);
	XMVECTOR r3 = XMVectorSet(_41, _42, _43, _44);
	
	__matrix.r[0] = r0;
	__matrix.r[1] = r1;
	__matrix.r[2] = r2;
	__matrix.r[3] = r3;
	
	UpdateArray();
}
//==============================================================================================================================
void XMMatrix::Assign(const XMMatrix& mat)
{
	__matrix = mat.Get();
	UpdateFields();
}
//==============================================================================================================================
void XMMatrix::Update()
{
	XMVECTOR r0 = XMVectorSet(_11, _12, _13, _14);
	XMVECTOR r1 = XMVectorSet(_21, _22, _23, _24);
	XMVECTOR r2 = XMVectorSet(_31, _32, _33, _34);
	XMVECTOR r3 = XMVectorSet(_41, _42, _43, _44);
	
	__matrix.r[0] = r0;
	__matrix.r[1] = r1;
	__matrix.r[2] = r2;
	__matrix.r[3] = r3;
}
//==============================================================================================================================
void XMMatrix::UpdateMatrixFromArray()
{
	XMVECTOR r0 = XMVectorSet( _m[0],  _m[1],  _m[2],  _m[3]);
	XMVECTOR r1 = XMVectorSet( _m[4],  _m[5],  _m[6],  _m[7]);
	XMVECTOR r2 = XMVectorSet( _m[8],  _m[9], _m[10], _m[11]);
	XMVECTOR r3 = XMVectorSet(_m[12], _m[13], _m[14], _m[15]);
	
	__matrix.r[0] = r0;
	__matrix.r[1] = r1;
	__matrix.r[2] = r2;
	__matrix.r[3] = r3;
}
//==============================================================================================================================
void XMMatrix::UpdateArray()
{
	_m[0] = _11;
	_m[1] = _12;
	_m[2] = _13;
	_m[3] = _14;
	
	_m[4] = _21;
	_m[5] = _22;
	_m[6] = _23;
	_m[7] = _24;
	
	_m[8] = _31;
	_m[9] = _32;
	_m[10] = _33;
	_m[11] = _34;
	
	_m[12] = _41;
	_m[13] = _42;
	_m[14] = _43;
	_m[15] = _44;
}
//==============================================================================================================================
void XMMatrix::ZeroOut()
{
	_11 = 0.0f;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;
	
	_21 = 0.0f;
	_22 = 0.0f;
	_23 = 0.0f;
	_24 = 0.0f;
	
	_31 = 0.0f;
	_32 = 0.0f;
	_33 = 0.0f;
	_34 = 0.0f;
	
	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 0.0f;
	
	XMVECTOR r0 = XMVectorSet(_11, _12, _13, _14);
	XMVECTOR r1 = XMVectorSet(_21, _22, _23, _24);
	XMVECTOR r2 = XMVectorSet(_31, _32, _33, _34);
	XMVECTOR r3 = XMVectorSet(_41, _42, _43, _44);
	
	__matrix.r[0] = r0;
	__matrix.r[1] = r1;
	__matrix.r[2] = r2;
	__matrix.r[3] = r3;
	
	UpdateArray();
}
//==============================================================================================================================
void XMMatrix::UpdateFields()
{
	XMFLOAT4 r0;
	XMFLOAT4 r1;
	XMFLOAT4 r2;
	XMFLOAT4 r3;
	
	XMStoreFloat4(&r0, __matrix.r[0]);
	XMStoreFloat4(&r1, __matrix.r[1]);
	XMStoreFloat4(&r2, __matrix.r[2]);
	XMStoreFloat4(&r3, __matrix.r[3]);
	
	_11 = r0.x;
	_12 = r0.y;
	_13 = r0.z;
	_14 = r0.w;
	
	_21 = r1.x;
	_22 = r1.y;
	_23 = r1.z;
	_24 = r1.w;
	
	_31 = r2.x;
	_32 = r2.y;
	_33 = r2.z;
	_34 = r2.w;
	
	_41 = r3.x;
	_42 = r3.y;
	_43 = r3.z;
	_44 = r3.w;
	
	UpdateArray();
}
//==============================================================================================================================
void XMMatrix::Decompose(XMFLOAT3& scale, XMFLOAT3& rotate, XMFLOAT3& translate)
{
	XMVECTOR scal; //for scaling
	XMVECTOR quat; //for rotation
	XMVECTOR tran; //for translation
	XMMatrixDecompose(&scal, &quat, &tran, __matrix);
	XMStoreFloat3(&translate, tran);
	XMStoreFloat3(&rotate, quat);
	XMStoreFloat3(&scale, scal);
}
//==============================================================================================================================
void XMMatrix::Decompose(ZShadeSandboxMath::XMMath3& scale, ZShadeSandboxMath::XMMath3& rotate, ZShadeSandboxMath::XMMath3& translate)
{
	XMFLOAT3 rScale = XMFLOAT3(scale.x, scale.y, scale.z);
	XMFLOAT3 rRotate = XMFLOAT3(rotate.x, rotate.y, rotate.z);
	XMFLOAT3 rTranslate = XMFLOAT3(translate.x, translate.y, translate.z);
	Decompose(rScale, rRotate, rTranslate);
	scale = rScale;
	rotate = rRotate;
	translate = rTranslate;
}
//==============================================================================================================================
void XMMatrix::Decompose(XMFLOAT4& scale, XMFLOAT4& rotate, XMFLOAT4& translate)
{
	XMVECTOR scal; //for scaling
	XMVECTOR quat; //for rotation
	XMVECTOR tran; //for translation
	XMMatrixDecompose(&scal, &quat, &tran, __matrix);
	XMStoreFloat4(&translate, tran);
	XMStoreFloat4(&rotate, quat);
	XMStoreFloat4(&scale, scal);
}
//==============================================================================================================================
void XMMatrix::Decompose(ZShadeSandboxMath::XMMath4& scale, ZShadeSandboxMath::XMMath4& rotate, ZShadeSandboxMath::XMMath4& translate)
{
	XMFLOAT4 rScale = XMFLOAT4(scale.x, scale.y, scale.z, scale.w);
	XMFLOAT4 rRotate = XMFLOAT4(rotate.x, rotate.y, rotate.z, rotate.w);
	XMFLOAT4 rTranslate = XMFLOAT4(translate.x, translate.y, translate.z, translate.w);
	Decompose(rScale, rRotate, rTranslate);
	scale = rScale;
	rotate = rRotate;
	translate = rTranslate;
}
//==============================================================================================================================
XMMatrix XMMatrix::Scale(float x, float y, float z)
{
	__matrix *= XMMatrixScaling(x, y, z);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Scale(XMFLOAT3 value)
{
	__matrix *= XMMatrixScaling(value.x, value.y, value.z);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Translate(float x, float y, float z)
{
	__matrix *= XMMatrixTranslation(x, y, z);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Translate(XMFLOAT3 value)
{
	__matrix *= XMMatrixTranslation(value.x, value.y, value.z);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Rotate(float x, float y, float z)
{
	XMMATRIX rotX, rotY, rotZ;
	rotX = XMMatrixRotationX(x);
	rotY = XMMatrixRotationY(y);
	rotZ = XMMatrixRotationZ(z);
	__matrix *= rotX * rotY * rotZ;
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Rotate(XMFLOAT3 value)
{
	XMMATRIX rotX, rotY, rotZ;
	rotX = XMMatrixRotationX(value.x);
	rotY = XMMatrixRotationY(value.y);
	rotZ = XMMatrixRotationZ(value.z);
	__matrix *= rotX * rotY * rotZ;
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::RotateX(float x)
{
	__matrix *= XMMatrixRotationX(x);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::RotateY(float y)
{
	__matrix *= XMMatrixRotationY(y);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::RotateZ(float z)
{
	__matrix *= XMMatrixRotationZ(z);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::RotationAxis(float x, float y, float z, float angle)
{
	XMFLOAT3 v = XMFLOAT3(x, y, z);
	__matrix = XMMatrixRotationAxis(XMLoadFloat3(&v), angle);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::RotationAxis(XMFLOAT3 value, float angle)
{
	__matrix = XMMatrixRotationAxis(XMLoadFloat3(&value), angle);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Transpose()
{
	__matrix = XMMatrixTranspose( __matrix );
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Inverse()
{
	__matrix = XMMatrixInverse( NULL, __matrix );
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix XMMatrix::Identity()
{
	__matrix = XMMatrixIdentity();
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
void XMMatrix::Multiply(const XMMatrix& mat)
{
	__matrix = XMMatrixMultiply(__matrix, (XMMATRIX)mat.Get());
	UpdateFields();
}
//==============================================================================================================================
void XMMatrix::Multiply(const XMMATRIX& mat)
{
	__matrix = XMMatrixMultiply(__matrix, (XMMATRIX)mat);
	UpdateFields();
}
//==============================================================================================================================
XMFLOAT4X4 XMMatrix::ToFloat4x4()
{
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, __matrix);
	return mat;
}
//==============================================================================================================================
void XMMatrix::LookAtLH(ZShadeSandboxMath::XMMath3 pos, ZShadeSandboxMath::XMMath3 target, ZShadeSandboxMath::XMMath3 up)
{
	XMVECTOR posV = pos.ToVector();
	XMVECTOR targetV = target.ToVector();
	XMVECTOR upV = up.ToVector();
	__matrix = XMMatrixLookAtLH(posV, targetV, upV);
	UpdateFields();
}
//==============================================================================================================================
XMMatrix XMMatrix::AffineTransform(ZShadeSandboxMath::XMMath3 scaling, ZShadeSandboxMath::XMMath3 rotation, ZShadeSandboxMath::XMMath3 rotQuat, ZShadeSandboxMath::XMMath3 translation)
{
	XMVECTOR scalingV = scaling.ToVector();
	XMVECTOR rotationV = rotation.ToVector();
	XMVECTOR rotQuatV = rotQuat.ToVector();
	XMVECTOR translationV = translation.ToVector();
	__matrix = XMMatrixAffineTransformation(scalingV, rotationV, rotQuatV, translationV);
	UpdateFields();
	return __matrix;
}
//==============================================================================================================================
XMMatrix& XMMatrix::operator * (const XMMatrix& mat)
{
	Multiply(mat);
	return *this;
}
//==============================================================================================================================
XMMatrix& XMMatrix::operator = (const XMMatrix& mat)
{
	Assign(mat);
	return *this;
}
//==============================================================================================================================
XMMatrix& XMMatrix::operator *= (const XMMatrix& mat)
{
	Multiply(mat);
	return *this;
}
//==============================================================================================================================
float& XMMatrix::operator [] (std::size_t idx)
{
	return _m[idx];
}
//==============================================================================================================================
const float& XMMatrix::operator [] (std::size_t idx) const
{
	return _m[idx];
}
//==============================================================================================================================
float& XMMatrix::operator () (std::size_t idx, std::size_t idy)
{
	return _m[((idy * 15) + idx)];
}
//==============================================================================================================================
const float& XMMatrix::operator () (std::size_t idx, std::size_t idy) const
{
	return _m[((idy * 15) + idx)];
}
//==============================================================================================================================
bool XMMatrix::operator == (const XMMatrix& mat)
{
	bool ret = true;
	
	for (uint32 i = 0; i < 4; i++)
	{
		for (uint32 j = 0; j < 4; j++)
		{
			if (__matrix.m[i][j] != mat.Get().m[i][j])
				ret = false;
		}
	}

	return ret;
}
//==============================================================================================================================