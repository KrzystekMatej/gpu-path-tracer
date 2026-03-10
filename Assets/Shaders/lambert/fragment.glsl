#version 460 core

in vec3 world_position;
in vec3 world_normal;
in vec4 world_tangent;
in vec2 fragment_uv;

layout(location = 0) out vec4 frag_color;

uniform sampler2D albedo_texture;
uniform sampler2D normal_texture;

const int LIGHT_LIMIT = 10;
const float PI = 3.14159265359;

struct PointLight
{
    vec3  position;
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

vec3 lambert(vec3 albedo, vec3 N, vec3 P)
{
    vec3 result = vec3(0.0);

    for (uint i = 0u; i < light_count; ++i)
    {
        vec3 L_vec = lights[i].position - P;
        float d2 = max(dot(L_vec, L_vec), 1e-6);
        vec3 L = L_vec * inversesqrt(d2);

        float n_dot_l = max(dot(N, L), 0.0);

        float attenuation = lights[i].intensity / (4.0 * PI * d2);

        result += albedo * n_dot_l * attenuation;
    }

    return result;
}

void main()
{
    vec3 albedo = texture(albedo_texture, fragment_uv).rgb;
    vec3 N = get_normal();

    vec3 color = lambert(albedo, N, world_position);

    frag_color = vec4(color, 1.0);
}