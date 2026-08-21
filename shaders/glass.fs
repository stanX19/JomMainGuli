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
    vec3 halfDir = normalize(lightDir + viewDir);

    float normalDotView = max(dot(normal, viewDir), 0.0);
    float rim = 1.0 - normalDotView;

    // Darker near edges x^1
    float darkRim = pow(rim, 1.0);
    vec3 darkTint = (colDiffuse * fragColor).rgb;

    // Border white, x^4
    float borderLight = pow(rim, 4.0);
    vec3 rimColor = vec3(1.0) * borderLight;

    // Light glowing, mid^128 (Blinn-Phong specular glint)
    float normalDotHalf = max(dot(normal, halfDir), 0.0);
    float lightReflect = pow(normalDotHalf, 128.0);
    vec3 specularColor = lightColor * lightReflect;

    // Combine all
    vec3 finalRgb = darkTint + rimColor + specularColor;
    float alpha = clamp(darkRim * 0.45 + borderLight * 0.5 + lightReflect, 0.0, 1.0);

    finalColor = vec4(finalRgb, alpha);
}


