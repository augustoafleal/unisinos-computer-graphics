#version 460 core

in vec4 vertexColor;
in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 ka;
uniform float kd;
uniform vec3 ks;
uniform float q;

uniform vec3 lightPos_0;
uniform vec3 lightColor_0;

uniform vec3 lightPos_1;
uniform vec3 lightColor_1;

uniform vec3 lightPos_2;
uniform vec3 lightColor_2;

uniform vec3 cameraPos;

out vec4 color;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(cameraPos - fragPos);

    // --- Luz 0 ---
    vec3 lightDir0 = lightPos_0 - fragPos;
    float distance0 = length(lightDir0);
    vec3 L0 = normalize(lightDir0);

    float kc = 1.0;
    float kl = 0.09;
    float kq = 0.032;
    float attenuation0 = 1.0 / (kc + kl * distance0 + kq * distance0 * distance0);

    vec3 ambient0 = lightColor_0 * ka;
    float diff0 = max(dot(N, L0), 0.0);
    vec3 diffuse0 = diff0 * lightColor_0 * kd;
    vec3 R0 = reflect(-L0, N);
    float spec0 = pow(max(dot(R0, V), 0.0), q);
    vec3 specular0 = spec0 * ks * lightColor_0;

    // --- Luz 1 ---;
    vec3 lightDir1 = lightPos_1 - fragPos;
    float distance1 = length(lightDir1);
    vec3 L1 = normalize(lightDir1);

    float attenuation1 = 1.0 / (kc + kl * distance1 + kq * distance1 * distance1);

    vec3 ambient1 = lightColor_1 * ka;
    float diff1 = max(dot(N, L1), 0.0);
    vec3 diffuse1 = diff1 * lightColor_1 * kd;
    vec3 R1 = reflect(-L1, N);
    float spec1 = pow(max(dot(R1, V), 0.0), q);
    vec3 specular1 = spec1 * ks * lightColor_1;

    // --- Luz 2 ---
    vec3 lightDir2 = lightPos_2 - fragPos;
    float distance2 = length(lightDir2);
    vec3 L2 = normalize(lightDir2);

    float attenuation2 = 1.0 / (kc + kl * distance2 + kq * distance2 * distance2);

    vec3 ambient2 = lightColor_2 * ka;
    float diff2 = max(dot(N, L2), 0.0);
    vec3 diffuse2 = diff2 * lightColor_2 * kd;
    vec3 R2 = reflect(-L2, N);
    float spec2 = pow(max(dot(R2, V), 0.0), q);
    vec3 specular2 = spec2 * ks * lightColor_2;

    // Soma tudo
    attenuation0 = 1;
    attenuation1 = 1;
    attenuation2 = 1;
    vec3 ambientTotal = ambient0 + ambient1 + ambient2;
    vec3 diffuseTotal = diffuse0 * attenuation0 + diffuse1 * attenuation1 + diffuse2 * attenuation2;
    vec3 specularTotal = specular0 * attenuation0 + specular1 * attenuation1 + specular2 * attenuation2;

    vec3 baseColor = vertexColor.rgb;
    vec3 result = (ambientTotal + diffuseTotal) * baseColor + specularTotal;
    color = vec4(result, 1.0f);
}
