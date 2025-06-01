#version 400

in vec3 scaledNormal;
in vec2 texcoord;
in vec3 fragpos;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;
uniform vec3 lightColor[3];
uniform vec3 lightPos[3];
uniform vec3 cameraPos;

out vec4 color;

void main()
{
    vec3 N = normalize(scaledNormal);
    vec3 V = normalize(cameraPos - fragpos);

    vec3 ambientTotal = vec3(0.0);
    vec3 diffuseTotal = vec3(0.0);
    vec3 specularTotal = vec3(0.0);

    float k_c = 1.0;
    float k_l = 0.09;
    float k_q = 0.032;

    for (int i = 0; i < 3; ++i)
    {
        vec3 L = normalize(lightPos[i] - fragpos);
        vec3 R = reflect(-L, N);

        float distance = length(lightPos[i] - fragpos);
        float attenuation = 1.0 / (k_c + k_l * distance + k_q * distance * distance);

        ambientTotal += ka * lightColor[i];

        float diff = max(dot(N, L), 0.0);
        diffuseTotal += kd * diff * lightColor[i] * attenuation;

        float spec = pow(max(dot(R, V), 0.0), q);
        specularTotal += ks * spec * lightColor[i] * attenuation;
    }

    vec3 result = (ambientTotal + diffuseTotal)  + specularTotal;

    color = vec4(result, 1.0);
}
