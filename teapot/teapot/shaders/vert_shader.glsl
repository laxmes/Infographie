#version 120

varying vec3 normal;
varying vec3 eye;

void main() {
	normal = gl_NormalMatrix * gl_Normal;
	gl_Position = gl_ModelViewMatrix * gl_Vertex;

	eye = normalize(-gl_Position.xyz);

	gl_Position = gl_ProjectionMatrix * gl_Position;

	gl_FrontColor = gl_Color;
}