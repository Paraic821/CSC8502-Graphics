#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 fragColor;

const float scaleFactors[7] = float [](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);

mat3 sx = mat3( 
    1.0, 2.0,	1.0, 
    0.0, 0.0,	0.0, 
   -1.0, -2.0, -1.0 
);
mat3 sy = mat3( 
    1.0, 0.0, -1.0, 
    2.0, 0.0, -2.0, 
    1.0, 0.0, -1.0 
);

void main ( void ) {

	vec3 diffuse = texture(sceneTex, IN.texCoord.st).rgb;
    mat3 I;
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            vec3 sample  = texelFetch(sceneTex, ivec2(gl_FragCoord) + ivec2(i-1,j-1), 0 ).rgb;
            I[i][j] = length(sample); 
		}
	}

	float gx = dot(sx[0], I[0]) + dot(sx[1], I[1]) + dot(sx[2], I[2]); 
	float gy = dot(sy[0], I[0]) + dot(sy[1], I[1]) + dot(sy[2], I[2]);

	float g = sqrt(pow(gx, 2.0)+pow(gy, 2.0));

    // Try different values and see what happens
    g = smoothstep(0.999, 0.001, g);

    vec3 edgeColor = vec3(0.0, 0.0, 0.0);
    float blurFactor = vec4(mix(diffuse, edgeColor, g), 1.0).r;

	if(blurFactor < 0.2){
		fragColor = texture2D(sceneTex, IN.texCoord.xy);
	}
	else{
		fragColor = vec4 (0, 0, 0, 1);
		vec2 delta = vec2 (0, 0);

		if(isVertical == 1) {
			delta = dFdy(IN.texCoord);
		}
		else {
			delta = dFdx(IN.texCoord);
		}

		for (int i = 0; i < 7; i++ ) {
			vec2 offset = delta * (i - 3);
			vec4 tmp = texture2D(sceneTex, IN.texCoord.xy + offset);
			fragColor += tmp * scaleFactors[i];
		}
	}
}
