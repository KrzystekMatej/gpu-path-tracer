#version 460 core

layout(location = 0) in vec3 local_position;

uniform mat4 projection;
uniform mat4 view;

out vec3 tex_dir;

void main()
{
    tex_dir = local_position;
    mat4 view_rotation = mat4(mat3(view));
    vec4 clip = projection * view_rotation * vec4(local_position, 1.0);
    gl_Position = clip.xyww;
}
