// 
// Per-pixel phong shading
// Fragment shader (GLSL)
// 

varying vec3 vNormal;
varying vec3 vPos;
varying vec4 vColor;

void main()
{
	vec3 n = normalize(vNormal);

	// Ambient term
	vec4 ambient = vColor * gl_LightSource[0].ambient;

	// Diffuse term
	vec3 lightDir = normalize(gl_LightSource[0].position.xyz - vPos);
	float NdotL = abs(dot(n, lightDir));
	vec4 diffuse = vColor * gl_LightSource[0].diffuse * NdotL;

	// Specular term
	vec3 rVector = normalize(2.0 * n * dot(n, lightDir) - lightDir);
	vec3 viewVector = normalize(-vPos);
	float RdotV = dot(rVector, viewVector);
	vec4 specular = vec4(0.0);
	if(RdotV > 0.0)
		specular = vec4(0.4, 0.4, 0.4, 1.0) * gl_LightSource[0].specular * pow(RdotV, 20.0);

	gl_FragColor = ambient + diffuse + specular;
}
