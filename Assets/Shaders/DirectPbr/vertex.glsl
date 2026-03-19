#version 460 core

layout(location = 0) in vec3 local_position;
layout(location = 1) in vec3 local_normal;
layout(location = 2) in vec4 local_tangent;
layout(location = 3) in vec2 uv;

out vec3 world_position;
out vec3 world_normal;
out vec4 world_tangent;
out vec2 fragment_uv;

uniform mat4 pvm_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main(void)
{
    gl_Position = pvm_matrix * vec4(local_position, 1.f);
    vec4 world_position_h = model_matrix * vec4(local_position, 1.f);
    world_position = world_position_h.xyz / world_position_h.w;

    world_normal = normalize(normal_matrix * local_normal);
    
    vec3 tangent_xyz = mat3(model_matrix) * local_tangent.xyz;
    tangent_xyz = normalize(tangent_xyz);
    tangent_xyz = normalize(tangent_xyz - world_normal * dot(world_normal, tangent_xyz));
    world_tangent = vec4(tangent_xyz, local_tangent.w);

    fragment_uv = uv;
}
