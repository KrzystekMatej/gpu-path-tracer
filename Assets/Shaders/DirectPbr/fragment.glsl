#version 460 core

in vec3 world_position;
in vec3 world_normal;
in vec4 world_tangent;
in vec2 fragment_uv;

layout(location = 0) out vec4 fragment_color;

uniform vec3 camera_position;
uniform float dielectric_F0;

uniform sampler2D color_texture;
uniform sampler2D rma_texture;
uniform sampler2D normal_texture;

const int LIGHT_LIMIT = 10;
const float PI = 3.14159265359;

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
};

uniform PointLight  lights[LIGHT_LIMIT];
uniform uint        light_count;

float distribution_ggx(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
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

void main()
{
    vec3 N = get_normal();
    vec3 V = normalize(camera_position - world_position);

    vec3 base_color = texture(color_texture, fragment_uv).rgb;
    vec3 rma = texture(rma_texture, fragment_uv).rgb;
    float roughness = rma.r;
    float metallic = rma.g;
    float ao = rma.b;
    
    vec3 F0 = mix(vec3(dielectric_F0), base_color, metallic);

    vec3 Lo = vec3(0.0);
    for (uint i = 0u; i < light_count; i++)
    {
        vec3 L_vec = lights[i].position - world_position;
        float d2 = max(dot(L_vec, L_vec), 1e-6);
        vec3 L = L_vec * inversesqrt(d2);
        vec3 H = normalize(V + L);

        float attenuation = 1.0 / d2;

        vec3 radiance = lights[i].color * (lights[i].intensity * attenuation);

        float NDF = distribution_ggx(N, H, roughness);
        float G   = geometry_smith(N, V, L, roughness);
        vec3  F   = fresnel_schlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-4;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * base_color / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * base_color * ao;
    vec3 color = ambient + Lo;

    fragment_color = vec4(postprocess(color), 1.0);
}
