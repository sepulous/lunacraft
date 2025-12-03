#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform mat3 rotation;

void main()
{    
    FragColor = texture(skybox, rotation * TexCoords);
}
