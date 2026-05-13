#version 460 core

in vec3 tex_dir;

out vec4 fragment_color;

uniform samplerCube environment_map;

vec3 reinhard(vec3 color)
{
    return color / (color + vec3(1.0));
}

vec3 linear_to_srgb(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));
}

vec3 postprocess(vec3 color)
{
    color = reinhard(color);
    color = linear_to_srgb(color);
    return color;
}


void main()
{
    vec3 color = texture(environment_map, tex_dir).rgb;
    fragment_color = vec4(postprocess(color), 1.0);
}
