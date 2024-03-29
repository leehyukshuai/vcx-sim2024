#version 410 core

layout(location = 0) in  vec3 v_Position;
layout(location = 1) in  vec3 v_Normal;
layout(location = 2) in  vec3 v_Color;

layout(location = 0) out vec4 f_Color;

struct Light {
    vec3  Intensity;
    vec3  Direction;
    vec3  Position;
    float CutOff;
    float OuterCutOff;
};

layout(std140) uniform PassConstants {
    mat4  u_Projection;
    mat4  u_View;
    vec3  u_ViewPosition;
    vec3  u_AmbientIntensity;
    Light u_Lights[4];
    int   u_CntPointLights;
    int   u_CntSpotLights;
    int   u_CntDirectionalLights;
};

uniform float u_AmbientScale;
uniform bool  u_UseBlinn;
uniform float u_Shininess;
uniform bool  u_UseGammaCorrection;
uniform int   u_AttenuationOrder;
// uniform float u_BumpMappingBlend; // Might not be needed if bump mapping is removed

// Removed uniform sampler2D declarations since textures are not used

vec3 Shade(vec3 lightIntensity, vec3 lightDir, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess) {
    vec3  diffuse        = max(dot(lightDir, normal), 0.0) * diffuseColor * lightIntensity;
    vec3  specular;
    if (u_UseBlinn) {
      specular = (shininess == 0 ? 1.: pow(max(dot(normal, normalize(lightDir + viewDir)), 0.0), shininess)) * specularColor * lightIntensity;
    } else {
      specular = (shininess == 0 ? 1.: pow(max(dot(reflect(-lightDir, normal), viewDir), 0.0), shininess)) * specularColor * lightIntensity;
    }
    return diffuse + specular;
}

vec3 GetNormal() {
    // Directly use the vertex normal since bump mapping and texture sampling are removed
    return normalize(v_Normal);
}

void main() {
    float gamma          = 2.2;
    // Removed texture sampling, use fixed colors or uniforms instead
    vec3 diffuseColor    = v_Color; // Assuming v_Color is used for diffuse color
    vec3 specularColor   = vec3(1.0); // Assuming a default specular color
    float shininess      = u_Shininess; // Use the provided shininess value
    vec3 normal          = GetNormal();
    vec3 viewDir         = normalize(u_ViewPosition - v_Position);
    // Ambient component.
    vec3 total = u_AmbientIntensity * u_AmbientScale * diffuseColor;
    // Iterate lights.
    for (int i = 0; i < u_CntPointLights; i++) {
        vec3 lightDir     = normalize(u_Lights[i].Position - v_Position);
        float dist        = length(u_Lights[i].Position - v_Position);
        float attenuation = 1. / (u_AttenuationOrder == 2 ? dist * dist : (u_AttenuationOrder == 1 ? dist : 1.));
        total += Shade(u_Lights[i].Intensity, lightDir, normal, viewDir, diffuseColor, specularColor, shininess) * attenuation;
    }
    for (int i = u_CntPointLights + u_CntSpotLights; i < u_CntPointLights + u_CntSpotLights + u_CntDirectionalLights; i++) {
        total += Shade(u_Lights[i].Intensity, u_Lights[i].Direction, normal, viewDir, diffuseColor, specularColor, shininess);
    }
    // Apply the vertex color to the final color
    vec3 finalColor = total;
    // Gamma correction.
    f_Color = vec4(u_UseGammaCorrection ? pow(finalColor, vec3(1. / gamma)) : finalColor, 1.);
}
