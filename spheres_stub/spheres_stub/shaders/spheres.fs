#version 120

uniform vec4 ogf_uniform_0;
varying vec3 center_position;
varying float radius;
varying vec3 eye;

void main(void) {
    float dist = distance(gl_FragCoord.xyz, center_position);
    
    vec3 frag_position = gl_FragCoord.xyz;
    //frag_position.z = ((gl_DepthRange.far - gl_DepthRange.near) / 2) * frag_position.z + (gl_DepthRange.far + gl_DepthRange.near) / 2;
    frag_position.z = sqrt(radius * radius - dist * dist);
    gl_FragDepth = center_position.z * eye.z + 1 / frag_position.z;

    if(dist > radius) discard;

    vec3 N;
    N = normalize(frag_position - center_position);    

    vec3 light_pos = gl_LightSource[0].position.xyz;
    float diffuse =  gl_LightSource[0].diffuse * dot(normalize(light_pos), N);
   
    vec3 halfVector = normalize(eye + light_pos);
    float spec = max(pow(dot(N, halfVector), 250), 0.); 

    gl_FragColor = vec4(gl_Color.rgb * diffuse + spec , 1);
}