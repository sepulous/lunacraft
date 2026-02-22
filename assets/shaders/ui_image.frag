#version 330 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D u_tex;
uniform float u_opacity;
uniform float u_darkness;
uniform vec4 u_crop; // (minX, minY, maxX, maxY) in UV space (0-1)

void main()
{
    vec2 cropped_uv = mix(u_crop.xy, u_crop.zw, v_uv);
    vec4 tex_color = texture(u_tex, cropped_uv);
    out_color = vec4(tex_color.rgb * (1 - u_darkness), tex_color.a * u_opacity);
}
