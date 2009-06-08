// 
// Per-pixel phong shading
// Vertex shader (GLSL)
// 

varying vec3 vNormal;
varying vec3 vPos;
varying vec4 vColor;

void main()
{
	// Vertex normal
	vNormal = normalize(gl_NormalMatrix * gl_Normal);

	// Vertex position in eye coordinates
	vPos = vec3(gl_ModelViewMatrix * gl_Vertex);

	// Vertex color (used for the ambient and diffuse terms)
	vColor = gl_Color;

	// Texture coordinate
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// Vertex position in screen coordinates
	gl_Position = ftransform();
}
