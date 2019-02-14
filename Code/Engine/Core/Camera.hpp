#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/d3d11/Viewport.hpp"
#include <memory>

class RenderTargetView;
class DepthStencilView;

enum eProjectionMode {
	ORTHOGRAPHIC,
	PERSPECTIVE
};

struct CameraBuffer_t {
	Matrix44 view;
	Matrix44 projection;
	Vector3 eyePosition;
	float padding;
};

class Camera {
public:
	Camera();
	virtual ~Camera();

	void					SetProjectionMode(eProjectionMode mode);
	eProjectionMode			GetProjectionMode() const;

	void					LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Up);
	void					SetOrtho(float height, float aspect, float nearZ = 0.f, float farZ = 1.f);
	void					SetOrtho(const Vector2& bottomLeft, const Vector2 topRight, float nearZ = 0.f, float farZ = 1.f);
	void					SetPerspective(float fovDegrees, float aspect, float nearZ = 0.f, float farZ = 1.f);
	void					SetView(const Matrix44& view);
	AABB2					GetOrtho() const { return m_ortho; }

	void					UpdateIfDirty();
	void					SetRenderTarget(RenderTargetView* rtv);
	void					SetDepthTarget(DepthStencilView* dsv);

	void					SetViewport(u32 idx, int topLeftX, int topLeftY, float width, float height, float minDepth = 0.f, float maxDepth = 1.f);

	RenderTargetView*		GetRTV() const { return m_renderTarget; }
	DepthStencilView*		GetDSV() const { return m_depthTarget; }

private:
	void					SetProjection(const Matrix44& proj);

public:
	Transform				m_transform;
	CameraBuffer_t			m_data;

	RenderTargetView*		m_renderTarget = nullptr;
	DepthStencilView*		m_depthTarget = nullptr;

	std::unique_ptr<Viewport> m_viewports[VIEWPORT_SLOT_COUNT];

private:
	eProjectionMode			m_projMode;
	AABB2					m_ortho;
	float					m_fov;
};