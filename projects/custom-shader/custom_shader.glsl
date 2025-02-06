#pragma sokol @ctype mat4 HMM_Mat4

@vs custom_shader_vs

in vec2 position;

layout(binding=0) uniform custom_shader_vs_params
{
    mat4 mvp;
};

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}

@end

@fs custom_shader_fs

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.);
}

@end

@program custom_shader custom_shader_vs custom_shader_fs