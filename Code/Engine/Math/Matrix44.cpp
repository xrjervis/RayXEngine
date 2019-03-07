#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/MathUtils.hpp"

Matrix44 Matrix44::GameToEngine = Matrix44();
Matrix44 Matrix44::EngineToGame = Matrix44();

Matrix44::Matrix44(const float* sixteenValuesBasisMajor) {
	SetValues(sixteenValuesBasisMajor);
}

Matrix44::Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation /*= Vector2(0.f, 0.f)*/)
	: Ix(iBasis.x)
	, Iy(iBasis.y)
	, Jx(jBasis.x)
	, Jy(jBasis.y)
	, Tx(translation.x)
	, Ty(translation.y) {}

Matrix44::Matrix44(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation /*= Vector3::ZERO*/) {
	SetValues(iBasis, jBasis, kBasis, translation);
}

Vector3 Matrix44::GetUpVector() const {
	return Vector3(Jx, Jy, Jz).GetNormalized();
}

Vector3 Matrix44::GetRightVector() const {
	return Vector3(Ix, Iy, Iz).GetNormalized();
}

Vector3 Matrix44::GetForwardVector() const {
	return Vector3(Kx, Ky, Kz).GetNormalized();
}

Vector3 Matrix44::GetTranslation() const {
	return Vector3(Tx, Ty, Tz);
}

Vector3 Matrix44::GetEulerAngles() {
	float h, p, b;
	float sp = -Ky;
	p = AsinDegrees(sp);
	p = ClampFloat(p, -90.f, 90.f);

	if (Abs(sp) > 0.99999f) {
		// We are looking straight up or down.
		// Slam bank to zero and just set heading.
		b = 0.f;
		h = Atan2Degrees(-Iz, Ix);
	}
	else {
		h = Atan2Degrees(Kx, Kz);
		b = Atan2Degrees(Iy, Jy);
	}
	return Vector3(p, h, b);
}

Vector3 Matrix44::GetScale() {
	auto sx = Vector3(Ix, Iy, Iz).GetLength();
	auto sy = Vector3(Jx, Jy, Jz).GetLength();
	auto sz = Vector3(Kx, Ky, Kz).GetLength();
	auto scale = Vector3(sx, sy, sz);
	return scale;
}

void Matrix44::SetIdentity() {
	float values[16] = { 1.f, 0.f, 0.f, 0.f,
						 0.f, 1.f, 0.f, 0.f,
						 0.f, 0.f, 1.f, 0.f,
						 0.f, 0.f, 0.f, 1.f };
	SetValues(values);
}

void Matrix44::SetValues(const float* sixteenValuesBasisMajor) {
	memcpy(this, sixteenValuesBasisMajor, sizeof(Matrix44));
}

void Matrix44::SetValues(const Vector3& I, const Vector3& J, const Vector3& K) {
	Ix = I.x; Iy = I.y; Iz = I.z;
	Jx = J.x; Jy = J.y; Jz = J.z;
	Kx = K.x; Ky = K.y; Kz = K.z;
}

void Matrix44::SetValues(const Vector3& I, const Vector3& J, const Vector3& K, const Vector3& T) {
	Ix = I.x; Iy = I.y; Iz = I.z;
	Jx = J.x; Jy = J.y; Jz = J.z;
	Kx = K.x; Ky = K.y; Kz = K.z;
	Tx = T.x; Ty = T.y; Tz = T.z;
}

void Matrix44::SetTranslation(const Vector3& translation) {
	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
}

void Matrix44::SetUpVector(const Vector3& vec3) {
	Jx = vec3.x;
	Jy = vec3.y;
	Jz = vec3.z;
}

