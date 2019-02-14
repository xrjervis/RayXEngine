#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector4.hpp"

void Transform::SetParent(Transform* parent) {
	if (m_parent) {
		m_parent->RemoveChild(this);
	}
	if (parent) {
		parent->AddChild(this);
	}
	m_parent = parent;
	SetDirty();
}

void Transform::AddChild(Transform* child) {
	m_children.push_back(child);
	child->m_isDirty = true;
}

void Transform::RemoveChild(Transform* child) {
	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i] == child) {
			m_children[i] = m_children[m_children.size() - 1];
			m_children.pop_back();
			break;
		}
	}
}

void Transform::RemoveChildren() {
	for (auto child : m_children) {
		child->SetParent(nullptr);
	}
	m_children.clear();
}

void Transform::SetDirty() {
	m_isDirty = true;
	for (auto child : m_children) {
		child->SetDirty();
	}
}

Matrix44 Transform::CalculateLocalToParentMatrix() const {
	// SRT
	return Matrix44::MakeScale(m_localScale) *
		Matrix44::MakeRotationFromEuler(m_localEulerAngles) *
		Matrix44::MakeTranslation(m_localPosition);
}

Matrix44 Transform::GetWorldMatrix() {
	if (m_isDirty) {
		if (m_parent == nullptr) {
			m_worldMatrix = CalculateLocalToParentMatrix();
		}
		else {
			m_worldMatrix = CalculateLocalToParentMatrix() * m_parent->GetWorldMatrix();
		}
		m_isDirty = false;
	}
	return m_worldMatrix;
}

Matrix44 Transform::GetLocalMatrix() {
	if (m_isDirty) {
		m_localMatrix = CalculateLocalToParentMatrix();
		m_isDirty = false;
	}
	return m_localMatrix;
}

Vector3 Transform::GetWorldPosition() {
	auto pos = GetWorldMatrix().GetTranslation();
	return pos;
}

Vector3 Transform::GetWorldEulerAngles() {
	if (m_parent == nullptr) {
		return m_localEulerAngles;
	}
	else {
		auto vec4 = Vector4(m_localEulerAngles, 1.f) * m_parent->GetWorldMatrix();
		return vec4.xyz();
	}
}

Vector3 Transform::GetLocalPosition() const {
	return m_localPosition;
}

Vector3 Transform::GetLocalEulerAngles() const {
	return m_localEulerAngles;
}

Vector3 Transform::GetLocalScale() const {
	return m_localScale;
}

Vector3 Transform::GetForward() {
	Matrix44 mat44 = GetWorldMatrix();
	return mat44.GetForwardVector();
}

Vector3 Transform::GetRight() {
	Matrix44 mat44 = GetWorldMatrix();
	return mat44.GetRightVector();
}

Vector3 Transform::GetUp() {
	Matrix44 mat44 = GetWorldMatrix();
	return mat44.GetUpVector();
}

// Do a matrix decomposition
void Transform::SetLocalMatrix(const Matrix44& mat44) {
	SetDirty();
	auto localMat = mat44;
	m_localPosition = localMat.GetTranslation();
	m_localScale = localMat.GetScale();
	localMat.NormalizeByScale(m_localScale);
	m_localEulerAngles = localMat.GetEulerAngles();
}

void Transform::SetLocalPositioin(const Vector3& pos) {
	SetDirty();
	m_localPosition = pos;
}

void Transform::SetLocalEulerAngles(const Vector3& euler) {
	SetDirty();
	auto vec3 = euler;
	vec3.x = ClampFloat(vec3.x, -90.f, 90.f);
	vec3.y = fmodf(vec3.y, 360.f);
	vec3.z = fmodf(vec3.z, 360.f);
	m_localEulerAngles = vec3;
}

void Transform::SetLocalScale(const Vector3& scale) {
	SetDirty();
	m_localScale = scale;
}

void Transform::SetWorldMatrix(const Matrix44& mat44) {
	SetDirty();
	m_worldMatrix = mat44;

	if (m_parent == nullptr) {
		SetLocalMatrix(mat44);
	}
	else {
		auto parentWorldMatrix = m_parent->GetWorldMatrix();
		auto localMatrix = mat44;
		auto result = localMatrix * parentWorldMatrix.Inverse();
		SetLocalMatrix(result);
	}
}

void Transform::SetWorldPosition(const Vector3& pos) {
	SetDirty();
	if (m_parent == nullptr) {
		m_localPosition = pos;
	}
	else {
		auto vec4 = Vector4(pos, 1.f) * m_worldMatrix.Inverse();
		m_localPosition = vec4.xyz();
	}
}

void Transform::SetWorldEulerAngles(const Vector3& euler) {
	SetDirty();
	if (m_parent == nullptr) {
		m_localEulerAngles = euler;
	}
	else {
		auto vec4 = Vector4(euler, 1.f) * m_worldMatrix.Inverse();
		m_localEulerAngles = vec4.xyz();
	}
}

void Transform::Translate(const Vector3& disp) {
	SetDirty();
	SetLocalPositioin(m_localPosition + disp);
}

void Transform::Rotate(const Vector3& euler) {
	SetDirty();
	SetLocalEulerAngles(m_localEulerAngles + euler);
}

void Transform::Scale(const Vector3& scale) {
	SetDirty();
	SetLocalScale(m_localScale + scale);
}

void Transform::LookAt(const Vector3& targetPos, const Vector3& up /*= Vector3::Up*/) {
	auto lookat = Matrix44::LookAtLH(GetWorldPosition(), targetPos, up);
	SetWorldMatrix(lookat.Inverse());
}

void Transform::ResetDirtyFlag() {
	m_isDirty = false;
	for (auto child : m_children) {
		child->ResetDirtyFlag();
	}
}
