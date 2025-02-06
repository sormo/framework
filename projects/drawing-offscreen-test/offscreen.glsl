@ctype mat4 HMM_Mat4

// shared code for all shaders
@block uniforms
layout(binding=0) uniform vs_params
{
    mat4 mvp;
};
@end

// offscreen rendering shaders
@vs vs_offscreen
@include_block uniforms

in vec4 position;
in vec4 normal;
out vec4 normal_fs;

void main()
{
    gl_Position = mvp * position;
    normal_fs = normal;
}
@end

@fs fs_offscreen
in vec4 normal_fs;
out vec4 frag_color;

void main()
{
    frag_color = vec4(normal_fs.xyz * 0.5 + 0.5, 1.0);
}
@end

@program offscreen vs_offscreen fs_offscreen

// display-pass shaders
@vs vs_display
@include_block uniforms

in vec2 position;
out vec2 uv;

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    uv = position + 0.5;
}
@end

@fs fs_display
layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler smp;

in vec2 uv;
out vec4 frag_color;

void main()
{
    vec4 c = texture(sampler2D(tex, smp), uv);
    frag_color = c;
}
@end

@program display vs_display fs_display
