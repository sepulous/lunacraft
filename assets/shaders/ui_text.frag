#version 330 core

in vec4 color;
in vec2 uvCoords;

uniform sampler2D uFontAtlasTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(uFontAtlasTexture, uvCoords).r) * color;
    //FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
