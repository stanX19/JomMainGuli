#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec4 colDiffuse;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(cameraPosition - fragPosition);
    vec3 lightDir = normalize(lightPosition - fragPosition);

    // Glass and light reflection
    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularShine = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 highlights = lightColor * (specularShine + rim * 0.5);

    // Sphere's color
    vec3 baseColor = (colDiffuse * fragColor).rgb;

    // Trasnparency for glass
    float alpha = 0.4 + rim * 0.1;

    finalColor = vec4(baseColor + highlights, alpha);
}
