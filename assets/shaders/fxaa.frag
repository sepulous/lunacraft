#version 330 core

in vec2 v_uv;
out vec4 out_color;

uniform sampler2D screenTexture;
uniform vec2 resolution;

float luma(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texel = 1.0 / resolution;

    vec3 rgbM = texture(screenTexture, v_uv).rgb;

    vec3 rgbNW = texture(screenTexture, v_uv + vec2(-texel.x, -texel.y)).rgb;
    vec3 rgbNE = texture(screenTexture, v_uv + vec2( texel.x, -texel.y)).rgb;
    vec3 rgbSW = texture(screenTexture, v_uv + vec2(-texel.x,  texel.y)).rgb;
    vec3 rgbSE = texture(screenTexture, v_uv + vec2( texel.x,  texel.y)).rgb;

    float lumaM  = luma(rgbM);
    float lumaNW = luma(rgbNW);
    float lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW);
    float lumaSE = luma(rgbSE);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * 0.0078125, 0.0009765625);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = clamp(dir * rcpDirMin, vec2(-5.0), vec2(5.0)) * texel;

    vec3 rgbA = 0.5 * (
        texture(screenTexture, v_uv + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(screenTexture, v_uv + dir * (2.0 / 3.0 - 0.5)).rgb
    );

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, v_uv + dir * -0.5).rgb +
        texture(screenTexture, v_uv + dir * 0.5).rgb
    );

    float subpixel = 0.5;
    rgbA = mix(rgbM, rgbA, subpixel);
    rgbB = mix(rgbM, rgbB, subpixel);

    float lumaB = luma(rgbB);

    if (lumaB < lumaMin || lumaB > lumaMax)
        out_color = vec4(rgbA, 1.0);
    else
        out_color = vec4(rgbB, 1.0);
}