Matrix44 Matrix44::Append(const Matrix44& matrixToAppend) {
	float newValues[16];

	newValues[0] = Ix * matrixToAppend.Ix + Iy * matrixToAppend.Jx + Iz * matrixToAppend.Kx + Iw * matrixToAppend.Tx;
	newValues[1] = Ix * matrixToAppend.Iy + Iy * matrixToAppend.Jy + Iz * matrixToAppend.Ky + Iw * matrixToAppend.Ty;
	newValues[2] = Ix * matrixToAppend.Iz + Iy * matrixToAppend.Jz + Iz * matrixToAppend.Kz + Iw * matrixToAppend.Tz;
	newValues[3] = Ix * matrixToAppend.Iw + Iy * matrixToAppend.Jw + Iz * matrixToAppend.Kw + Iw * matrixToAppend.Tw;

	newValues[4] = Jx * matrixToAppend.Ix + Jy * matrixToAppend.Jx + Jz * matrixToAppend.Kx + Jw * matrixToAppend.Tx;
	newValues[5] = Jx * matrixToAppend.Iy + Jy * matrixToAppend.Jy + Jz * matrixToAppend.Ky + Jw * matrixToAppend.Ty;
	newValues[6] = Jx * matrixToAppend.Iz + Jy * matrixToAppend.Jz + Jz * matrixToAppend.Kz + Jw * matrixToAppend.Tz;
	newValues[7] = Jx * matrixToAppend.Iw + Jy * matrixToAppend.Jw + Jz * matrixToAppend.Kw + Jw * matrixToAppend.Tw;

	newValues[8] = Kx * matrixToAppend.Ix + Ky * matrixToAppend.Jx + Kz * matrixToAppend.Kx + Kw * matrixToAppend.Tx;
	newValues[9] = Kx * matrixToAppend.Iy + Ky * matrixToAppend.Jy + Kz * matrixToAppend.Ky + Kw * matrixToAppend.Ty;
	newValues[10] = Kx * matrixToAppend.Iz + Ky * matrixToAppend.Jz + Kz * matrixToAppend.Kz + Kw * matrixToAppend.Tz;
	newValues[11] = Kx * matrixToAppend.Iw + Ky * matrixToAppend.Jw + Kz * matrixToAppend.Kw + Kw * matrixToAppend.Tw;

	newValues[12] = Tx * matrixToAppend.Ix + Ty * matrixToAppend.Jx + Tz * matrixToAppend.Kx + Tw * matrixToAppend.Tx;
	newValues[13] = Tx * matrixToAppend.Iy + Ty * matrixToAppend.Jy + Tz * matrixToAppend.Ky + Tw * matrixToAppend.Ty;
	newValues[14] = Tx * matrixToAppend.Iz + Ty * matrixToAppend.Jz + Tz * matrixToAppend.Kz + Tw * matrixToAppend.Tz;
	newValues[15] = Tx * matrixToAppend.Iw + Ty * matrixToAppend.Jw + Tz * matrixToAppend.Kw + Tw * matrixToAppend.Tw;

	SetValues(newValues);

	return *this;
}


Matrix44 Matrix44::Inverse() {
	float inv[16], det, m[16], invOut[16];
	memcpy(m, this, sizeof(Matrix44));

	inv[0] =
		m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] =
		-m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] =
		m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] =
		-m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] =
		-m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] =
		m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] =
		-m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] =
		m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] =
		m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] =
		-m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] =
		m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] =
		-m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] =
		-m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] =
		m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] =
		-m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] =
		m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0) {
		return *this;
	}

	det = 1.0f / det;

	for (int i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;
	memcpy(this, invOut, sizeof(Matrix44));
	return *this;
}

void Matrix44::Transpose() {
	std::swap(Jx, Iy);
	std::swap(Kx, Iz);
	std::swap(Ky, Jz);
	std::swap(Tx, Iw);
	std::swap(Ty, Jw);
	std::swap(Ty, Kw);
}

void Matrix44::RotateEulerAngle(const Vector3& rotation) {
	this->Append(MakeRotationFromEuler(rotation));
}

void Matrix44::Translate(const Vector3& translation) {
	this->Append(MakeTranslation(translation));
}

void Matrix44::Scale(const Vector3& scale) {
	this->Append(MakeScale(scale));
}

void Matrix44::NormalizeByScale(const Vector3& scale) {
	Ix /= scale.x; Iy /= scale.x; Iz /= scale.x;
	Jx /= scale.y; Jy /= scale.y; Jz /= scale.y;
	Kx /= scale.z; Ky /= scale.z; Kz /= scale.z;
}

