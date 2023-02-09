#version 330 core

//uniform sampler2D bumpTex;
//uniform sampler2D rockBumpTex;
//uniform sampler2D diffuseTex;
//uniform sampler2D rockDiffuseTex;
//uniform sampler2D grassTex;

uniform sampler2D ssao;
uniform vec2 pixelSize; // reciprocal of resolution

uniform sampler2D diffuseTex0;
uniform sampler2D diffuseTex1;
uniform sampler2D diffuseTex2;

//uniform float heights[3];

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

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

	float heights[4] = float[4](30, 50, 60, 120);

	vec4 diffuse = vec4(0,0,0,0);
	vec3 normal = IN.normal;
	
	if(IN.worldPos.y > heights[3]){
		diffuse = texture( diffuseTex2, IN.texCoord );
	}
	else if(IN.worldPos.y <= heights[3] && IN.worldPos.y  > heights[2]){
		vec4 tex1 = texture( diffuseTex2, IN.texCoord );
		vec4 tex2 = texture( diffuseTex1, IN.texCoord );		
		float blendFactor = (IN.worldPos.y - heights[2]) / (heights[3] - heights[2]);	
		diffuse = mix(tex2, tex1, blendFactor);
	}
	else if(IN.worldPos.y <= heights[2] && IN.worldPos.y  > heights[1]){
		diffuse = texture( diffuseTex1, IN.texCoord );
	}
	else if(IN.worldPos.y <= heights[1] && IN.worldPos.y  > heights[0]){
		vec4 tex1 = texture( diffuseTex1, IN.texCoord );
		vec4 tex2 = texture( diffuseTex0, IN.texCoord );		
		float blendFactor = (IN.worldPos.y - heights[0]) / (heights[1] - heights[0]);	
		diffuse = mix(tex2, tex1, blendFactor);
	}
	else if(IN.worldPos.y <= heights[0]){
		diffuse = texture( diffuseTex0, IN.texCoord );
	}

	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float ambientOcclusion = texture(ssao, texCoord).r;
	
	float lambert = max( dot( incident, normal ), 0.0f );
	float distance = length( lightPos - IN.worldPos );
	float attenuation = 1.0 - clamp( distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp( dot( halfDir , normal ), 0.0, 1.0);
	specFactor = pow( specFactor , 60.0 );

	vec3 surface = diffuse.rgb * lightColour.rgb;
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += ( lightColour.rgb * specFactor ) * attenuation * 0.33;
	fragColour.rgb += surface * 0.2f * ambientOcclusion; //ambient!
	fragColour.a = diffuse.a;

	fragColour = vec4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1.0);
}
