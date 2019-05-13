#version 420 core
// Debug Functions
vec3 WorldNormalToColor( vec3 normal )
{
   return (normal + vec3(1.0f)) * .5f; 
}

// return [0.f, 1.f]
float CalAttenuation(float dist, float inner, float outer){
    if(dist < inner){
        return 1.f;
    }
    else if (inner <= dist && dist <= outer){
        float t = (dist - inner) / (outer - inner);
        float a = mix(0.f, 1.f, t);
        float b = mix(0.f, a, t);
        float c = mix(a, 1.f, t);
        float d = 1 - mix(b, c, t);
        return d;
    }
    else{
        return 0.f;
    }
}

#define MAX_LIGHTS (8)
struct LightData_t{
    vec3 position;		    //12
	float pad00;			//16
	vec4 colorIntensity;	//32
	vec2 range;			    //40
    float dotInnerAngle;
	float dotOuterAngle;	//48
    vec3 direction;
    float directionFactor;
};

// Uniforms ==============================================
layout(binding=1, std140) uniform uboCamera{
    mat4 VIEW;
    mat4 PROJECTION;
    vec3 CAMERA_POSITION;
    float pad00;
};

layout(binding=2, std140) uniform uboLight{
    vec4 ambience;
    LightData_t lights[MAX_LIGHTS];
};

layout(std140) uniform MaterialPropertyBlock{
    float _specularStrength;
	float _specularPower;
	vec2 _pad01;
};

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal;

// Attributes ============================================
in vec2 passUV; 
in vec4 passColor; 
in vec3 passWorldPos;   // new
in mat3 TBN;

out vec4 outColor; 

// Entry Point ===========================================
void main( void )
{
    vec4 texColor = texture( gTexDiffuse, passUV);
    vec4 surfaceColor = texColor * passColor;
    vec3 surfaceNormal = texture(gTexNormal, passUV).xyz;
    surfaceNormal = normalize(surfaceNormal * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) );
    vec3 worldNormal = surfaceNormal * transpose(TBN);

    // # debug mode = WORLD NORMAL
    //outColor = vec4(WorldNormalToColor(worldNormal), 1.f);
    //return;

    //outColor = vec4(passUV.xy, 0.f, 1.f);
    //return;

    // # debug mode = SURFACE NORMAL
    //outColor = vec4(WorldNormalToColor(surfaceNormal), 1.f);
    //return;

    // # debug mode = COLOR ONLY
    //outColor = surfaceColor;
    //return;

    // # debug mode = TANGENT
    //outColor = vec4(WorldNormalToColor(TBN[0]), 1.f);
    //return;

    // # debug mode = BITANGENT
    //outColor = vec4(WorldNormalToColor(TBN[1]), 1.f);
    //return;

    vec3 diffuse = ambience.xyz * ambience.w; // ambience is the base value
    vec3 specular = vec3(0.0f); 

    vec3 eyeDir = normalize(CAMERA_POSITION - passWorldPos);

    for (int light_idx = 0; light_idx < MAX_LIGHTS; ++light_idx) {
        float lightIntensity = lights[light_idx].colorIntensity.w;
        vec3 lightColor = lights[light_idx].colorIntensity.xyz;
        if(lightIntensity == 0.f){
            continue;
        }
        
        vec3 lightForward = normalize(lights[light_idx].direction);
        vec3 lightDir = lights[light_idx].position - passWorldPos;
        float lightDist = length(lightDir);
        lightDir /= lightDist;

        // figure out how far away angle-wise I am from the forward of the light (useful for spot lights)
        float dotAngle = dot( lightForward, -lightDir ); 
        float angleAttenuation = smoothstep(lights[light_idx].dotOuterAngle, lights[light_idx].dotInnerAngle, dotAngle);
        lightIntensity = lightIntensity * angleAttenuation;
        // get actual direction light is pointing (spotlights point in their "forward", point lights point everywhere (ie, toward the point))
        lightDir = mix(lightDir, -lightForward, lights[light_idx].directionFactor);
        
        float attentuation = CalAttenuation(lightDist, lights[light_idx].range.x, lights[light_idx].range.y);
        float specAttenuation = CalAttenuation(lightDist, lights[light_idx].range.x, lights[light_idx].range.y);
        
        float dot3 = dot(lightDir, worldNormal);
        float diffuseFactor = clamp(attentuation * dot3, 0.0f, 1.0f);
    
        // specular
        vec3 r = reflect(-lightDir, worldNormal);
        float specAmount = max(dot(r, eyeDir), 0.f);
        float specIntensity = (specAttenuation * _specularStrength) * pow(specAmount, _specularPower);
        diffuse += lightColor * diffuseFactor;
        specular += lightColor * specIntensity;
    }

    diffuse = clamp( diffuse, vec3(0), vec3(1) );      
    vec4 finalColor = vec4(diffuse, 1.0f) * surfaceColor;
    //final_color = max( final_color, vec4(emissive.xyz, 0) ); 
    outColor = clamp(finalColor, vec4(0), vec4(1));
    
}