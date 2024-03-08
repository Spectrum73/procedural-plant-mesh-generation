#version 330 core

out vec4 FragColor;

in vec3 Normal;

void main()
{
	// diffuse lighting
	vec3 normal = normalize(Normal);

	FragColor = vec4(normal/2 + 0.5f, 0);
}