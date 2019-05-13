#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Rgba.hpp"

constexpr size_t MAX_LIGHTS = 8;

enum eLightType : int {
	LIGHT_TYPE_POINT = 0,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_DIRECTIONAL
};

struct LightData_t {
	int lightType;													//4	
	Vector3 position;												//16
	Vector4 colorIntensity = Vector4(1.f, 1.f, 1.f, 0.f);			//32
	Vector2 range;													//40
	float innerAngle;												//44
	float outerAngle;												//48
	Vector3 direction;												//60
	float padding;													//64
};

class Light {
public:
	Light();
	~Light();

	void SetType(eLightType type);
	void SetRange(const Vector2& range);
	void SetInnerAngle(float degrees);
	void SetOuterAngle(float degrees);
	void SetColor(const Rgba& color);
	void SetIntensity(float intensity);
	void UpdateIfDirty();

	void RestoreDefaults();

public:
	eLightType m_type;
	LightData_t m_data;
	Transform m_transform;
};