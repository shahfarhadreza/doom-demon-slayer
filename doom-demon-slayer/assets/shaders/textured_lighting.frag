#version 420 core

layout(binding = 0) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 textureCoordinate;
    vec3 normal; // World/Model space
} fs_in;

void main() {
    
    vec4 albedo_color = texture(texture_sampler, fs_in.textureCoordinate).rgba;

    //if (albedo_color.a < 0.01)
	    //discard;

    vec3 lightColor = vec3(0.8, 0.7, 0.7);
    vec3 lightDir = vec3(0.8, 0.7, -0.8);

    float diff = max(dot(fs_in.normal, lightDir), 0.0) * 1.1;

    vec3 ambient = vec3(0.2, 0.2, 0.2);

    vec3 diffuse = diff * lightColor;

    vec3 color = (albedo_color.rgb * diffuse) + (ambient * albedo_color.rgb);

    fragColor = vec4(color, 1);
}


