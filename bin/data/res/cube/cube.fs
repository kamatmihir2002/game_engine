
const float PI = 3.14159;

float ndf(vec3 N, vec3 H, float a)
{
    float a2 = pow(a, 2.0);
    float ndoth = max(dot(N, H), 0.0);
    float ndoth2 = pow(ndoth, 2.0);
	
    float nom = a2;
    float denom = (ndoth2 * (a2 - 1.0) + 1.0);
    denom = PI * pow(denom, 2.0);
	
    return nom / denom;
}

float geometry(float ndv, float k)
{
    float nom = ndv;
    float denom = ndv * (1.0 - k) + k;
	
    return nom / denom;
}

float smith(vec3 N, vec3 V, vec3 L, float k)
{
    float ndv = max(dot(N, V), 0.0);
    float ndl = max(dot(N, L), 0.0);
    return geometry(ndv, k) * geometry(ndl, k);
}

vec3 fresnel(float cosTheta, vec4 albedo_color, float metallic)
{
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo_color.rgb, metallic);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float fm(float a, float b) {
    return (a > b)? a:b;
}

float k(float p) {
    return (p > 0)?p:-p;

}


vec2 pcf_offset(float x, float y) {
    return 1.0 / textureSize(shadow_map, 0) + vec2(x, y);
}

float ShadowCalculation(vec4 light_space_position)
{
    
    vec3 ls_coords = light_space_position.xyz / light_space_position.w;
    float cdepth = ls_coords.z;

    ls_coords = (ls_coords + vec3(1.0, 1.0, 1.0)) * 0.5;
    float closestDepth = texture(shadow_map, ls_coords.xy).r; 
    float currentDepth = ls_coords.z;
    
    
    float shadow = 0.0;
    float tdir = max(0.0, dot(object_normal.xyz, -light_direction));
    float sval = max(0.4, pow(tdir * 0.1 + (1.0 - tdir) * 1.0, 2.0)); 
    
    
    float bias = max(0.001 * (1.0 - dot(object_normal.xyz, -light_direction)), 0.007);
    float ofs = max(0.001, 0.01 * (currentDepth - closestDepth));
    vec2 size = pcf_offset(ofs, ofs);
    
    float pcfDepth = texture(shadow_map, ls_coords.xy + vec2(1.0, 0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(-1.0, 0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(0.0, 1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(0.0, -1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(1.0, -1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(-1.0, -1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(1.0, 1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;
    pcfDepth = texture(shadow_map, ls_coords.xy + vec2(-1.0, 1.0) * size).r; 
    shadow += (currentDepth - bias) > pcfDepth ? sval : 1.0;

    ofs = 0.002 * max(0.0, currentDepth - bias - closestDepth);

    shadow += ((currentDepth - bias) > (closestDepth))? sval:1.0;
    shadow /= 9.0;
    
    
    
    if (ls_coords.x < 0.0 || ls_coords.y < 0.0 || ls_coords.x > 1.0 || ls_coords.y > 1.0 || ls_coords.z > 1.0 || ls_coords.z < 0.0)
        shadow = 1.0;
    return shadow;//((closestDepth / 50.0) > current_depth)?1.0:0.2;
}  

vec3 blinnPhong(sampler2D tex) {
   vec3 L = -light_direction;
   vec3 V = camera_position - object_transformed_position.xyz;
   vec3 H = normalize(L + V);
   vec3 ldir = normalize(L);
   float att = 1.0;
   float NdotH = clamp(dot(normalize(object_normal), H), 0.0, 1.0);
   float p = 1.0 / att;
   vec4 k =  p * (max(dot(normalize(object_normal), ldir), 0.1) * (vec4(material_diffuse, 1.0) + texture(tex, object_texture_uvs)) + vec4(material_specular, 1.0) * pow(NdotH, material_shininess));
   return k;
}

void main()
{
    
    float object_metallicity = 0.1;
    float object_roughness = 3.0;
    vec4 cdiff = texture(material_albedo, object_texture_uvs);

    vec3 N = normalize(object_normal);
    vec3 V = normalize(camera_position - object_transformed_position.xyz);

    vec3 L = normalize(-light_direction);
    vec3 H = normalize(V + L);

    vec3 radiance = vec3(1.0, 1.0, 1.0);       

    float NDF = ndf(N, H, object_roughness);        
    float G   = smith(N, V, L, object_roughness);      
    vec3 F    = fresnel(max(dot(H, V), 0.1), vec4(material_diffuse, 1.0), object_metallicity);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - object_metallicity;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.1) * max(dot(N, L), 0.1) + 0.0001;
    vec3 specular     = numerator / denominator;  
        
    
    float NdotL = max(dot(N, L), 0.01);                
    vec3 Lo = (kD * vec3(cdiff) / PI + specular) * radiance * NdotL; 


    vec3 color = vec3(0.01) * vec3(cdiff) + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = ShadowCalculation(object_light_space_position) * vec4(color, 1.0);
 

    // vec3 wi = normalize(-light_direction);
    
    // vec3 N = normalize(object_normal);
    // vec3 V = normalize(camera_position - object_transformed_position.xyz);

    
    // float cosTheta    = max(dot(N, wi), 0.0);
    // vec3 k = blinnPhong(material_albedo);
    // FragColor = ShadowCalculation(object_light_space_position) * vec4(k.xyz, 1.0);
}
