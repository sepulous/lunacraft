#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec2 TileOrigin;
in vec3 FaceNormal;

uniform vec3 MainLight;
uniform sampler2D ourTexture;

void main()
{
    vec2 localUV = fract(TexCoord);
    vec2 atlasUV = TileOrigin + localUV / 14.0; // atlas is 14x14

    // float light = min(dot(FaceNormal, MainLight) + 1 + 0.2, 1);
    // vec4 theTexture = texture(ourTexture, atlasUV);
    // FragColor = vec4(light * theTexture.rgb, theTexture.a);

    FragColor = texture(ourTexture, atlasUV);
}
