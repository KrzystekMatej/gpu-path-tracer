#version 460 core

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec3 localNormal;
layout(location = 3) in vec2 texCoords;
layout(location = 4) in vec4 localTangent;
layout(location = 5) in uint materialIndex;

out vec3 worldPosition;
out vec3 worldNormal;
out vec2 fragTexCoords;
out vec4 worldTangent;
flat out uint fragMaterialIndex;

uniform mat4 pvmMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    gl_Position = pvmMatrix * vec4(localPosition, 1.f);
    vec4 worldPositionH = modelMatrix * vec4(localPosition, 1.f);
    worldPosition = worldPositionH.xyz / worldPositionH.w;
    worldNormal = normalize(normalMatrix * localNormal);
    worldTangent = vec4(normalize((modelMatrix * vec4(localTangent.xyz, 0)).xyz), localTangent.w);
    fragTexCoords = texCoords;
    fragMaterialIndex = materialIndex;
}