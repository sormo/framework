@ctype mat4 HMM_Mat4
@ctype vec3 frame::vec3
@ctype vec4 frame::col4

@vs sshapes_vs
uniform sshapes_vs_params
{
    mat4 mvp;
    mat4 model;
    vec4 color;
    int shading_type;
    vec3 light_direction;
};

const int shading_type_none = 0;
const int shading_type_flat = 1;

in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color0;

out vec4 out_color;

void main()
{
    gl_Position = mvp * position;

    if (shading_type == shading_type_flat)
    {
        vec3 world_normal = normalize(vec3(model * vec4(normal, 0.0)));
        float color_factor = clamp(dot(light_direction, world_normal), 0.5, 1.0);
        out_color = vec4(color.rgb * color_factor, color.a);
    }
    else
    {
        out_color = color;
    }
    //out_color = vec4((normal + 1.0) * 0.5, 1.0);
}
@end

@fs sshapes_fs
in vec4 out_color;
out vec4 frag_color;

void main()
{
    frag_color = out_color;
}
@end

@program sshapes sshapes_vs sshapes_fs