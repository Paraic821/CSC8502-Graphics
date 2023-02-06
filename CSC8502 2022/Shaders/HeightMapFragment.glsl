#version 330 core

uniform sampler2D diffuseTex;
//uniform sampler2D bumpTex;
uniform sampler2D rockDiffuseTex;
//uniform sampler2D rockBumpTex;
uniform sampler2D grassTex;
uniform sampler2D shadowTex;

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
	vec3 tangent;
	vec3 binormal;
	vec4 shadowProj;
} IN;

out vec4 fragColour;

void main ( void ) {
	vec3 incident = normalize( lightPos - IN.worldPos );
	vec3 viewDir = normalize( cameraPos - IN.worldPos );
	vec3 halfDir = normalize( incident + viewDir );

	mat3 TBN = mat3( normalize( IN.tangent ), normalize ( IN.binormal ) , normalize( IN.normal ));

	float heights[4] = float[4](30, 50, 60, 120);

	vec4 diffuse = vec4(0,0,0,0);
	vec3 normal = IN.normal;
	normal = normalize( TBN * normal * 2.0 - 1.0);
	
	if(IN.worldPos.y > heights[3]){
		diffuse = texture( rockDiffuseTex, IN.texCoord );
	}
	else if(IN.worldPos.y <= heights[3] && IN.worldPos.y  > heights[2]){
		vec4 tex1 = texture( rockDiffuseTex, IN.texCoord );
		vec4 tex2 = texture( grassTex, IN.texCoord );		
		float blendFactor = (IN.worldPos.y - heights[2]) / (heights[3] - heights[2]);	
		diffuse = mix(tex2, tex1, blendFactor);
	}
	else if(IN.worldPos.y <= heights[2] && IN.worldPos.y  > heights[1]){
		diffuse = texture( grassTex, IN.texCoord );
	}
	else if(IN.worldPos.y <= heights[1] && IN.worldPos.y  > heights[0]){
		vec4 tex1 = texture( grassTex, IN.texCoord );
		vec4 tex2 = texture( diffuseTex, IN.texCoord );		
		float blendFactor = (IN.worldPos.y - heights[0]) / (heights[1] - heights[0]);	
		diffuse = mix(tex2, tex1, blendFactor);
	}
	else if(IN.worldPos.y <= heights[0]){
		diffuse = texture( diffuseTex, IN.texCoord );
	}
	
	float lambert = max( dot( incident, normal ), 0.0f );
	float distance = length( lightPos - IN.worldPos );
	float attenuation = 1.0 - clamp( distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp( dot( halfDir , normal ), 0.0, 1.0);
	specFactor = pow( specFactor , 60.0 );

	float shadow = 1.0;

	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w ;

	if( abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ) {
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture( shadowTex, biasCoord.xy ).x;
		if( shadowZ < biasCoord.z ) {
			shadow = 0.0f;
		}
	}

	vec3 surface = diffuse.rgb * lightColour.rgb;
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += ( lightColour.rgb * specFactor ) * attenuation * 0.33;
	fragColour.rgb *= shadow; // shadowing factor
	fragColour.rgb += surface * 0.1f; //ambient!
	fragColour.a = diffuse.a;
}
