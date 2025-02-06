#pragma sokol @ctype mat4 HMM_Mat4

@block shadertoy_shader

@end

@vs shadertoy_vs

in vec2 uv;

out vec2 fs_uv;

layout(binding=0) uniform vs_params_shadertoy
{
    mat4 mvp;
};

void main()
{
    gl_Position = mvp * vec4(uv, 0.0, 1.0);
    
    fs_uv = uv;
}

@end

@fs shadertoy_fs

out vec4 frag_color;
in vec2 fs_uv;

layout(binding=1) uniform fs_params_shadertoy
{
    vec3 iResolution;
    float iTime;
    float iTimeDelta;
    float iFrame;
    //float iChannelTime[4];
    vec4 iMouse;
    vec4 iDate;
    //float iSampleRate;
    //vec3 iChannelResolution[4];
    //samplerXX iChanneli;
};

//@include_block shadertoy_shader
@include shadertoy-shader.glsl

void main()
{
    vec2 pixel_position = fs_uv * vec2(iResolution.xy);

    mainImage(frag_color, pixel_position);
}

@end

@program shadertoy shadertoy_vs shadertoy_fs