#version 150 

in  vec4 color;
in vec2 o_texture_coordinate;
in vec3 cubeTexCoord;
out vec4 fColor;
uniform int PhongShading;
uniform int Texture;
uniform int BumpTexture;
uniform int EnvironmentTexture;
uniform float Shininess;
uniform vec4 LightPosition;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform sampler2D tex;
uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform samplerCube cubemap_texture;
in Data
{
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
}DataIn;

void main() 
{ 
	if(PhongShading == 1)
	{
		vec4 spec = vec4(0.0);
		vec3 n = normalize(DataIn.normal);
		vec3 e = normalize(DataIn.eye);

		float intensity = max(dot(n, DataIn.lightDir), 0.0);
		if(intensity > 0.0)
		{
			vec3 h = normalize(DataIn.lightDir + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = SpecularProduct * pow(intSpec, Shininess);
		}
		fColor = max(intensity * DiffuseProduct + spec, AmbientProduct);
	}
	// Parametric texture mapping
	else if(Texture == 1)
	{
		fColor =  texture(tex, o_texture_coordinate);
	}
	else if(BumpTexture == 1) {
		vec3 normal = normalize(texture2D(normal_texture, o_texture_coordinate).rgb * 2.0 - 1.0);
		vec3 light_pos = DataIn.lightDir;
		
		// Calculate the lighting diffuse value
		float diffuse = max(dot(normal, light_pos), 0.0);
		
		vec3 color = diffuse * texture2D(color_texture, o_texture_coordinate).rgb;
		fColor = vec4(color, 1.0); 
	}
	else if(EnvironmentTexture == 1) {
		fColor = texture(cubemap_texture, cubeTexCoord);
	}
	else
	{
		fColor = color;
	}
}