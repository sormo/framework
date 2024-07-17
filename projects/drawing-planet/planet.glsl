#pragma sokol @ctype mat4 hmm_mat4

@vs planet_vs

in vec2 position;
in vec2 uv;

out vec3 fs_color;
out vec2 fs_uv;
out vec3 fs_light_position;

uniform vs_params_planet
{
    mat4 mvp;
    vec3 color;
    vec3 light_position;
};

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    
    fs_color = color;
    fs_uv = uv - 0.5; // send to fragment shader uv with [0,0] in the center, ranging [-0.5, 0.5] in both u and v
    fs_light_position = light_position;
}

@end

@fs planet_fs

out vec4 FragColor;
in vec3 fs_color;
in vec2 fs_uv;
in vec3 fs_light_position;

const float radius = 0.5;

uniform fs_params_planet
{
    vec3 camera_position;
    float light_power;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 spec_color;
    float shininess;
    float screen_gamma;
    int light_mode;
};

//const float light_power = 40.0;
//const vec3 ambient_color = vec3(0.1, 0.0, 0.0);
//const vec3 diffuse_color = vec3(0.5, 0.0, 0.0);
//const vec3 spec_color = vec3(1.0, 1.0, 1.0);
//const float shininess = 16.0;
//const float screen_gamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

vec3 sphere_normal(vec2 p, float r)
{
    float pxy = sqrt(p.x * p.x + p.y * p.y);
    float alpha = acos(pxy / r);
    float pz = r * sin(alpha);

    return normalize(vec3(p, pz));
}

bool is_point_in_circle()
{
    return distance(fs_uv, vec2(0.0, 0.0)) <= radius;
}

void main()
{
    if (!is_point_in_circle())
        discard;

    vec3 normal = sphere_normal(fs_uv, radius);
    vec3 sphere_position = normal * radius; // not sure whether this should be position on spehere or position of sphere
    //vec3 sphere_position = vec3(0.0, 0.0, 0.0);

    vec3 light_dir = fs_light_position - sphere_position;
    float distance = length(light_dir);
    distance = distance * distance;
    light_dir = normalize(light_dir);

    float lambertian = max(dot(normal, light_dir), 0.0);
    float specular = 0.0;

    if (lambertian > 0.0 && light_mode > 0)
    {
        //vec3 view_dir = normalize(-sphere_position);
        vec3 view_dir = normalize(camera_position - sphere_position);

        // this is blinn phong
        vec3 half_dir = normalize(light_dir + view_dir);
        float spec_angle = max(dot(half_dir, normal), 0.0);
        specular = pow(spec_angle, shininess);

        // this is phong (for comparison)
        if (light_mode == 2)
        {
            vec3 reflect_dir = reflect(-light_dir, normal);
            spec_angle = max(dot(reflect_dir, view_dir), 0.0);
            // note that the exponent is different here
            specular = pow(spec_angle, shininess / 4.0);
        }
    }

    vec3 color_linear = ambient_color + 
                        diffuse_color * lambertian * fs_color * light_power / distance + 
                        spec_color * specular * fs_color * light_power / distance;
    
    //vec3 color_linear = ambient_color + diffuse_color * lambertian * fs_color * light_power / distance;

    // apply gamma correction (assume ambientColor, diffuseColor and specColor
    // have been linearized, i.e. have no gamma correction in them)
    //vec3 color_gamma_corrected = pow(color_linear, vec3(1.0 / screen_gamma));
    vec3 color_gamma_corrected = color_linear;
    // use the gamma corrected color in the fragment
    FragColor = vec4(color_gamma_corrected, 1.0);
}

@end

@program planet planet_vs planet_fs