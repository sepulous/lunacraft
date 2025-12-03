#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord; // Stores repeat information, not exact UV coords
layout (location = 2) in vec2 aTileOrigin;
layout (location = 3) in vec3 aFaceNormal;

out vec2 TexCoord;
out vec2 TileOrigin;
out vec3 FaceNormal;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    TileOrigin = aTileOrigin;
    FaceNormal = aFaceNormal;
}
