#pragma sokol @ctype mat4 HMM_Mat4
#pragma sokol @ctype vec3 frame::vec3

@vs planet_vs

in vec2 position;

out vec2 fs_position;

layout(binding=0) uniform vs_params_planet
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

const float planet_radius = 0.25;
const float e = 2.718281828459;

layout(binding=1) uniform fs_params_planet
{
    vec3 camera_position;
    
    // light
    vec3 light_position;
    float light_power;

    // material
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float shininess;

    // atmosphere
    float atmosphere_radius_relative; // [0,1] radius relative to planet radius
    vec3 atmosphere_color;
    float atmosphere_density;
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
    return distance(fs_position, vec2(0.0, 0.0)) <= planet_radius + planet_radius * atmosphere_radius_relative;
}

vec4 draw_sphere(float radius, vec3 ambient_color, vec3 diffuse_color, vec3 specular_color, float light_power)
{
    float d = length(fs_position);
    if (d > radius)
        return vec4(0.);

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
                  diffuse_color * lambertian * light_power / distance +
                  specular_color * specular * light_power / distance;

    return vec4(result, 1.);
}

float remap01(float a, float b, float t)
{
    return (t - a) / (b - a);
}

float compute_attenuation(float x)
{
    return pow(1 + x / 0.5, 2.)* pow(e, -3.8 * x);
}

//float compute_attenuation(float x)
//{
//    return 0.5 * cos(3.1 * x) + 0.5;
//}

// convert to corresponding vec4 color with alpha set, resulting color should maintain same
// appearance while drawn agains black background
vec4 convert_to_vec4(vec3 rgb)
{
    float a = max(rgb.r, max(rgb.g, rgb.b));
    if (a == 0.)
        return vec4(0.);

    vec3 rgb_adjusted = rgb / a;
    return vec4(rgb_adjusted, a);
}

vec4 draw_atmosphere()
{
    float d = length(fs_position);

    float atmosphere_radius = planet_radius + atmosphere_radius_relative * planet_radius;

    vec3 normal = sphere_normal(fs_position, atmosphere_radius);
    vec3 sphere_position = normal * atmosphere_radius;

    vec3 light_dir = light_position - sphere_position;
    float distance = length(light_dir);
    distance = distance * distance;
    light_dir = normalize(light_dir);

    float lambertian = max(dot(normal, light_dir), 0.0);

    vec3 result = atmosphere_color * smoothstep(atmosphere_radius, planet_radius, d);
    //vec3 result = atmosphere_color;

    // less atmosphere toward center
    float attenuation = compute_attenuation(remap01(0., atmosphere_radius, atmosphere_radius - d)) * atmosphere_density;

    //float attenuation =  0.01f;
    // not sure about this, this will compute intersection of ray from camera with atmosphere 
    //vec3 ro = camera_position;
    //vec3 rd = normalize(vec3(fs_position, -1.));
    //vec3 s = vec3(0.);
    //float r = atmosphere_radius;
    //float rp = planet_radius;
    //float t = dot(s - ro, rd);
    //vec3 p = ro + rd * t;
    //float y = length(s - p);
    //if (y < r)
    //{
    //    float x = sqrt(r * r - y * y);
    //    float t1 = t - x;
    //    float t2 = t + x;

    //    float c = abs(t2 - t1);
    //    c = remap01(0.0, 2. * r, c);
    //    //c = c * atmosphere_density;
    //    // this is magic, didn't wanted to have so much light in the center of sphere because color of planet was not visible
    //    c = c * atmosphere_density * clamp(smoothstep(0.0, atmosphere_radius, d * 1.9), 0.1, 1.0);
    //    attenuation = c;
    //}

    if (d > planet_radius)
        return convert_to_vec4(result * lambertian * light_power / distance);

    return vec4(result * lambertian * light_power / distance, clamp(attenuation, 0.0, 0.9));
}

void main()
{
    if (!is_point_in_circle())
        discard;

    vec3 result = vec3(0.);
    
    vec4 sphere = draw_sphere(planet_radius, ambient_color, diffuse_color, specular_color, light_power);

    result = mix(result, sphere.rgb, sphere.a);

    vec4 atmosphere = draw_atmosphere();

    float d = length(fs_position);

    if (d > planet_radius)
        FragColor = atmosphere;
    else
        FragColor = vec4(mix(result, atmosphere.rgb, atmosphere.a), 1.);
}

@end

@program planet planet_vs planet_fs