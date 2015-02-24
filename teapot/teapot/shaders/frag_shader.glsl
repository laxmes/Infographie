#version 120

varying vec3 normal;
varying vec3 eye;

void main(void) {
	vec3 n = normalize(normal);
	vec3 l = normalize(gl_LightSource[0].position.xyz);

	float diffuse  = .2 + clamp(dot(l, n), 0, 1);

	vec3 halfVector = normalize(l + eye);
	float specular = pow(max(0.0, dot(n, halfVector)), 50);
	
	vec3 specularColor = vec3(1, 1, 1);

	gl_FragColor = gl_Color * diffuse + vec4(specular * specularColor, 1);
	return;
}