#version 460 core

in vec2 fragment_uv;

layout(location = 0) out vec4 fragment_color;

uniform sampler2D emission_texture;

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
    vec3 emission = texture(emission_texture, fragment_uv).rgb;
    fragment_color = vec4(postprocess(emission), 1.0);
}
