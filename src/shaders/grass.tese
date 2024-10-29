#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(quads, equal_spacing, ccw) in;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// TODO: Declare tessellation evaluation shader inputs and outputs
layout(location = 0) in vec3 inV0[];
layout(location = 1) in vec3 inV1[];
layout(location = 2) in vec3 inV2[];
layout(location = 3) in vec3 inParams[];
/** For Rendering Tessellation **/
layout(location = 4) in float inTessLevel[]; 
layout(location = 5) in float inMinTessLevel[]; 
layout(location = 6) in float inMaxTessLevel[]; 

layout(location = 0) out vec2 outUV;
/** For Rendering Tessellation **/
layout(location = 1) out float outTessLevel; 
layout(location = 2) out float outMinTessLevel; 
layout(location = 3) out float outMaxTessLevel;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

	// TODO: Use u and v to parameterize along the grass blade and output positions for each vertex of the grass blade
    vec3 v0 = inV0[0];
    vec3 v1 = inV1[0];
    vec3 v2 = inV2[0];
    float o = inParams[0].x;
    vec3 t1 = vec3(cos(o), 0.0f, sin(o));
    float w = inParams[0].z;

    vec3 a = v0 + v * (v1 - v0);
    vec3 b = v1 + v * (v2 - v1);
    vec3 c = a + v * (b - a);
    vec3 c0 = c - w * t1;
    vec3 c1 = c + w * t1;
    vec3 t0 = normalize(b - a);
    vec3 n = normalize(cross(t0, t1));

    // for quad t = u
    // below is for triangle:
    float t = u + 0.5 * v - u * v;
    vec3 p = (1.0f - t) * c0 + t * c1;
    gl_Position = camera.proj * camera.view * vec4(p, 1.0);

    outUV = vec2(u, v);
    /** For Rendering Tessellation **/
    outTessLevel = inTessLevel[0];
    outMinTessLevel = inMinTessLevel[0];
    outMaxTessLevel = inMaxTessLevel[0];
}
