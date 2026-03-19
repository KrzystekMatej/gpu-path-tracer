#version 460 core

in vec3 world_normal;
in vec4 world_tangent;
in vec2 fragment_uv;

layout(location = 0) out vec4 frag_color;

uniform sampler2D normal_texture;


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
    frag_color = vec4(get_normal() * 0.5 + 0.5, 1.0);
}
