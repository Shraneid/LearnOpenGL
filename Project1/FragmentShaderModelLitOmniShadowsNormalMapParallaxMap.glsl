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
	bool casts_shadows;

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

uniform bool wireframeMode;

uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform samplerCube omniShadowMap;
uniform float far_plane;

uniform float parallax_strength;
uniform float parallax_max_layers;

uniform bool parallax_self_shadow;
uniform float parallax_self_shadow_exponent;

vec3 CalcPointLight(PointLight light, vec3 lightPos, vec3 tangentFragPos, vec3 tangentViewDir, vec2 texCoords);
vec2 ParallaxMapping(vec2 texCoords, vec3 tangentViewDir);

float OmniShadowCalculation(vec3 worldFragPos, vec3 worldLightPos);
float ParallaxShadow(vec3 tangentLightDir, vec2 texCoords);

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
		if (!wireframeMode)
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

	float nbOfLayers = mix(maxLayers, minLayers, abs(dot(vec3(0,0,1), viewDir)));
	float layerDepth = 1.0/nbOfLayers;
	float currentLayerDepth = 0.0;

	vec2 P = viewDir.xy / viewDir.z * parallax_strength;
	vec2 deltaTexCoords = P / nbOfLayers;

	vec2 currentTexCoords = texCoords;
	float parallax_sample = texture(material.texture_parallax1, texCoords).r;
	float currentDepthMapValue = parallax_sample;

	while (currentLayerDepth < currentDepthMapValue){
		currentLayerDepth += layerDepth;

		currentTexCoords -= deltaTexCoords;

		parallax_sample = texture(material.texture_parallax1, currentTexCoords).r;
		currentDepthMapValue = parallax_sample;
	}

	vec2 previousTexCoords = currentTexCoords + deltaTexCoords;

	float afterHitDepth = currentDepthMapValue - currentLayerDepth;
	float beforeHitDepth = texture(material.texture_parallax1, previousTexCoords).r - currentLayerDepth + layerDepth;

	float weight = afterHitDepth / (afterHitDepth - beforeHitDepth);

	vec2 finalSampleTexCoords = previousTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalSampleTexCoords;
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

	vec3 ambient = light.ambient * diffuse_sample;
	vec3 diffuse = light.diffuse * diff * diffuse_sample;
	vec3 specular = light.specular * spec * specular_sample;

	float distance = length(tangentLightPos - tangentFragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
								light.quadratic * pow(distance, 2));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float pointLightShadow = light.casts_shadows ? OmniShadowCalculation(fs_in.FragPos, light.position) : 0.0;
	
	float parallaxShadowMultiplier = parallax_self_shadow ? ParallaxShadow(lightDir, texCoords) : 1.0;

	vec3 litColor =  ambient + (1.0 - pointLightShadow) * parallaxShadowMultiplier * (diffuse + specular);

	return litColor;
}


float ParallaxShadow(vec3 lightDir, vec2 texCoords){
	float shadowMultiplier = 1;

	const float minLayers = 8.0;
	float maxLayers = parallax_max_layers;
	
	if (dot(vec3(0,0,1), lightDir) < 0)
		return 0.0;
		
	shadowMultiplier = 0;

	float numSamplesUnderSurface = 0;
	float nbOfLayers = mix(maxLayers, minLayers, abs(dot(vec3(0,0,1), lightDir)));
	
	float parallax_sample = texture(material.texture_parallax1, texCoords).r;
	float currentHeight = parallax_sample;

	float layerHeight = currentHeight / nbOfLayers;
	vec2 texDelta = parallax_strength * lightDir.xy / lightDir.z / nbOfLayers;

	int stepIndex = 1;

	while(currentHeight > 0){
		if (parallax_sample < currentHeight)  // we are under the mesh so a shadow is cast
			numSamplesUnderSurface += 1;

		texCoords += texDelta;
		parallax_sample = texture(material.texture_parallax1, texCoords).r;

		stepIndex += 1;
		currentHeight -= layerHeight;
	}

	return pow(1.0 - (numSamplesUnderSurface / nbOfLayers), parallax_self_shadow_exponent);
}


float OmniShadowCalculation(vec3 worldFragPos, vec3 worldLightPos){
	float shadow = 0.0;
	float bias = 0.05;
	int samples = 20;

	float viewDistance = length(viewPos - worldFragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
	
	vec3 lightToFrag = fs_in.FragPos - worldLightPos;
	float currentDepth = length(lightToFrag);
	
	for (int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(omniShadowMap, lightToFrag + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;
				
		if (currentDepth-bias > closestDepth)
		{
			shadow += 1.0;
		}
	}
	
	shadow /= float(samples);

	return shadow;
}
