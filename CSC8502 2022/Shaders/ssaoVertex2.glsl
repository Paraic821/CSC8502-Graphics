#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;

void main(void) {
	gl_Position = vec4(position, 1.0);
}