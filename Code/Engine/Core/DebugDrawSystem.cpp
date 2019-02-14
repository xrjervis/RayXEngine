#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/FontRenderer.hpp"

Uptr<DebugDrawSystem>	g_theDebugDraw;

DebugDrawOptions_t::DebugDrawOptions_t()
	: m_startColor(Rgba::WHITE)
	, m_endColor(Rgba::WHITE)
	, m_lifetime(0.0f)
	, m_mode(DEBUG_RENDER_USE_DEPTH) {}

DebugDrawOptions_t::DebugDrawOptions_t(float liftTime, const Rgba& startColor, const Rgba& endColor, eDebugDrawMode mode)
	: m_startColor(startColor)
	, m_endColor(endColor)
	, m_lifetime(liftTime)
	, m_mode(mode) {}

DebugObject::DebugObject(const DebugDrawOptions_t& options)
	: m_options(options) {}

DebugObject::~DebugObject() {}

void DebugObject::Update(float deltaSeconds) {
	m_age += deltaSeconds;
	m_color = Interpolate(m_options.m_startColor, m_options.m_endColor, GetNormalizedAge());
}

bool DebugObject::IsDead() const {
	return m_age >= m_options.m_lifetime;
}

float DebugObject::GetNormalizedAge() const {
	return Clamp01(m_age / m_options.m_lifetime);
}


DebugLine3D::DebugLine3D(const Vector3& startPos, const Vector3& endPos, const DebugDrawOptions_t& options)
	: DebugObject(options)
	, m_startPos(startPos)
	, m_endPos(endPos) {}

DebugLine3D::~DebugLine3D() {

}

void DebugLine3D::Render() const {
	g_theRHI->GetImmediateRenderer()->DrawLine3D(m_startPos, m_endPos, m_color);
}

DebugDrawSystem::DebugDrawSystem() {

}

DebugDrawSystem::~DebugDrawSystem() {

}

void DebugDrawSystem::SetCamera2D(Camera* cam) {
	m_camera2D = cam;
}

void DebugDrawSystem::SetCamera3D(Camera* cam) {
	m_camera3D = cam;
}

void DebugDrawSystem::Update(float deltaSeconds) {
	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* obj2d = m_debugObjects2D[i].get();
		if (obj2d) {
			if (obj2d->IsDead()) {
				m_debugObjects2D[i].reset();
			}
			else {
				obj2d->Update(deltaSeconds);
			}
		}
	}

	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* obj3d = m_debugObjects3D[i].get();
		if (obj3d) {
			if (obj3d->IsDead()) {
				m_debugObjects3D[i].reset();
			}
			else {
				obj3d->Update(deltaSeconds);
			}
		}
	}
}

void DebugDrawSystem::Render2D() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->BindCamera(m_camera2D);
	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* obj2d = m_debugObjects2D[i].get();
		if (obj2d) {
			obj2d->Render();
		}
	}
}

void DebugDrawSystem::Render3D() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->BindCamera(m_camera3D);
	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* obj3d = m_debugObjects3D[i].get();
		if (obj3d) {
			obj3d->Render();
		}
	}
}

void DebugDrawSystem::AddDebugObject2D(Uptr<DebugObject>&& obj2d) {
	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* objSlot = m_debugObjects2D[i].get();
		if (!objSlot) {
			m_debugObjects2D[i] = std::move(obj2d);
			return;
		}
	}
}

void DebugDrawSystem::AddDebugObject3D(Uptr<DebugObject>&& obj3d) {
	for (size_t i = 0; i < MAX_DEBUG_OBJECTS; ++i) {
		DebugObject* objSlot = m_debugObjects3D[i].get();
		if (!objSlot) {
			m_debugObjects3D[i] = std::move(obj3d);
			return;
		}
	}
}

void DebugDrawBase(float lifeTime, const Vector3& originPos, const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, float axisLength /*= 1.f*/) {
	DebugDrawOptions_t options_x(lifeTime, Rgba::RED, Rgba::RED);
	Uptr<DebugObject> axis_x = std::make_unique<DebugLine3D>(originPos, originPos + xAxis * axisLength, options_x);

	DebugDrawOptions_t options_y(lifeTime, Rgba::GREEN, Rgba::GREEN);
	Uptr<DebugObject> axis_y = std::make_unique<DebugLine3D>(originPos, originPos + yAxis * axisLength, options_y);

	DebugDrawOptions_t options_z(lifeTime, Rgba::BLUE, Rgba::BLUE);
	Uptr<DebugObject> axis_z = std::make_unique<DebugLine3D>(originPos, originPos + zAxis * axisLength, options_z);

	g_theDebugDraw->AddDebugObject3D(std::move(axis_x));
	g_theDebugDraw->AddDebugObject3D(std::move(axis_y));
	g_theDebugDraw->AddDebugObject3D(std::move(axis_z));
}
