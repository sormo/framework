#pragma sokol @ctype mat4 hmm_mat4

#pragma sokol @vs model3d_flat_vs
layout(binding=0) uniform model3d_flat_vs_params
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec3 position;

out vec3 position1;
out vec3 position2;
out vec3 position3;

void main()
{
    vec4 world_position = model * vec4(position, 1.0);

    gl_Position = projection * view * world_position;

    if (gl_VertexIndex % 3 == 0)
        position1 = vec3(gl_Position);
    else if (gl_VertexIndex % 3 == 1)
        position2 = vec3(gl_Position);
    else if (gl_VertexIndex % 3 == 2)
        position3 = vec3(gl_Position);
}
#pragma sokol @end

#pragma sokol @fs model3d_flat_fs

in vec3 position1;
in vec3 position2;
in vec3 position3;

out vec4 frag_color;

void main()
{
    vec3 edge1 = position2 - position1;
    vec3 edge2 = position3 - position1;
    vec3 normal = normalize(cross(edge1, edge2));

    frag_color = vec4(normal * 0.5 + 0.5, 1.0);
}
#pragma sokol @end

#pragma sokol @program model3d_flat model3d_flat_vs model3d_flat_fs