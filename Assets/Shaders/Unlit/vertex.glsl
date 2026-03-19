#version 460 core

layout(location = 0) in vec3 local_position;
layout(location = 3) in vec2 uv;

out vec3 world_position;
out vec2 fragment_uv;

uniform mat4 pvm_matrix;
uniform mat4 model_matrix;

void main(void)
{
    gl_Position = pvm_matrix * vec4(local_position, 1.f);
    vec4 world_position_h = model_matrix * vec4(local_position, 1.f);
    world_position = world_position_h.xyz / world_position_h.w;

    fragment_uv = uv;
}