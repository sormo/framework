#pragma sokol @ctype mat4 HMM_Mat4
#pragma sokol @ctype vec4 frame::col4

#pragma sokol @vs basic_image_vs

uniform basic_image_vs_params
{
    mat4 mvp;
    vec4 color0;
};

in vec2 position;

out vec4 color;
out vec2 uv;

void main()
{
    gl_Position = mvp * vec4(position - 0.5, 0.0, 1.0);
    color = color0;
    uv = position;
}

#pragma sokol @end


#pragma sokol @fs basic_image_fs

uniform texture2D texture_fs;
uniform sampler sampler_fs;

in vec4 color;
in vec2 uv;
out vec4 frag_color;

void main()
{
    frag_color = texture(sampler2D(texture_fs, sampler_fs), uv) * color;
}

#pragma sokol @end

#pragma sokol @program basic_image basic_image_vs basic_image_fs