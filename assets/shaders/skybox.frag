#version 330 core

out vec4 out_color;

in vec3 v_tex;

uniform samplerCube u_skybox;
uniform mat3 u_rotation;

void main()
{    
    out_color = texture(u_skybox, u_rotation * v_tex);
}
