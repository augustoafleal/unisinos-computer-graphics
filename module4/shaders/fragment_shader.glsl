#version 460 core

in vec4 vertexColor;        // From vertex shader
in vec3 fragNormal;         // From vertex shader
in vec3 fragPos;            // From vertex shader
in vec2 texCoord;           // From vertex shader

// Material properties
uniform vec3 ka;
uniform float kd;
uniform vec3 ks;
uniform float q;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightColor;

// Camera position
uniform vec3 cameraPos;

// Texture sampler
uniform sampler2D colorBuffer;

// Output
out vec4 color;

void main()
{
    // Ambient
    vec3 ambient = lightColor * ka;

    // Diffuse
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * kd;

    // Specular
    vec3 R = reflect(-L, N);
    vec3 V = normalize(cameraPos - fragPos);
    float spec = pow(max(dot(R, V), 0.0), q);
    vec3 specular = spec * ks * lightColor;

    // Texture color
    vec3 texColor = texture(colorBuffer, texCoord).rgb;

    // Combine
    vec3 result = (ambient + diffuse) * texColor + specular;
    color = vec4(result, 1.0f);
}
