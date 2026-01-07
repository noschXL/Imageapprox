#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D targetTex;
uniform vec2 p0;
uniform vec2 p1;
uniform vec2 p2;
uniform vec2 resolution;

bool pointInTriangle(vec2 p, vec2 a, vec2 b, vec2 c)
{
    vec2 v0 = c - a;
    vec2 v1 = b - a;
    vec2 v2 = p - a;

    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0 - v - w;

    return (u >= 0.0) && (v >= 0.0) && (w >= 0.0);
}

void main()
{
    vec2 pixel = fragTexCoord * resolution;

    if (pointInTriangle(pixel, p0, p1, p2))
    {
        vec3 c = texture(targetTex, fragTexCoord).rgb;
        finalColor = vec4(c, 1.0); // RGB sum, A = count
    }
    else
    {
        finalColor = vec4(0.0);
    }
}
