#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// TODO: Declare fragment shader inputs
layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    // TODO: Compute fragment color
    // Define base colors for the grass
    vec3 grassLightColor = vec3(0.4, 0.7, 0.3);  // Lighter green
    vec3 grassDarkColor = vec3(0.2, 0.6, 0.2);   // Darker green

    // Generate subtle noise based on UV, reduced in intensity
    float noise = fract(sin(dot(inUV, vec2(12.9898, 78.233))) * 43758.5453) * 0.1 + 0.9;

    // Apply a vertical gradient to simulate lighting (slightly dark at the base)
    float gradient = smoothstep(0.2, 0.8, inUV.y);
    vec3 baseColor = mix(grassDarkColor, grassLightColor, gradient);

    // Blend the base color with noise for subtle variation
    vec3 grassColor = baseColor * noise;

    outColor = vec4(grassColor, 1.0);
}
