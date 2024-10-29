#version 450
#extension GL_ARB_separate_shader_objects : enable

// For debugging
#define USE_TESS_LEVEL_AS_COLOR 0

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// TODO: Declare fragment shader inputs
layout(location = 0) in vec2 inUV;
/** For Rendering Tessellation **/
layout(location = 1) in float inTessLevel; 
layout(location = 2) in float inMinTessLevel; 
layout(location = 3) in float inMaxTessLevel; 

layout(location = 0) out vec4 outColor;

void main() {
    // TODO: Compute fragment color
    // Define base colors for the grass
    /** Green **/
    // vec3 grassLightColor = vec3(0.4, 0.7, 0.3);  
    // vec3 grassDarkColor = vec3(0.2, 0.6, 0.2);   
    /** Salmon **/
    vec3 grassLightColor = vec3(251, 196, 171) / 255.0;
    vec3 grassDarkColor = vec3(240, 128, 128) / 255.0;  

    // Apply a vertical gradient to simulate lighting (slightly dark at the base)
    float gradient = smoothstep(0.2, 0.8, inUV.y);
    
    // Blend the base color with noise for subtle variation
    vec3 grassColor = mix(grassDarkColor, grassLightColor, gradient);

    #if USE_TESS_LEVEL_AS_COLOR
        float normalised_tess_level = (inTessLevel - inMinTessLevel) / (inMaxTessLevel - inMinTessLevel);
        outColor = vec4(normalised_tess_level);
    #else
        outColor = vec4(grassColor, 1.0);
    #endif  
}
