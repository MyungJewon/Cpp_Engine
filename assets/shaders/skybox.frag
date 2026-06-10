#version 410 core
in vec3 vDir;
out vec4 FragColor;
uniform sampler2D uSkybox;
const float PI = 3.14159265359;
void main() {
    vec3 dir = normalize(vDir);
    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = 1.0 - (asin(clamp(dir.y, -1.0, 1.0)) / PI + 0.5);
    FragColor = texture(uSkybox, vec2(u, v));
}
