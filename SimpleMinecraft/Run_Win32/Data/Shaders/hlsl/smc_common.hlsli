Texture2D g_texDiffuse : register(t0);
SamplerState g_texDiffuseSampler : register(s0);

struct VS_INPUT {
	float4 position : POSITION;
	float4 color : COLOR;
	float2 texCoords : TEXCOORD;
};

struct PS_INPUT {
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 color : COLOR;
	float2 texCoords : TEXCOORD;
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix g_view;
	row_major matrix g_projection;
	float3 g_eyePos;
	float padding00;
}

cbuffer ObjectBuffer : register(b1)
{
	row_major matrix g_model;
	row_major matrix g_inverseTransposeModel;
}

#define MAX_LIGHTS 8
struct LightData_t {
	int lightType;					//4	
	float3 position;				//16
	float4 colorIntensity;			//32
	float2 range;					//40
	float innerAngle;				//44
	float outerAngle;				//48
	float3 direction;				//60
	float padding;					//64
};

cbuffer LightBuffer : register(b2)
{
	LightData_t g_lights[MAX_LIGHTS];
}

// in hlsl bool is 4 bytes; but in cpp bool is 1 byte
cbuffer MaterialBuffer : register(b3)
{
	bool useDiffuse;
	bool useNormal;
	bool useSpecular;
	bool useLighting;
}

cbuffer GlobalBuffer : register(b4)
{
	float3 g_skyColor;
	float g_fogStart;
	float3 g_defaultIndoorLightColor;
	float g_fogEnd;
    float3 g_defaultOutdoorLightColor;
}