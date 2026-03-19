#version 460 core

in vec3 tex_dir;

out vec4 frag_color;

uniform samplerCube environment_map;

void main()
{
    vec3 color = texture(environment_map, tex_dir).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    frag_color = vec4(color, 1.0);
}
