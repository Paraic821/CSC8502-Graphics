#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D ssao;

uniform vec2 pixelSize; // reciprocal of resolution

uniform float hasBumpMap;
uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;
uniform float toggleAO;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main ( void ) {
	vec3 incident = normalize( lightPos - IN.worldPos );
	vec3 viewDir = normalize( cameraPos - IN.worldPos );
	vec3 halfDir = normalize( incident + viewDir );
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float ambientOcclusion = texture(ssao, texCoord).r;
	ambientOcclusion = toggleAO >= 0 ? ambientOcclusion : 1.0;

	vec4 diffuse = texture( diffuseTex , IN.texCoord );
	vec3 normal = IN.normal;
	if(hasBumpMap > 0.0){
		normal = texture( bumpTex , IN.texCoord ).rgb;
	}
	
	float lambert = max( dot( incident, normal ), 0.0f );
	float distance = length( lightPos - IN.worldPos );
	float attenuation = 1.0 - clamp( distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp( dot( halfDir , normal ), 0.0, 1.0);
	specFactor = pow( specFactor , 60.0 );

	vec3 surface = diffuse.rgb * lightColour.rgb;
	//surface *= 0.3f * ambientOcclusion;
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += ( lightColour.rgb * specFactor ) * attenuation * 0.33;
	fragColour.rgb += surface * 0.3f * ambientOcclusion; //ambient!
	fragColour.a = diffuse.a;

	//fragColour = vec4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1.0);
}
