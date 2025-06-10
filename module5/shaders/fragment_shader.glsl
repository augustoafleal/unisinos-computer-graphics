#version 460 core

in vec3 fragNormal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 cameraPos;

uniform sampler2D colorBuffer;

out vec4 color;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(cameraPos - fragPos);
    vec3 R = reflect(-L, N);

    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    vec3 ambient = ka * lightColor * 0.05;

    float diff = max(dot(N, L), 0.0);
    vec3 texColor = texture(colorBuffer, texCoord).rgb;
    vec3 diffuse = diff * kd * lightColor * texColor * attenuation * 3;

    vec3 specular = vec3(0.0);
    if (diff > 0.0)
    {
        float spec = pow(max(dot(R, V), 0.0), q);
        specular = spec * ks * lightColor * attenuation * 2.0;
    }
    vec3 result = ambient + diffuse + specular;

    color = vec4(result, 1.0);
}
