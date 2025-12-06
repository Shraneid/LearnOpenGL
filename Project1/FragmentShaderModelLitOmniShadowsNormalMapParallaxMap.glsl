#version 330 core

#define MAX_NR_LIGHTS 10

out vec4 FragColor;

in VS_OUT {
	vec3 Debug;
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	vec3[MAX_NR_LIGHTS] tangentPointLightPositions;
} fs_in;

struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLights[MAX_NR_LIGHTS];
uniform int NUMBER_OF_DIRECTIONAL_LIGHTS;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
uniform PointLight pointLights[MAX_NR_LIGHTS];
uniform int NUMBER_OF_POINT_LIGHTS;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	sampler2D texture_parallax1;
};
uniform Material material;

uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform samplerCube omniShadowMap;
uniform float far_plane;

uniform float parallax_strength;
uniform float parallax_max_layers;

vec3 CalcPointLight(PointLight light, vec3 lightPos, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

float OmniShadowCalculation(vec3 worldFragPos, vec3 worldLightPos);

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

void main()
{
	vec3 tangentViewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoords = ParallaxMapping(fs_in.TexCoords, tangentViewDir);

	if (texCoords.x > 1.0 || texCoords.x < 0.0 || texCoords.y > 1.0 || texCoords.y < 0.0){
		discard;
	}

	vec3 result = vec3(0.0);

	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++){
		result += CalcPointLight(
			pointLights[i], 
			fs_in.tangentPointLightPositions[i], 
			fs_in.TangentFragPos, 
			tangentViewDir,
			texCoords
		);
	}

	FragColor = vec4(result, 1.0f);
};


vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
	const float minLayers = 8.0;
	float maxLayers = parallax_max_layers;

	float nbOfLayers = mix(minLayers, maxLayers, abs(dot(vec3(0,0,1), viewDir)));
	float layerDepth = 1.0/nbOfLayers;
	float currentLayerDepth = 0.0;

	vec2 P = viewDir.xy / viewDir.z * parallax_strength;
	P.y = -P.y;
	vec2 deltaTexCoords = P / nbOfLayers;

	vec2 previousTexCoords = texCoords;
	vec2 currentTexCoords = texCoords;
	float parallax_sample = texture(material.texture_parallax1, texCoords).r;
	float currentDepthMapValue = parallax_sample;

	while (currentLayerDepth < currentDepthMapValue){
		currentLayerDepth += layerDepth;

		previousTexCoords = currentTexCoords;
		currentTexCoords -= deltaTexCoords;

		parallax_sample = texture(material.texture_parallax1, currentTexCoords).r;
		currentDepthMapValue = parallax_sample;
	}

	return currentTexCoords;
}


vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 tangentFragPos, vec3 tangentViewDir, vec2 texCoords) {
	vec3 diffuse_sample = vec3(texture(material.texture_diffuse1, texCoords));
	vec3 specular_sample = vec3(texture(material.texture_specular1, texCoords));
	vec3 normal_sample = vec3(texture(material.texture_normal1, texCoords));
	float parallax_sample = texture(material.texture_parallax1, texCoords).r;

	vec3 normal = normalize(normal_sample * 2.0 - 1.0);

	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(tangentViewDir, reflectDir), 0.0f), 200.0f);
	
	if (diff < 0.001)
		spec = 0;

	vec3 ambient = light.ambient * 0.5 * diffuse_sample;
	vec3 diffuse = light.diffuse * diff * diffuse_sample;
	vec3 specular = light.specular * spec * specular_sample;

	float distance = length(tangentLightPos - tangentFragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance, 2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float shadow = OmniShadowCalculation(fs_in.FragPos, light.position);

	vec3 litColor =  ambient + (1.0 - shadow) * (diffuse + specular);

	return litColor;
}


float OmniShadowCalculation(vec3 worldFragPos, vec3 worldLightPos){
	float shadow = 0.0;
	float bias = 0.05;
	int samples = 20;

	float viewDistance = length(viewPos - worldFragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
	
	vec3 fragToLight = fs_in.FragPos - worldLightPos;
	float currentDepth = length(fragToLight);
	
	for (int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(omniShadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;
				
		if (currentDepth-bias > closestDepth)
		{
			shadow += 1.0;
		}
	}
	
	shadow /= float(samples);

	return shadow;
}
