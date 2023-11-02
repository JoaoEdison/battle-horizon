#version 330

// Input vertex attributes
in vec4 v_Color;
in vec2 fragTexCoord;
// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    finalColor = v_Color * texelColor;
    
    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
