#version 330

uniform vec2 Resolution;
uniform sampler2D MainTex;

void main() {
	vec2 uv=gl_FragCoord.xy/Resolution;
	uv.y=1-uv.y;
	gl_FragColor=texture(MainTex, uv);
}