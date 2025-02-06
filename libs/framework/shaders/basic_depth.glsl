@vs basic_depth_vs

in vec3 position;
out vec4 fs_color;

layout(binding=0) uniform basic_depth_vs_params
{
    mat4 mvp;
    vec4 color;
};

void main()
{
    gl_Position = mvp * vec4(position.xyz, 1.0);

    fs_color = color;
}

@end

@fs basic_depth_fs

out vec4 FragColor;
in vec4 fs_color;

void main()
{
    FragColor = fs_color;
}

@end

@program basic_depth basic_depth_vs basic_depth_fs