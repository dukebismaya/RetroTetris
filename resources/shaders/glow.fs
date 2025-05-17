#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniform variables
uniform float glowIntensity = 0.6;    // Intensity of the glow effect (0.0 to 1.0)
uniform float glowSize = 0.15;        // Size of the glow effect
uniform float glowSpeed = 1.0;        // Speed of the glow pulsation
uniform float time;                   // Time in seconds (automatically updated by raylib)

void main()
{
    // Calculate distance from the center of the fragment
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(fragTexCoord, center);
    
    // Calculate glow intensity based on distance
    float glow = 1.0 - smoothstep(0.0, glowSize, dist);
    
    // Add pulsating effect to the glow
    float pulse = (sin(time * glowSpeed) * 0.2 + 0.8);
    glow *= pulse * glowIntensity;
    
    // Original fragment color
    vec4 baseColor = fragColor;
    
    // Mix original color with glow
    vec4 glowColor = vec4(baseColor.rgb, baseColor.a * glow);
    
    // Output final color
    finalColor = glowColor;
}