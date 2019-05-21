#define MAX_LIGHTS 8

Texture2D g_texDiffuse : register(t0);
SamplerState g_texDiffuseSampler : register(s0);

struct VS_INPUT {
    float4 position : POSITION;
    float4 color : COLOR;
    float2 texCoords : TEXCOORD;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoords : TEXCOORD;
};

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

cbuffer CameraBuffer : register(b0) {
	row_major matrix g_view;  
    row_major matrix g_projection; 
	float3 g_eyePos;
	float padding00;
}

cbuffer ObjectBuffer : register(b1) {
    row_major matrix g_model;
    row_major matrix g_inverseTransposeModel;
}

cbuffer LightBuffer : register(b2) {
    LightData_t g_lights[MAX_LIGHTS];
}

// in hlsl bool is 4 bytes; but in cpp bool is 1 byte
cbuffer MaterialBuffer : register(b3){
    bool useDiffuse;
    bool useNormal;
    bool useSpecular;
    bool useLighting;
}

cbuffer GlobalBuffer : register(b4){
    float g_fogStart;
	float g_fogRange;
    float2 padding;
	float4 g_fogColor;
	float4 g_ambient;
}

struct LightResult_t {
	float3 diffuse;
	float3 specular;
};

float3 DoDiffuse(LightData_t light, float3 lightDir, float3 normal){
    float NdotL = max(0.f, dot(normal, lightDir));
    return light.colorIntensity.xyz * NdotL;
}

float3 DoSpecular(LightData_t light, float3 eyeDir, float3 lightDir, float3 normal){
    // Phong lighting
    float3 reflection = normalize(reflect(-lightDir, normal));
    float RdotV = max(0.f, dot(reflection, eyeDir));

    // Blinn-Phong lighting
    float3 halfVec = normalize(lightDir + eyeDir);
    float NdotH = max(0.f, dot(normal, halfVec));

    return light.colorIntensity.xyz * pow(RdotV, 32.f);
}

float DoAttenuation(LightData_t light, float distance){
    return 1.f - smoothstep(light.range.x, light.range.y, distance);
}

float DoAngleAttenuation(LightData_t light, float cosAngle){
    float minCos = cos(radians(light.innerAngle));
    float maxCos = cos(radians(light.outerAngle));
    return 1.f - smoothstep(minCos, maxCos, cosAngle);
}

LightResult_t DoDirectionalLight(LightData_t light, float3 eyeDir, float3 normal){
    LightResult_t result;
    float3 lightDir = -light.direction;

    result.diffuse = DoDiffuse(light, lightDir, normal) * light.colorIntensity.w;
    result.specular = DoSpecular(light, eyeDir, lightDir, normal) * light.colorIntensity.w;

    return result;
}

LightResult_t DoPointLight(LightData_t light, float3 eyeDir, float3 pos, float3 normal){
    LightResult_t result;
    float3 lightDir = light.position - pos;
    float distance = length(lightDir);
    lightDir = lightDir / distance;

    float attenuation = DoAttenuation(light, distance);
    
    result.diffuse = DoDiffuse(light, lightDir, normal) * attenuation * light.colorIntensity.w;
    result.specular = DoSpecular(light, eyeDir, lightDir, normal) * attenuation * light.colorIntensity.w;

    return result;
}

LightResult_t DoSpotLight(LightData_t light, float3 eyeDir, float3 pos, float3 normal){
    LightResult_t result;

    float3 lightDir = light.position - pos;
    float distance = length(lightDir);
    lightDir = lightDir / distance;

    float attenuation = DoAttenuation(light, distance);

    float cosAngle = dot(light.direction , -lightDir);
    float angleAttenuation = DoAngleAttenuation(light, cosAngle);

    result.diffuse = DoDiffuse(light, lightDir, normal) * attenuation * angleAttenuation * light.colorIntensity.w;
    result.specular = DoSpecular(light, eyeDir, lightDir, normal) * attenuation * angleAttenuation * light.colorIntensity.w;

    return result;
}

LightResult_t ComputeLighting(float3 pos, float3 normal) {
    float3 eyeDir = normalize(g_eyePos - pos);

    LightResult_t finalResult = { {0,0,0}, {0,0,0} };
    
    [unroll]
    for(int i = 0; i < MAX_LIGHTS; ++i){
        LightResult_t result = { {0,0,0}, {0,0,0} };

        if(g_lights[i].colorIntensity.w == 0.f) continue;

        // point light
        if(g_lights[i].lightType == 0){
            result = DoPointLight(g_lights[i], eyeDir, pos, normal);
        }
        // spot light
        else if(g_lights[i].lightType == 1){
            result = DoSpotLight(g_lights[i], eyeDir, pos, normal);
        }
        // directional light
        else if(g_lights[i].lightType == 2){
            result = DoDirectionalLight(g_lights[i], eyeDir, normal);
        }

        finalResult.diffuse += result.diffuse;
        finalResult.specular += result.specular;
    }

    finalResult.diffuse = saturate(finalResult.diffuse);
    finalResult.specular = saturate(finalResult.specular);

    return finalResult;
}
