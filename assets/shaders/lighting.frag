#version 460 core

in vec2 uv;
out vec4 FragColor;

#include "common/lightStruct.glsl"


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 cameraPosition;

uniform PointLight pointLights[4];

void main(){
	
	vec3 PositionMap = texture(gPosition, uv).rgb;
    vec3 normalMap = texture(gNormal, uv).rgb;
    vec3 diffuseMap = texture(gAlbedoSpec, uv).rgb;
    float specularMap = texture(gAlbedoSpec, uv).a;

    // ambient
    vec3 result = diffuseMap * 0.1;
	vec3 viewDir = normalize(cameraPosition - PositionMap);

    for(int i = 0; i < 4; i++){
        
        // diffuse
        vec3 lightDir = normalize(pointLights[i].position - PositionMap);
        vec3 diffuse = max(dot(normalMap, lightDir), 0.0) * diffuseMap * pointLights[i].color;

        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normalMap, halfwayDir), 0.0), 4.0);
        vec3 specular = pointLights[i].color * spec * specularMap;

        // attenuation
        float dist = length(pointLights[i].position - PositionMap);
        float attenuation = 1.0 / (0.44 * dist * dist + 0.35 * dist + 1);
        
        diffuse *= attenuation;
        specular *= attenuation;
        result += diffuse + specular;
    }

    FragColor = vec4(result, 1.0);

}