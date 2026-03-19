#version 460 core

in vec2 fragment_uv;

layout(location = 0) out vec4 frag_color;

uniform sampler2D albedo_texture;

void main()
{
    frag_color = vec4(texture(albedo_texture, fragment_uv).xyz, 1.0);
}
