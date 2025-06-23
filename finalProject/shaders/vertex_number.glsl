#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) in vec3 normal;

out vec2 texCoord;
out vec3 fragPos;
out vec3 fragNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = projection * view * worldPos;

    texCoord = vec2(tex_coord.x, 1.0 - tex_coord.y); // Corrige orientação se necessário
    fragPos = vec3(worldPos);
    fragNormal = mat3(transpose(inverse(model))) * normal;
}
