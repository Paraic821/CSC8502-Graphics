#version 330 core

uniform sampler2D texNoise;
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
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

//in Vertex {
//	vec2 texCoord;
//	vec3 normal;
//	vec3 worldPos;
//} IN;

//out float4 fragColour;
out vec4 fragColour;

int kernelSize = 64;
float radius = 5.0;
float bias = 0.025;

void main()
{
	//vec3 fragPos   = texture(gPosition, IN.texCoord).xyz;
	//vec3 normal    = texture(gNormal, IN.texCoord).rgb;
	//vec3 randomVec = texture(texNoise, TexCoords * screenSize).xyz; 

	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;
	vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1.0);
	//vec3 worldPos = invClipPos.xyz / invClipPos.w;
	vec3 worldPos = texture(worldPosTex, texCoord.xy).rgb;


	vec3 normal = normalize(texture(normTex, texCoord.xy).xyz * 2.0 -1.0);


	//vec3 randomVec = texture(texNoise, IN.texCoord * screenSize).xyz; 
	vec3 randomVec = texture(texNoise, texCoord * screenSize).xyz; 
	
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);  
	
	
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		vec3 samplePos = TBN * (samples[i] * radius); // from tangent to view-space
		samplePos = worldPos + samplePos; 
		
		vec4 offset = vec4(samplePos, 1.0);
		offset      = projMatrix * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 

		float sampleDepth = texture(worldPosTex, offset.xy).z; 
		//float sampleDepth = depth;
		//float sampleDepth = IN.worldPos.z; 

		//occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);  

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(worldPos.z - sampleDepth));
		occlusion       += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
	}  
	occlusion = 1.0 - (occlusion / kernelSize);

	fragColour = vec4(occlusion, occlusion, occlusion, 1.0);
	
	//fragColour = vec4(worldPos, 1.0);
	//fragColour = vec4(invClipPos);
	//fragColour = vec4(ndcPos.x, ndcPos.y, ndcPos.z, 1.0);
	//fragColour = vec4(depth, depth, depth, 1.0);
	//fragColour = vec4(normal.x, normal.y, normal.z, 1.0);
	//fragColour = vec4(texCoord, 0.0, 1.0);
}