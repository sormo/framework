#pragma sokol @ctype mat4 hmm_mat4

@vs planet_vs

in vec2 position;

out vec2 fs_position;

uniform vs_params_planet
{
    mat4 mvp;
};

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    
    fs_position = position; // position ranges in [-0.5, 0.5] in both x and y
}

@end

@fs planet_fs

out vec4 FragColor;
in vec2 fs_position;

const float radius = 0.5;

uniform fs_params_planet
{
    vec3 planet_color;
    vec3 light_position;
    vec3 camera_position;

    float light_power;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float shininess;
};

vec3 sphere_normal(vec2 p, float r)
{
    float pxy = sqrt(p.x * p.x + p.y * p.y);
    float alpha = acos(pxy / r);
    float pz = r * sin(alpha);

    return normalize(vec3(p, pz));
}

bool is_point_in_circle()
{
    return distance(fs_position, vec2(0.0, 0.0)) <= radius;
}

void main()
{
    if (!is_point_in_circle())
        discard;

    vec3 normal = sphere_normal(fs_position, radius);
    vec3 sphere_position = normal * radius;

    vec3 light_dir = light_position - sphere_position;
    float distance = length(light_dir);
    distance = distance * distance;
    light_dir = normalize(light_dir);

    float lambertian = max(dot(normal, light_dir), 0.0);
    float specular = 0.0;

    if (lambertian > 0.0)
    {
        vec3 view_dir = normalize(camera_position - sphere_position);

        // this is blinn phong
        vec3 half_dir = normalize(light_dir + view_dir);
        float spec_angle = max(dot(half_dir, normal), 0.0);
        specular = pow(spec_angle, shininess);

        // this is phong (for comparison)
        //vec3 reflect_dir = reflect(-light_dir, normal);
        //spec_angle = max(dot(reflect_dir, view_dir), 0.0);
        //specular = pow(spec_angle, shininess / 4.0);
    }

    vec3 result = ambient_color + 
                  diffuse_color * lambertian * planet_color * light_power / distance + 
                  specular_color * specular * planet_color * light_power / distance;
    
    FragColor = vec4(result, 1.0);
}

@end

@program planet planet_vs planet_fs