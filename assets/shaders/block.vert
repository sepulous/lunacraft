#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aUV;
layout (location = 2) in vec3 aFaceNormal;

out vec2 TexCoord;
out vec2 TileOrigin;
out vec3 FaceNormal;
out vec3 pos_ws;

uniform mat4 view_projection;

void main()
{
    pos_ws = aPos;
    gl_Position = view_projection * vec4(aPos, 1.0);
    TexCoord = aUV.xy;
    TileOrigin = aUV.zw;
    FaceNormal = aFaceNormal;
}
