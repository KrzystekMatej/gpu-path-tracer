#version 460 core

in vec3 world_position;
in vec3 world_normal;
in vec4 world_tangent;
in vec2 fragment_uv;

layout(location = 0) out vec4 fragment_color;

uniform vec3 camera_position;
uniform sampler2D color_texture;
uniform sampler2D specular_texture;
uniform sampler2D shininess_texture;
uniform sampler2D normal_texture;

const int LIGHT_LIMIT = 10;
const float PI = 3.14159265359f;
const float MIN_SHININESS = 1.f;
const float MAX_SHININESS = 1000.f;
const vec3 AMBIENT_STRENGTH = vec3(0.1f);

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

vec3 phong(vec3 albedo, vec3 specular, float shininess, vec3 normal, vec3 fragment_position)
{
    vec3 result = vec3(0.0);
    vec3 view_direction = normalize(camera_position - fragment_position);

    for (uint i = 0u; i < light_count; ++i)
    {
        vec3 light_vector = lights[i].position - fragment_position;
        float distance_squared = max(dot(light_vector, light_vector), 1e-6);
        vec3 light_direction = light_vector * inversesqrt(distance_squared);
        vec3 reflect_direction = reflect(-light_direction, normal);

        float cos_theta = max(dot(normal, light_direction), 0.0);
        if (cos_theta <= 0.0) continue;
            
        float spec = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);

        float attenuation = lights[i].intensity / (4.0 * PI * distance_squared);

        vec3 diffuse_component = albedo * cos_theta * lights[i].color;
        vec3 specular_component = specular * spec * lights[i].color;

        result += (diffuse_component + specular_component) * attenuation;
    }

    vec3 ambient = AMBIENT_STRENGTH * albedo;
    return result + ambient;
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
    vec3 specular = texture(specular_texture, fragment_uv).rgb;
    float shininess = texture(shininess_texture, fragment_uv).r * (MAX_SHININESS - MIN_SHININESS) + MIN_SHININESS;
    vec3 normal = get_normal();

    vec3 color = phong(base_color, specular, shininess, normal, world_position);

    fragment_color = vec4(postprocess(color), 1.0);
}
