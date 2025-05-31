#version 460 core

in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;

uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform vec3 cameraPos;

out vec4 color;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(cameraPos - fragPos);

    vec3 ambientTotal = vec3(0.0);
    vec3 diffuseTotal = vec3(0.0);
    vec3 specularTotal = vec3(0.0);

    for (int i = 0; i < 3; ++i)
    {
        vec3 lightDir = lightPos[i] - fragPos;
        float distance = length(lightDir);
        vec3 L = normalize(lightDir);

        float kc = 1.0;
        float kl = 0.09;
        float kq = 0.032;
        float attenuation = 1.0 / (kc + kl * distance + kq * distance * distance);

        vec3 ambient = lightColor[i] * ka;

        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * lightColor[i] * kd;

        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(R, V), 0.0), q);
        vec3 specular = spec * ks * lightColor[i];

        ambientTotal  += ambient;
        diffuseTotal  += diffuse * attenuation;
        specularTotal += specular * attenuation;
    }

    vec3 result = ambientTotal + diffuseTotal + specularTotal;
    color = vec4(result, 1.0f);
}
