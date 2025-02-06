#pragma sokol @ctype mat4 HMM_Mat4
#pragma sokol @ctype vec3 frame::vec3

#pragma sokol @vs body_draw_model_vs
layout(binding=0) uniform body_draw_model_vs_params
{
    mat4 model;
    mat4 projection_view;
};

in vec3 position;
in vec3 normal;

out vec3 frag_normal;
out vec3 frag_position;

void main()
{
    frag_normal = mat3(model) * normal;
    frag_position = vec3(model * vec4(position, 1.0));
    gl_Position = projection_view * model * vec4(position, 1.0);
}
#pragma sokol @end

#pragma sokol @fs body_draw_model_fs
layout(binding=1) uniform body_draw_model_fs_params
{
    vec3 object_color;
    vec3 light_color;

    vec3 view_position;
    vec3 light_position;

    float ambient_strength;
    float specular_strength;
};

in vec3 frag_normal;
in vec3 frag_position;
out vec4 frag_color;

vec3 compute_phong() 
{
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_direction = normalize(light_position - frag_position);
    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 view_direction = normalize(view_position - frag_position);
    vec3 reflect_direction = reflect(-light_direction, norm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    return (ambient + diffuse + specular) * object_color;
}

void main()
{
    frag_color = vec4(compute_phong(), 1.0);
}
#pragma sokol @end

#pragma sokol @program body_draw_model body_draw_model_vs body_draw_model_fs