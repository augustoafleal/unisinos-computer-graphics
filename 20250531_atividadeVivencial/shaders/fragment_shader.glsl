#version 460 core

in vec4 vertexColor;
in vec3 fragNormal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 ka;
uniform float kd;
uniform vec3 ks;
uniform float q;

//uniform vec3 lightPos;
//uniform vec3 lightColor;

uniform vec3 lightPos[3];
uniform vec3 lightColor[3];

uniform vec3 cameraPos;

uniform sampler2D colorBuffer;

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

    // Equation
    vec3 result = (ambient + diffuse) * texColor + specular;
    color = vec4(result, 1.0f);
}
