#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
//layout (location = 2) out vec4 gAlbedoSpec;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(IN.worldPos, 0);
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(IN.normal);
    // and the diffuse per-fragment color, ignore specular
    //gAlbedoSpec.rgb = vec3(0.95);
}  