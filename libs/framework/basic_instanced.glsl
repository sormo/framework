@vs basic_instanced_vs
in vec2 position;
in vec4 model0;
in vec4 model1;
in vec4 model2;
in vec4 model3;
in vec4 color;
out vec4 fs_color;

uniform basic_instanced_vs_params
{
    mat4 view_projection;
};

void main()
{
    mat4 model = mat4(model0, model1, model2, model3);

    gl_Position = view_projection * model * vec4(position, 0.0, 1.0);
    fs_color = color;
}
@end

@fs basic_instanced_fs
out vec4 FragColor;
in vec4 fs_color;

void main()
{
    FragColor = fs_color;
}
@end

@program basic_instanced basic_instanced_vs basic_instanced_fs