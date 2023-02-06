# version 330 core

in Vertex {
	vec4 colour ;
} IN ;

void triforce(){
	if(IN.colour.r < 0.5 && IN.colour.g < 0.5 && IN.colour.b < 0.5) discard;
}

void interiorTriangle(){
	if(IN.colour.r > 0.1 && IN.colour.g > 0.1 && IN.colour.b > 0.1) discard;
}

void corners(){
	if((IN.colour.r > 0.8 && IN.colour.r < 0.9) || (IN.colour.g > 0.8 && IN.colour.g < 0.9)  || (IN.colour.b > 0.8 && IN.colour.b < 0.9) ) discard;
}

out vec4 fragColour ;
	void main ( void ) {
	
	//triforce();
	//interiorTriangle();
	//corners();

	fragColour = IN.colour ;
}
