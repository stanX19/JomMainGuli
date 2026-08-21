#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragNormal;
out vec3 fragPosition;
out vec2 fragTexCord;
out vec4 fragColor;

void main() {
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;
    
    fragNormal = normalize(mat3(matNormal) * vertexNormal);
    fragTexCord = vertexTexCord;
    fragColor = vertexColor;
    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

