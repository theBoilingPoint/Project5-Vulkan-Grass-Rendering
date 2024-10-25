#version 450
#extension GL_ARB_separate_shader_objects : enable

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

void main() {
	// Don't move the origin location of the patch
    // gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outV0[gl_InvocationID] = inV0[gl_InvocationID];
    outV1[gl_InvocationID] = inV1[gl_InvocationID];
    outV2[gl_InvocationID] = inV2[gl_InvocationID];
    outParams[gl_InvocationID] = inParams[gl_InvocationID];

	// TODO: Write any shader outputs

	// TODO: Set level of tesselation
    gl_TessLevelInner[0] = 2;
    gl_TessLevelInner[1] = 2;
    gl_TessLevelOuter[0] = 2;
    gl_TessLevelOuter[1] = 2;
    gl_TessLevelOuter[2] = 2;
    gl_TessLevelOuter[3] = 2;
}
