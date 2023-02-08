#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;
in vec3 normal; //New Attribute!
in vec4 tangent;
in vec2 texCoord;

out Vertex {
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} OUT;

void main ( void ) {
	OUT.texCoord = texCoord;

	//mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));

	vec3 wNormal = normalize( normalMatrix * normalize( normal ));
	vec3 wTangent = normalize( normalMatrix * normalize( tangent.xyz ));

	//OUT.normal = wNormal;
	//OUT.normal = normal;
	OUT.normal = normalMatrix * normal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	vec4 worldPos = (viewMatrix * modelMatrix * vec4(position, 1.0));

	OUT.worldPos = worldPos.xyz;

	//gl_Position = (projMatrix * viewMatrix) * worldPos;
	gl_Position = projMatrix * worldPos;
}