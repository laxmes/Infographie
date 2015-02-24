#version 120

uniform vec4 ogf_uniform_0; // viewport
attribute float attr;
varying vec3 center_position;
varying float radius;
varying vec3 eye;

void main() {
	gl_FrontColor = gl_Color;
	gl_PointSize = attr * 700;
	gl_Position = gl_ModelViewMatrix * gl_Vertex;

	eye = normalize(-gl_Position.xyz);

	gl_Position = gl_ProjectionMatrix * gl_Position;

	radius = gl_PointSize / 2;

	center_position    = (gl_Position.xyz / gl_Position.w);
	center_position.xy = ((ogf_uniform_0.zw / 2) * center_position.xy) + ogf_uniform_0.xy + ogf_uniform_0.zw /2;
	center_position.z  = (gl_DepthRange.diff / 2) * center_position.z + (gl_DepthRange.far + gl_DepthRange.near) / 2;
}