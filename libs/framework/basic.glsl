@vs basic_vs
in vec2 position;
out vec4 fs_color;

uniform basic_vs_params
{
    mat4 mvp;
    vec4 color;
};

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    fs_color = color;
}
@end

@fs basic_fs
out vec4 FragColor;
in vec4 fs_color;

void main()
{
    FragColor = fs_color;
}
@end

@program basic basic_vs basic_fs