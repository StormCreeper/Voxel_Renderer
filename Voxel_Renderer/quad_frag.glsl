#version 430 core

out vec4 FragColor;
  
in vec2 fragPos;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;
uniform bool useSRGB;
uniform bool useACES;

uniform int u_FrameSinceLastReset;

float tseed = 0;
uint rngState = uint(uint(gl_FragCoord.x) * uint(19873) + uint(gl_FragCoord.y) * uint(92787) + uint(tseed * 100) * uint(26699) + u_FrameSinceLastReset) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}
float RandomFloat01(inout uint state) {
    return float(wang_hash(state)) / 4294967296.0;
}

vec3 LessThan(vec3 f, float value)
{
    return vec3(
        (f.x < value) ? 1.0f : 0.0f,
        (f.y < value) ? 1.0f : 0.0f,
        (f.z < value) ? 1.0f : 0.0f);
}
 
vec3 LinearToSRGB(vec3 rgb)
{
    rgb = clamp(rgb, 0.0f, 1.0f);
     
    return mix(
        pow(rgb, vec3(1.0f / 2.4f)) * 1.055f - 0.055f,
        rgb * 12.92f,
        LessThan(rgb, 0.0031308f)
    );
}
 
vec3 SRGBToLinear(vec3 rgb)
{
    rgb = clamp(rgb, 0.0f, 1.0f);
     
    return mix(
        pow(((rgb + 0.055f) / 1.055f), vec3(2.4f)),
        rgb / 12.92f,
        LessThan(rgb, 0.04045f)
    );
}
vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0f, 1.0f);
}

void main() {
    FragColor = texture(u_Texture, fragPos * 0.5 + 0.5);
    //FragColor *= (FragColor.x > 1.5 || FragColor.y > 1.5 || FragColor.z > 1.5) ? 1.0 : 0.0;

    if(useACES) FragColor.rgb = ACESFilm(FragColor.rgb * 0.5);
    if(useSRGB) FragColor.rgb = LinearToSRGB(FragColor.rgb);

    // add bloom 
    vec4 bloom = texture(u_BloomTexture, fragPos * 0.5 + 0.5);
    FragColor += bloom * 0.5;
}