#version 460 core

in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;

out vec4 FragColor;

//uniform sampler2D texture_diffuse1; // ou "numberTexture"
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0);
uniform vec3 objectColor = vec3(0.8, 0.0, 0.0); // vermelho

void main()
{
    // Normal e vetores
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    // Difuso
    float diff = max(dot(norm, lightDir), 0.0);

    // Especular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // Combina
    vec3 ambient = 0.2 * objectColor;
    vec3 diffuse = 0.6 * diff * objectColor;
    vec3 specular = 0.3 * spec * lightColor;

    vec3 result = ambient + diffuse + specular;

 //   vec4 texColor = texture(texture_diffuse1, texCoord);
    FragColor = vec4(result, 1.0);// * texColor;
}
