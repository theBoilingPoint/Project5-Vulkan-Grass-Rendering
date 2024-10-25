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
    vec3 grassLightColor = vec3(0.5, 0.8, 0.3);  // Lighter green (top)
    vec3 grassDarkColor = vec3(0.1, 0.5, 0.1);   // Darker green (base)

    // Add color variation based on UV coordinates to simulate texture
    float noise = fract(sin(dot(inUV, vec2(12.9898, 78.233))) * 43758.5453);

    // Interpolate between light and dark colors based on noise
    vec3 grassColor = mix(grassDarkColor, grassLightColor, noise);

    // Optional: add a subtle vertical gradient based on the v-coordinate
    float gradient = smoothstep(0.0, 1.0, inUV.y);
    grassColor = mix(grassColor, grassLightColor, gradient * 0.5);

    // Optional: add more visual texture (noise-driven darker patches)
    float patchyNoise = fract(sin(dot(inUV * 10.0, vec2(91.0, 67.0))) * 89231.5453);
    grassColor -= vec3(patchyNoise * 0.1);

    outColor = vec4(grassColor, 1.0);
}
