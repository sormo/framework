void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy;

    uv -= .5;
    uv.x *= iResolution.x / iResolution.y;

    float d = length(uv);
    float r = 0.3;
    
    float c = smoothstep(r, r - 0.1, d);

    fragColor = vec4(vec3(c), 1.0);
}