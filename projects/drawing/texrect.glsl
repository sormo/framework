#pragma sokol @ctype mat4 hmm_mat4

#pragma sokol @vs vs
uniform vs_params
{
    mat4 mvp;
    vec4 color0;
};

in vec2 position;
in vec2 texcoord0;

out vec4 color;
out vec2 uv;

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    color = color0;
    uv = texcoord0;
}
#pragma sokol @end

#pragma sokol @fs fs
uniform texture2D tex;
uniform sampler smp;

in vec4 color;
in vec2 uv;
out vec4 frag_color;

void main()
{
    frag_color = texture(sampler2D(tex,smp), uv) * color;
}
#pragma sokol @end

#pragma sokol @program texrect vs fs