Matrix44 Matrix44::operator*(const Matrix44& rightMat) {
	return this->Append(rightMat);
}

void Matrix44::operator*=(const Matrix44& rightMat) {
	*this = this->Append(rightMat);
}

Vector4 operator*(const Vector4& vec4, const Matrix44& rightMat) {
	float x = vec4.x * rightMat.Ix + vec4.y * rightMat.Jx + vec4.z * rightMat.Kx + vec4.w * rightMat.Tx;
	float y = vec4.x * rightMat.Iy + vec4.y * rightMat.Jy + vec4.z * rightMat.Ky + vec4.w * rightMat.Ty;
	float z = vec4.x * rightMat.Iz + vec4.y * rightMat.Jz + vec4.z * rightMat.Kz + vec4.w * rightMat.Tz;
	float w = vec4.x * rightMat.Iw + vec4.y * rightMat.Jw + vec4.z * rightMat.Kw + vec4.w * rightMat.Tw;

	return Vector4(x, y, z, w);
}

Matrix44 Lerp(const Matrix44& a, const Matrix44& b, float t) {
	Vector3 a_right = a.GetRightVector();
	Vector3 b_right = b.GetRightVector();
	Vector3 a_up = a.GetUpVector();
	Vector3 b_up = b.GetUpVector();
	Vector3 a_forward = a.GetForwardVector();
	Vector3 b_forward = b.GetForwardVector();
	Vector3 a_translation = a.GetTranslation();
	Vector3 b_translation = b.GetTranslation();

	Vector3 newRight = Slerp(a_right, b_right, t);
	Vector3 newUp = Slerp(a_up, b_up, t);
	Vector3 newForward = Slerp(a_forward, b_forward, t);
	Vector3 newTranslation = Interpolate(a_translation, b_translation, t);

	return Matrix44(newRight, newUp, newForward, newTranslation);
}

Matrix44 Matrix44::MakeYawRotationDegrees(float h) {
	Matrix44 mat44;
	mat44.Ix = CosDegrees(h);
	mat44.Kx = SinDegrees(h);
	mat44.Iz = -SinDegrees(h);
	mat44.Kz = CosDegrees(h);
	return mat44;
}

Matrix44 Matrix44::MakePitchRotationDegrees(float p) {
	Matrix44 mat44;
	mat44.Jy = CosDegrees(p);
	mat44.Ky = -SinDegrees(p);
	mat44.Jz = SinDegrees(p);
	mat44.Kz = CosDegrees(p);
	return mat44;
}

Matrix44 Matrix44::MakeRollRotationDegrees(float b) {
	Matrix44 mat44;
	mat44.Ix = CosDegrees(b);
	mat44.Jx = -SinDegrees(b);
	mat44.Iy = SinDegrees(b);
	mat44.Jy = CosDegrees(b);
	return mat44;
}

Matrix44 Matrix44::MakeTranslation(const Vector3& translation) {
	Matrix44 mat44;
	mat44.Tx = translation.x;
	mat44.Ty = translation.y;
	mat44.Tz = translation.z;
	return mat44;
}

Matrix44 Matrix44::MakeRotationFromEuler(const Vector3& euler) {
	// B-banking P-pitch H-heading
	auto B = MakeRollRotationDegrees(euler.z);
	auto P = MakePitchRotationDegrees(euler.x);
	auto H = MakeYawRotationDegrees(euler.y);
	return B * P * H;
}

Matrix44 Matrix44::MakeScale(const Vector3& scale) {
	Matrix44 mat44;
	mat44.Ix = scale.x;
	mat44.Jy = scale.y;
	mat44.Kz = scale.z;
	return mat44;
}

