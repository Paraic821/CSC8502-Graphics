# version 330 core
uniform sampler2D diffuseTex;

in Vertex {
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main ( void ) {
	fragColour = texture(diffuseTex, IN.texCoord);
}