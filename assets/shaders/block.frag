#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec2 TileOrigin;
in vec3 FaceNormal;
in vec3 pos_ws;

uniform vec3 main_light;
uniform sampler2D block_texture;
uniform vec3 camera_pos_ws;
uniform vec4 fog_color;
uniform float fog_distance;

void main()
{
    vec2 localUV = fract(TexCoord);
    vec2 atlasUV = TileOrigin + localUV / 14.0; // atlas is 14x14
    vec4 tex_color = texture(block_texture, atlasUV);

    // float light = min(dot(FaceNormal, main_light) + 1 + 0.2, 1);
    // vec4 theTexture = texture(block_texture, atlasUV);
    // FragColor = vec4(light * theTexture.rgb, theTexture.a);

    float view_distance = length(camera_pos_ws - pos_ws) - fog_distance;
    float fog_factor = clamp(exp(-0.1 * view_distance), 0, 1);

    FragColor = mix(tex_color, mix(fog_color, tex_color, fog_factor), fog_color.a);
}
