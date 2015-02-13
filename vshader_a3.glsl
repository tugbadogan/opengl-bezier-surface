#version 150 

in  vec4 vPosition;
in  vec3 vNormal;
in vec2 texture_coordinate;
out vec4 color;
out vec2 o_texture_coordinate;
out Data
{
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
}DataOut;
out vec3 cubeTexCoord;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;
uniform int Wireframe;
uniform int Texture;
uniform int BumpTexture;
uniform int EnvironmentTexture;

void main()
{
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
	
    vec3 L = normalize( LightPosition.xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd*DiffuseProduct;

    float Ks = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = Ks * SpecularProduct;
    
    if( dot(L, N) < 0.0 )
	{
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = Projection * ModelView * vPosition;

	if(Wireframe == 0)
	{
		color = ambient + diffuse + specular;
		color.a = 1.0;
	}
	else
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	// Parametric texture mapping
	if(Texture == 1)
	{
		color = specular;
		o_texture_coordinate = texture_coordinate;
	}
	// Bump Texture
	else if(BumpTexture == 1) {
		o_texture_coordinate = texture_coordinate;
	}
	// Environment texture mapping
	if(EnvironmentTexture == 1) {
		cubeTexCoord = pos;
	}
	DataOut.normal = N;
	DataOut.eye = E;
	DataOut.lightDir = L; 
}