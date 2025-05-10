#version 460 core

in vec2 uv;
in vec3 normal;
in vec3 worldPosition;

out vec3 gPosition;
out vec3 gNormal;
out vec4 gAlbedoSpec;

uniform sampler2D diffuse;
uniform sampler2D specular;

void main()
{    
    
    gPosition = worldPosition;
    
    gNormal = normalize(normal);
  
    gAlbedoSpec.rgb = texture(diffuse, uv).rgb;

    gAlbedoSpec.a = texture(specular, uv).r;
}