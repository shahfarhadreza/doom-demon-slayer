#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;

out VS_OUT {
	vec2 textureCoordinate;
	vec3 normal; // World/Model space
} vs_out;

layout(std140, binding = 0) uniform CBPerFrame
{
    mat4 proj;
    mat4 proj_ortho;
    mat4 view;
    mat4 world;
    float width;
    float height;
} cbPerFrame;

layout(std140, binding = 1) uniform CBPerObject
{
    mat4 world;
    float opacity;
} cbPerObject;

void main() {

	mat4 modelView = cbPerFrame.view * cbPerObject.world;

    int spherical = 0; // 1 for spherical; 0 for cylindrical

    // First colunm.
    modelView[0][0] = 1.0; 
    modelView[0][1] = 0.0; 
    modelView[0][2] = 0.0; 

    if (spherical == 1)
    {
        // Second colunm.
        modelView[1][0] = 0.0; 
        modelView[1][1] = 1.0; 
        modelView[1][2] = 0.0; 
    }

    // Thrid colunm.
    modelView[2][0] = 0.0; 
    modelView[2][1] = 0.0; 
    modelView[2][2] = 1.0; 

	gl_Position = cbPerFrame.proj * modelView * vec4(position, 1.0);
	vs_out.textureCoordinate = textureCoord;
	vs_out.normal = normal;
}

