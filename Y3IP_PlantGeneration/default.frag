#version 330 core

out vec4 FragColor;

in vec3 currentPos;
in vec3 normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
	//FragColor = vec4(color, 1.0f);
	FragColor = texture(diffuse0, texCoord);
}