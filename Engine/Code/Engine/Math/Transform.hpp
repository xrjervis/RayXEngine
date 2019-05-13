#pragma once
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vector3.hpp"
#include <vector>

class Transform {
public:
	Transform() = default;
	~Transform() = default;

	void SetParent(Transform* parent);
	Transform* GetParent() const;

	Matrix44 GetWorldMatrix() const;					// Matrix that transform a point from local space into world space
	Matrix44 GetLocalMatrix() const;					// Matrix that transform a point from world space into local space
	Vector3 GetWorldPosition() const;					// The position of the transform in world space
	Vector3 GetWorldEulerAngles() const;				// The rotation as Euler angles in degrees
	Vector3 GetLocalPosition() const;					// Position of the transform relative to the parent transform
	Vector3 GetLocalEulerAngles() const;				// The rotation as Euler angles in degrees relative to the parent transform's rotation
	Vector3 GetLocalScale() const;						// The scale of the transform relative to the parent
	Vector3 GetForward() const;							// The blue axis of the transform in world space
	Vector3 GetRight() const;							// The red axis of the transform in world space
	Vector3 GetUp() const;								// The green axis of the transform in world space

	void SetLocalMatrix(const Matrix44& mat44);
	void SetLocalPositioin(const Vector3& pos);
	void SetLocalEulerAngles(const Vector3& euler);
	void SetLocalScale(const Vector3& scale);
	void SetWorldMatrix(const Matrix44& mat44);
	void SetWorldPosition(const Vector3& pos);
	void SetWorldEulerAngles(const Vector3& euler);
	void SetWorldScale(const Vector3& scale);

	void Translate(const Vector3& disp);		// Moves the transform in the direction and distance of translation.
	void Rotate(const Vector3& euler);			// Applies a rotation of eulerAngles.z degrees around the z axis, eulerAngles.x degrees around the x axis, and eulerAngles.y degrees around the y axis (in that order).
	void Scale(const Vector3& scale);
	void LookAt(const Vector3& pos, const Vector3& targetPos, const Vector3& up = Vector3::Up);
	void ResetDirtyFlag();

private:
	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
	void RemoveChildren();
	void SetDirty();

public:
	mutable bool m_isDirty = true;

private:
	Vector3					m_localPosition = Vector3::ZERO;
	Vector3					m_localEulerAngles = Vector3::ZERO;
	Vector3					m_localScale = Vector3::ONE;
	Transform*				m_parent = nullptr;
	std::vector<Transform*> m_children;
};