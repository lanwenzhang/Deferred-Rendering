#version 460 core

out vec4 FragColor;

in vec2 uv;
in vec3 normal;


uniform float near;
uniform float far;


void main()
{


	float Zndc = gl_FragCoord.z * 2.0 - 1.0;

	float linearDepth = 2 * near / (far + near - Zndc*(far - near));

	vec3 finalColor = vec3(linearDepth, linearDepth, linearDepth);

	FragColor = vec4(finalColor, 1.0);

}