#version 330 core
in vec2 TexCoords;
in vec2 v_position;
out vec4 color;
uniform vec3 u_offset; //X = Y offset; Y = MaxDistance; Z MinDistance
uniform sampler2D u_texture;
uniform vec3 textColor;

void main()
{   
	if(u_offset.y > v_position.y)
	{
		discard;
	}
	if(u_offset.z < v_position.y)
	{
		discard;
	}
	float sampled = texture(u_texture, TexCoords).a;
	color = vec4(textColor, sampled);
}  