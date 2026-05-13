#version 460 core

layout(location = 0) in vec3 local_position;
layout(location = 1) in vec3 local_normal;
layout(location = 2) in vec4 local_tangent;
layout(location = 3) in vec2 uv;

out vec2 fragment_uv;

uniform mat4 pvm_matrix;

void main(void)
{
    gl_Position = pvm_matrix * vec4(local_position, 1.f);
    fragment_uv = uv;
}


