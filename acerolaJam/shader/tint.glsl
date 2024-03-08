#version 330

uniform vec2 Resolution;
uniform sampler2D Main_Tex;

uniform vec3 Tint;

void main() {
	vec2 uv=gl_FragCoord.xy/Resolution;
	gl_FragColor=mix(texture(Main_Tex, uv), vec4(Tint, 1), .5);
}