Matrix44 Matrix44::MakeOrthoLH(const Vector2& bottomLeft, const Vector2& topRight, float near, float far) {
	float width = topRight.x - bottomLeft.x;
	float height = topRight.y - bottomLeft.y;
	float distance = far - near;
	float tx = -(topRight.x + bottomLeft.x) / width;
	float ty = -(topRight.y + bottomLeft.y) / height;
	float tz = near / -distance;

	const float values[] = {
		2.f / width, 0.f, 0.f, 0.f,
		0.f, 2.f / height, 0.f, 0.f,
		0.f, 0.f, 1.f / distance, 0.f,
		tx, ty, tz, 1.f
	};

	return Matrix44(values);
}

Matrix44 Matrix44::MakeOrthoLH(float width, float height, float near /*= 0.f*/, float far /*= 1.f*/) {
	if (width == 0.f || height == 0.f || far == near) {
		return Matrix44();
	}

	const float values[] = {
		2.f / width, 0.f, 0.f, 0.f,
		0.f, 2.f / height, 0.f, 0.f,
		0.f, 0.f, 1.f / (far - near), 0.f,
		0.f, 0.f, near / (near - far), 1.f
	};
	return Matrix44(values);
}

// fov : [0.01f, 179.99f]
// somehow nearZ cannot be 0.f
Matrix44 Matrix44::MakePerspectiveLH(float fov, float aspect, float nearZ, float farZ) {
	if (nearZ == 0.f) {
		nearZ = 0.1f;
	}
	if (fov > 179.99f) {
		fov = 179.99f;
	}
	if (fov < 0.01f) {
		fov = 0.01f;
	}
	float d = 1.0f / TanDegrees(fov / 2.f);
	float q = 1.0f / (farZ - nearZ);

	const float values[] = {
		d / aspect, 0.f, 0.f, 0.f,
		0.f, d, 0.f, 0.f,
		0.f, 0.f, (nearZ + farZ) * q, 1.f,
		0.f, 0.f, -2.f * nearZ * farZ * q, 0.f
	};
	return Matrix44(values);
}

Matrix44 Matrix44::LookAtLH(const Vector3& camPos, const Vector3& targetPos, const Vector3& worldUp /*= Vector3::Up*/) {
	Vector3 dir = (targetPos - camPos).GetNormalized();
	Vector3 forward = dir;
	Vector3 right = CrossProduct(worldUp, forward).GetNormalized();
	Vector3 up = CrossProduct(forward, right);

	float tx = -DotProduct(right, camPos);
	float ty = -DotProduct(up, camPos);
	float tz = -DotProduct(forward, camPos);

	const float values[] = {
		right.x, up.x, forward.x, 0.f,
		right.y, up.y, forward.y, 0.f,
		right.z, up.z, forward.z, 0.f,
		tx, ty, tz, 1.f
	};

	return Matrix44(values);
}

Matrix44 Matrix44::MakeReflection(const Vector3& planeNormal, const Vector3& position) {
	float d = -DotProduct(planeNormal, position);
	float a = planeNormal.x;
	float b = planeNormal.y;
	float c = planeNormal.z;

	const float values[] = {
		1 - 2 * a*a, -2 * a*b, -2 * a*c, 0.f,
		-2 * a*b, 1 - 2 * b*b, -2 * b*c, 0.f,
		-2 * a*c, -2 * b*c, 1 - 2 * c*c, 0.f,
		-2 * a*d, -2 * b*d, -2 * c*d, 1.f
	};

	return Matrix44(values);
}

Matrix44 Matrix44::MakeShadowMatrix(const Vector3& planeNomral, const Vector3& position, const Vector4& lightPosition) {
	Vector3 N = planeNomral;
	Vector3 L = lightPosition.xyz();
	float Lw = lightPosition.w; // Lw = 0: Directional Light; Lw = 1: point Light
	float d = -DotProduct(N, position);
	float NdotL = DotProduct(N, L);

	const float values[] = {
		NdotL + d * Lw - L.x*N.x, -L.y*N.x, -L.z*N.x, -Lw * N.x,
		-L.x*N.y, NdotL + d * Lw - L.y*N.y, -L.z*N.y, -Lw * N.y,
		-L.x*N.z, -L.y*N.z, NdotL + d * Lw - L.z*N.z, -Lw * N.z,
		-L.x*d, -L.y*d, -L.z*d, NdotL
	};

	return Matrix44(values);
}

