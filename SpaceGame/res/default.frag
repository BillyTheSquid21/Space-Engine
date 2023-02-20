#version 430 core

in vec3 v_Normal;
in vec2 v_Tex;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    //Color based on texture
    FragColor = vec4(vec3(texture(u_Texture, v_Tex)), 1.0);
}