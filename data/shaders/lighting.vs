#version 330

#define     MAX_LIGHTS              160
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform vec4 colDiffuse;

// Output vertex attributes
out vec4 v_Color;
out vec2 fragTexCoord;

void main()
{
    vec3 fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    vec4 fragColor = vertexColor;
    vec3 fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
    fragTexCoord = vertexTexCoord;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    
    // Calculate the color per-vertex
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
    float attenuation;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
		attenuation = 1.0;
            }

            if (lights[i].type == LIGHT_POINT)
            {
		vec3 to_light = lights[i].position - fragPosition;
		float d = length(to_light);
                light = normalize(to_light);
		attenuation = clamp(1.0 / (d * d * 0.05), 0.0, 1.0);
            }

            float NdotL = max(dot(normal, light), 0.0) * attenuation;
            lightDot += lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
            specular += specCo * attenuation;
        }
    }

    v_Color = (colDiffuse + vec4(specular, 1.0)) * vec4(lightDot, 1.0);
    v_Color += ambient / 10.0;
}
