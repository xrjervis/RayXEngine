#include "Engine/Core/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/d3d11/ResourceView.hpp"

Camera::Camera()
	: m_projMode(PERSPECTIVE)
	, m_fov(50.f) {
}

Camera::~Camera() {

}

void Camera::SetProjectionMode(eProjectionMode mode) {
	m_projMode = mode;
	if (mode == PERSPECTIVE) {
		//	SetPerspective(50.f, g_clientAspect, 0.1f, 100.f);
	}

	//#TODO(RX):add Orthographic default value
}

eProjectionMode Camera::GetProjectionMode() const {
	return m_projMode;
}

void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::Up*/) {
	Matrix44 viewMat = Matrix44::LookAtLH(position, target, up);
	SetView(viewMat);
	Matrix44 modelMat = viewMat.Inverse();
	m_transform.SetLocalMatrix(modelMat);
	m_data.eyePosition = modelMat.GetTranslation();
}

void Camera::SetProjection(const Matrix44& proj) {
	m_data.projection = proj;
}

void Camera::SetView(const Matrix44& view) {
	m_data.view = view;
}

void Camera::UpdateIfDirty() {
	if (m_transform.m_isDirty) {
		SetView(m_transform.GetWorldMatrix().Inverse());
		m_data.eyePosition = m_transform.GetWorldPosition();
		m_transform.ResetDirtyFlag();
	}
}

void Camera::SetRenderTarget(RenderTargetView* rtv) {
	m_renderTarget = rtv;
}

void Camera::SetDepthTarget(DepthStencilView* dsv) {
	m_depthTarget = dsv;
}

void Camera::SetViewport(u32 idx, int topLeftX, int topLeftY, float width, float height, float minDepth /*= 0.f*/, float maxDepth /*= 1.f*/) {
	m_viewports[idx] = std::make_unique<Viewport>();
	D3D11_VIEWPORT& v = m_viewports[idx]->m_d3d11Viewport;
	v.Width = width;
	v.Height = height;
	v.MinDepth = minDepth;
	v.MaxDepth = maxDepth;
	v.TopLeftX = (float)topLeftX;
	v.TopLeftY = (float)topLeftY;
}

// center is (0, 0)
void Camera::SetOrtho(float height, float aspect, float nearVal /*= 0.f*/, float farVal /*= 1.f*/) {
	float halfWidth = (height * aspect) / 2.f;
	float halfHeight = height / 2.f;
	m_ortho = AABB2(Vector2(-halfWidth, -halfHeight), Vector2(halfWidth, halfHeight));
	Matrix44 projMat = Matrix44::MakeOrthoLH(height * aspect, height, nearVal, farVal);
	SetProjection(projMat);
}

void Camera::SetOrtho(const Vector2& bottomLeft, const Vector2 topRight, float nearVal /*= 0.f*/, float farVal /*= 1.f*/) {
	m_ortho = AABB2(bottomLeft, topRight);
	Matrix44 projMat = Matrix44::MakeOrthoLH(bottomLeft, topRight, nearVal, farVal);
	SetProjection(projMat);
}

void Camera::SetPerspective(float fovDegrees, float aspect, float nearZ /*= 0.f*/, float farZ /*= 1.f*/) {
	m_fov = fovDegrees;
	Matrix44 projMat = Matrix44::MakePerspectiveLH(fovDegrees, aspect, nearZ, farZ);
	SetProjection(projMat);

	float top = nearZ * TanDegrees(m_fov / 2.f);
	float right = top * aspect;

	m_ortho = AABB2(Vector2::ZERO, right, top);
}
