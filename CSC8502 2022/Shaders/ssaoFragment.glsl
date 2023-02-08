#version 330 core

uniform sampler2D texNoise;
uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D worldPosTex;

uniform vec3 samples[64];
uniform vec2 screenSize;

uniform vec2 pixelSize; // reciprocal of resolution

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 inverseProjView;

out vec4 fragColour;

int kernelSize = 64;
float radius = 10.5;
float bias = 0.025;

void main()
{
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	vec3 worldPos = texture(worldPosTex, texCoord.xy).xyz;
	vec3 normal = texture(normTex, texCoord.xy).rgb;
	float fragDepth = texture(depthTex, texCoord.xy).r;

	vec2 noiseScale = vec2(screenSize.x / 4.0, screenSize.y / 4.0);
	vec3 randomVec = texture(texNoise, texCoord * noiseScale).xyz; 
	
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 	
	
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 samplePos = TBN * samples[i]; // from tangent to view-space
		samplePos = worldPos + samplePos * radius;
		
		vec4 offset = vec4(samplePos, 1.0);
		offset      = projMatrix * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 

		float sampleDepth = texture(worldPosTex, offset.xy).z; 

		//occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);  

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(worldPos.z - sampleDepth));
		occlusion       += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
	}  
	occlusion = 1.0 - (occlusion / kernelSize);

	fragColour = vec4(occlusion, occlusion, occlusion, 1.0);
	//fragColour = occlusion;
}