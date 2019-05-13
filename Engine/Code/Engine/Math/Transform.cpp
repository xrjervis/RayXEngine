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


Transform* Transform::GetParent() const {
	return m_parent;
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

Matrix44 Transform::GetWorldMatrix() const{
	if (m_parent == nullptr) {
		return GetLocalMatrix();
	}
	else {
		return GetLocalMatrix() * m_parent->GetWorldMatrix();
	}
}

Matrix44 Transform::GetLocalMatrix() const{
	return Matrix44::MakeScale(m_localScale) *
		Matrix44::MakeRotationFromEuler(m_localEulerAngles) *
		Matrix44::MakeTranslation(m_localPosition);
}

Vector3 Transform::GetWorldPosition() const{
	return GetWorldMatrix().GetTranslation();
}

Vector3 Transform::GetWorldEulerAngles() const{
	return GetWorldMatrix().GetEulerAngles();
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

Vector3 Transform::GetForward() const{
	return GetWorldMatrix().GetForwardVector();
}

Vector3 Transform::GetRight() const{
	return GetWorldMatrix().GetRightVector();
}

Vector3 Transform::GetUp() const{
	return GetWorldMatrix().GetUpVector();
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
		auto worldMat = GetWorldMatrix();
		auto vec4 = Vector4(pos, 1.f) * worldMat.Inverse();
		m_localPosition = vec4.xyz();
	}
}

void Transform::SetWorldEulerAngles(const Vector3& euler) {
	SetDirty();
	if (m_parent == nullptr) {
		m_localEulerAngles = euler;
	}
	else {
		auto worldMat = GetWorldMatrix();
		auto vec4 = Vector4(euler, 1.f) * worldMat.Inverse();
		m_localEulerAngles = vec4.xyz();
	}
}


void Transform::SetWorldScale(const Vector3& scale) {
	SetDirty();
	if (m_parent == nullptr) {
		m_localScale = scale;
	}
	else {
		auto worldMat = GetWorldMatrix();
		auto vec4 = Vector4(scale, 1.f) * worldMat.Inverse();
		m_localScale = vec4.xyz();
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

void Transform::LookAt(const Vector3& pos, const Vector3& targetPos, const Vector3& up /*= Vector3::Up*/) {
	auto lookat = Matrix44::LookAtLH(pos, targetPos, up);
	SetWorldMatrix(lookat.Inverse());
}

void Transform::ResetDirtyFlag() {
	m_isDirty = false;
	for (auto child : m_children) {
		child->ResetDirtyFlag();
	}
}
