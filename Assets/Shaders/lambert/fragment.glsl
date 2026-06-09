#version 460 core

in vec3 world_position;
in vec3 world_normal;
in vec4 world_tangent;
in vec2 fragment_uv;

layout(location = 0) out vec4 fragment_color;

uniform sampler2D color_texture;
uniform sampler2D normal_texture;

const int LIGHT_LIMIT = 10;
const float PI = 3.14159265359;
const vec3 AMBIENT_STRENGTH = vec3(0.1);

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
};

uniform PointLight  lights[LIGHT_LIMIT];
uniform uint        light_count;

vec3 get_normal()
{
    vec3 N = normalize(world_normal);

    vec3 T = normalize(world_tangent.xyz);
    T = normalize(T - N * dot(N, T));

    vec3 B = cross(N, T) * world_tangent.w;

    mat3 tbn = mat3(T, B, N);

    vec3 tangent_normal = texture(normal_texture, fragment_uv).xyz * 2.0 - 1.0;
    N = normalize(tbn * tangent_normal);
    return N;
}

vec3 lambert(vec3 albedo, vec3 normal, vec3 fragment_position)
{
    vec3 result = vec3(0.0);

    for (uint i = 0u; i < light_count; ++i)
    {
        vec3 light_vector = lights[i].position - fragment_position;
        float distance_squared = max(dot(light_vector, light_vector), 1e-6);
        vec3 light_direction = light_vector * inversesqrt(distance_squared);

        float cos_theta = max(dot(normal, light_direction), 0.0);
        if (cos_theta <= 0.0) continue;

        float attenuation = lights[i].intensity / (4.0 * PI * distance_squared);

        result += albedo * lights[i].color * cos_theta * attenuation;
    }

    return result + (AMBIENT_STRENGTH * albedo);
}

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
    vec3 base_color = texture(color_texture, fragment_uv).rgb;
    vec3 normal = get_normal();

    vec3 color = lambert(base_color, normal, world_position);

    fragment_color = vec4(postprocess(color), 1.0);
}