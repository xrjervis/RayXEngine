#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/FontRenderer.hpp"

Uptr<DebugDrawSystem>	g_theDebugDrawSystem;

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

DebugLine3D::DebugLine3D(const Vector3& startPos, const Vector3& endPos, const DebugDrawOptions_t& options)
	: DebugObject(options)
	, m_startPos(startPos)
	, m_endPos(endPos) {}

DebugLine3D::~DebugLine3D() {

}

void DebugLine3D::Render() const {
	g_theRHI->GetImmediateRenderer()->DrawLine3D(m_startPos, m_endPos, m_color);
}


DebugLine::DebugLine(const Vector2& startPos, const Vector2& endPos, const DebugDrawOptions_t& options) 
	: DebugObject(options)
	, m_startPos(startPos)
	, m_endPos(endPos) {

}

DebugLine::~DebugLine() {

}

void DebugLine::Render() const {
	g_theRHI->GetImmediateRenderer()->DrawLine2D(m_startPos, m_endPos, m_color);

}

DebugCube3D::DebugCube3D(const Vector3& center, const Vector3& size, const DebugDrawOptions_t& options) 
	: DebugObject(options)
	, m_center(center)
	, m_size(size) {}

DebugCube3D::~DebugCube3D() {

}

void DebugCube3D::Render() const {
	g_theRHI->GetImmediateRenderer()->DrawCube(m_center, m_size, m_color);
}

DebugText2D::DebugText2D(const std::string& text, const DebugDrawOptions_t& options)
	: DebugObject(options)
	, m_text(text) {

}

DebugText2D::~DebugText2D() {

}


DebugDrawSystem::DebugDrawSystem() {

}

DebugDrawSystem::~DebugDrawSystem() {

}

void DebugDrawSystem::SetCamera2D(Camera* cam) {
	m_camera2D = cam;
	m_debugStringBounds = m_camera2D->GetOrtho();
	m_debugStringBounds.mins.x += 0.05f * m_camera2D->GetOrtho().GetDimensions().x;
	m_debugStringBounds.maxs.y -= 0.051f * m_camera2D->GetOrtho().GetDimensions().y;

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

	if (!m_debugText2DQueue.empty()) {
		for (auto it = m_debugText2DQueue.begin(); it != m_debugText2DQueue.end(); ) {
			if ((*it)->IsDead()) {
				it = m_debugText2DQueue.erase(it);
			}
			else {
				(*it)->Update(deltaSeconds);
				++it;
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

	g_theRHI->GetFontRenderer()->SetSize(m_debugStringSize);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_TOP);
	for (auto it = m_debugText2DQueue.begin(); it != m_debugText2DQueue.end(); ++it) {
		AABB2 bound = m_debugStringBounds;
		bound.maxs.y -= m_debugStringSize * (it - m_debugText2DQueue.begin());
		g_theRHI->GetFontRenderer()->DrawTextInBox((*it)->m_text, bound, (*it)->m_color);

	}
}

void DebugDrawSystem::Render3D() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("debug"));
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

void DebugDrawSystem::AddDebugText2D(Uptr<DebugText2D>&& text2D) {
	m_debugText2DQueue.push_back(std::move(text2D));
	if (m_debugText2DQueue.size() > m_numDebugStringInQueue) {
		m_debugText2DQueue.pop_front();
	}
}

void DebugDrawBase(float lifeTime, const Vector3& originPos, const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, float axisLength /*= 1.f*/) {
	DebugDrawOptions_t options_x(lifeTime, Rgba::RED, Rgba::RED);
	Uptr<DebugObject> axis_x = std::make_unique<DebugLine3D>(originPos, originPos + xAxis * axisLength, options_x);

	DebugDrawOptions_t options_y(lifeTime, Rgba::GREEN, Rgba::GREEN);
	Uptr<DebugObject> axis_y = std::make_unique<DebugLine3D>(originPos, originPos + yAxis * axisLength, options_y);

	DebugDrawOptions_t options_z(lifeTime, Rgba::BLUE, Rgba::BLUE);
	Uptr<DebugObject> axis_z = std::make_unique<DebugLine3D>(originPos, originPos + zAxis * axisLength, options_z);

	g_theDebugDrawSystem->AddDebugObject3D(std::move(axis_x));
	g_theDebugDrawSystem->AddDebugObject3D(std::move(axis_y));
	g_theDebugDrawSystem->AddDebugObject3D(std::move(axis_z));
}

void DebugString(float lifeTime, const std::string& text, const Rgba& startColor, const Rgba& endColor) {
	DebugDrawOptions_t options(lifeTime, startColor, endColor);
	Uptr<DebugText2D> text2D = std::make_unique<DebugText2D>(text, options);
	g_theDebugDrawSystem->AddDebugText2D(std::move(text2D));
}

void DebugDrawLine(float lifeTime, const Vector2& startPos, const Vector2& endPos, const Rgba& startColor, const Rgba& endColor) {
	DebugDrawOptions_t options(lifeTime, startColor, endColor);
	Uptr<DebugObject> line = std::make_unique<DebugLine>(startPos, endPos, options);
	g_theDebugDrawSystem->AddDebugObject2D(std::move(line));
}

void DebugDrawLine3D(float lifeTime, const Vector3& startPos, const Vector3& endPos, const Rgba& startColor, const Rgba& endColor) {
	DebugDrawOptions_t options(lifeTime, startColor, endColor);
	Uptr<DebugObject> line = std::make_unique<DebugLine3D>(startPos, endPos, options);
	g_theDebugDrawSystem->AddDebugObject3D(std::move(line));
}

void DebugDrawCube(float lifeTime, const Vector3& center, const Vector3& size, const Rgba& startColor, const Rgba& endColor) {
	DebugDrawOptions_t options(lifeTime, startColor, endColor);
	Uptr<DebugObject> cube = std::make_unique<DebugCube3D>(center, size, options);
	g_theDebugDrawSystem->AddDebugObject3D(std::move(cube));
}
