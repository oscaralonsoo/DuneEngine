#[vertex]
#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    TexCoord = aTexCoords;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormals, 0.0)));
    TBN = mat3(T, B, N);
}

#[fragment]
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in mat3 TBN;

uniform vec4 material_color;
uniform sampler2D material_albedoMap;
uniform sampler2D material_normalMap;
uniform sampler2D material_emissiveMap;
uniform sampler2D material_aoMap;
uniform int material_hasAlbedo;
uniform int material_hasNormal;
uniform int material_hasEmissive;
uniform int material_hasAO;

void main()
{
    vec4 color = material_color;
    if (material_hasAlbedo == 1)
    {
        color *= texture(material_albedoMap, TexCoord);
    }

    vec3 normal = vec3(0, 0, 1);
    if (material_hasNormal == 1)
    {
        normal = texture(material_normalMap, TexCoord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
    }

    if (material_hasEmissive == 1)
    {
        color += texture(material_emissiveMap, TexCoord);
    }

    if (material_hasAO == 1)
    {
        color *= texture(material_aoMap, TexCoord).r;
    }

    FragColor = color;
}
