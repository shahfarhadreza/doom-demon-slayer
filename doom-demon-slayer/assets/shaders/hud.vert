#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;

out VS_OUT {
	vec2 textureCoordinate;
} vs_out;

layout(std140, binding = 0) uniform CBPerFrame
{
    mat4 proj;
    mat4 proj_ortho;
    mat4 view;
    float width;
    float height;
} cbPerFrame;

void main() {
	//vec4(position.xy * vec2(2.0/cbPerFrame.width,2.0/cbPerFrame.height) - vec2(1,1), 0.0, 1.0);

	gl_Position = cbPerFrame.proj_ortho * vec4(position.xyz, 1.0);

	vs_out.textureCoordinate = textureCoord;
}

