#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

//Row major matrix class
class Matrix44 {
public:
	~Matrix44() = default;
	Matrix44() = default;// default-construct to Identity matrix (via variable initialization)
	explicit Matrix44(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
	explicit Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation = Vector2::ZERO);
	explicit Matrix44(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation = Vector3::ZERO);


	// Accessors
	Vector3			GetUpVector() const;
	Vector3			GetRightVector() const;
	Vector3			GetForwardVector()const;
	Vector3			GetTranslation() const;
	Vector3			GetEulerAngles();
	Vector3			GetScale();

	// Mutators
	void			SetIdentity();
	void			SetValues(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
	void			SetValues(const Vector3& I, const Vector3& J, const Vector3& K);
	void			SetValues(const Vector3& I, const Vector3& J, const Vector3& K, const Vector3& T);
	void			SetTranslation(const Vector3& translation);
	void			SetUpVector(const Vector3& vec3);

	Matrix44		Append(const Matrix44& matrixToAppend); // a.k.a. Concatenate (right-multiply)
	Matrix44		Inverse();
	void			Transpose();
	void			RotateEulerAngle(const Vector3& rotation);
	void			Translate(const Vector3& translation);
	void			Scale(const Vector3& scale);
	void			NormalizeByScale(const Vector3& scale);

	// Operators
	Matrix44		operator*(const Matrix44& rightMat);
	void			operator*=(const Matrix44& rightMat);
	friend Vector4  operator*(const Vector4& vec4, const Matrix44& rightMat);

	// Producers
	static Matrix44 MakeYawRotationDegrees(float h);
	static Matrix44 MakePitchRotationDegrees(float p);
	static Matrix44 MakeRollRotationDegrees(float b);
	static Matrix44 MakeTranslation(const Vector3& translation);
	static Matrix44 MakeRotationFromEuler(const Vector3& euler);
	static Matrix44 MakeScale(const Vector3& scale);
	static Matrix44 MakeOrthoLH(const Vector2& bottomLeft, const Vector2& topRight, float near = 0.f, float far = 1.f);
	//(0, 0) is the client center
	static Matrix44 MakeOrthoLH(float width, float height, float near = 0.f, float far = 1.f);
	static Matrix44 MakePerspectiveLH(float fov, float aspect, float nearZ = 0.1f, float farZ = 1000.f);
	static Matrix44 LookAtLH(const Vector3& camPos, const Vector3& targetPos, const Vector3& worldUp = Vector3::Up);
	static Matrix44 MakeReflection(const Vector3& planeNormal, const Vector3& position);
	static Matrix44 MakeShadowMatrix(const Vector3& planeNomral, const Vector3& position, const Vector4& lightPosition);

public:
	float Ix = 1, Iy = 0, Iz = 0, Iw = 0;
	float Jx = 0, Jy = 1, Jz = 0, Jw = 0;
	float Kx = 0, Ky = 0, Kz = 1, Kw = 0;
	float Tx = 0, Ty = 0, Tz = 0, Tw = 1;

	static Matrix44 Mat_GameToEngine;
	static Matrix44 Mat_Game;
};

Matrix44			Lerp(const Matrix44& a, const Matrix44& b, float t);
