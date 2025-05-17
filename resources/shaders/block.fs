#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Uniform variables
uniform float breathSpeed = 1.0;      // Speed of the breathing effect
uniform float breathDepth = 0.2;      // Depth/amount of the breathing effect
uniform float highlightIntensity = 0.8; // Intensity of the highlight effect
uniform float time;                   // Time in seconds (automatically updated by raylib)

void main()
{
    // Base block color from fragment
    vec4 baseColor = fragColor;
    
    // Calculate breathing factor
    float breath = sin(time * breathSpeed) * breathDepth + 1.0;
    
    // Create a gradient based on position for more interesting effect
    float gradient = mix(0.8, 1.0, fragTexCoord.y);
    
    // Add a subtle moving highlight
    float highlight = pow(sin(fragTexCoord.x * 3.14159 + time * 0.5) * 0.5 + 0.5, 3.0) * highlightIntensity;
    highlight *= pow(sin(fragTexCoord.y * 3.14159) * 0.5 + 0.5, 2.0);
    
    // Add a beveled edge effect
    float bevel = 0.0;
    if (fragTexCoord.x < 0.1) bevel = smoothstep(0.0, 0.1, fragTexCoord.x) * 0.3;
    if (fragTexCoord.x > 0.9) bevel = smoothstep(1.0, 0.9, fragTexCoord.x) * 0.3;
    if (fragTexCoord.y < 0.1) bevel = max(bevel, smoothstep(0.0, 0.1, fragTexCoord.y) * 0.3);
    if (fragTexCoord.y > 0.9) bevel = max(bevel, smoothstep(1.0, 0.9, fragTexCoord.y) * 0.3);
    
    // Apply breathing effect to the color
    vec3 finalRGB = baseColor.rgb * breath * gradient + vec3(highlight) + vec3(bevel);
    
    // Output the final color with original alpha
    finalColor = vec4(finalRGB, baseColor.a);
}