#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_TESS_LEVEL 10.0
#define MIN_TESS_LEVEL 2.0
#define DISTANCE_FALL_OFF 50.0

layout(vertices = 1) out;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// TODO: Declare tessellation control shader inputs and outputs
layout(location = 0) in vec3 inV0[];
layout(location = 1) in vec3 inV1[];
layout(location = 2) in vec3 inV2[];
layout(location = 3) in vec3 inParams[];

layout(location = 0) out vec3 outV0[];
layout(location = 1) out vec3 outV1[];
layout(location = 2) out vec3 outV2[];
layout(location = 3) out vec3 outParams[];
layout(location = 4) out float outTessLevel[]; // For debugging
layout(location = 5) out float outMinTessLevel[]; // For debugging
layout(location = 6) out float outMaxTessLevel[]; // For debugging

void main() {
	// Don't move the origin location of the patch
    // TODO: Write any shader outputs
    outV0[gl_InvocationID] = inV0[gl_InvocationID];
    outV1[gl_InvocationID] = inV1[gl_InvocationID];
    outV2[gl_InvocationID] = inV2[gl_InvocationID];
    outParams[gl_InvocationID] = inParams[gl_InvocationID];
    /** For Rendering Tessellation **/
    outMinTessLevel[gl_InvocationID] = MIN_TESS_LEVEL; 
    outMaxTessLevel[gl_InvocationID] = MAX_TESS_LEVEL; 

	// TODO: Set level of tesselation
    // Calculate the distance from the blade to the camera
    vec3 bladePosition = inV0[gl_InvocationID]; // Use inV0 as the base position of the blade
    vec3 cameraPosition = vec3(inverse(camera.view)[3]); // Extract camera position from view matrix
    float distanceToCamera = distance(bladePosition, cameraPosition);
    float tessellationFactor = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, clamp(distanceToCamera / DISTANCE_FALL_OFF, 0.0, 1.0));

    gl_TessLevelInner[0] = tessellationFactor;
    gl_TessLevelInner[1] = tessellationFactor;
    gl_TessLevelOuter[0] = tessellationFactor;
    gl_TessLevelOuter[1] = tessellationFactor;
    gl_TessLevelOuter[2] = tessellationFactor;
    gl_TessLevelOuter[3] = tessellationFactor;
    /** For Rendering Tessellation **/
    outTessLevel[gl_InvocationID] = tessellationFactor;
}
