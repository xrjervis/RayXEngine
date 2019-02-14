#include "Engine/Renderer/d3d11/Light.hpp"
#include "Engine/Math/MathUtils.hpp"

Light::Light() {

}

Light::~Light() {

}

void Light::SetType(eLightType type) {
	m_data.lightType = type;
	RestoreDefaults();
}

void Light::SetRange(const Vector2& range) {
	m_data.range = range;
}

void Light::SetInnerAngle(float degrees) {
	m_data.innerAngle = degrees;
}

void Light::SetOuterAngle(float degrees) {
	m_data.outerAngle = degrees;
}

void Light::SetColor(const Rgba& color) {
	m_data.colorIntensity.x = color.GetFloatR();
	m_data.colorIntensity.y = color.GetFloatG();
	m_data.colorIntensity.z = color.GetFloatB();
}

void Light::SetIntensity(float intensity) {
	intensity = Clamp01(intensity);
	m_data.colorIntensity.w = intensity;
}

void Light::UpdateIfDirty() {
	if(m_transform.m_isDirty){
		m_data.position = m_transform.GetWorldPosition();
		m_data.direction = m_transform.GetForward();
		m_transform.m_isDirty = false;
	}
}

void Light::RestoreDefaults() {
	m_transform.SetWorldPosition(Vector3::ZERO);
	m_transform.SetWorldEulerAngles(Vector3::ZERO);
	m_transform.SetLocalScale(Vector3::ONE);

	m_data.colorIntensity = Vector4(1.f, 1.f, 1.f, 0.f);
}
