#version 330

uniform vec2 Resolution;
uniform sampler2D Main_Tex;

uniform vec2 R_Off, G_Off, B_Off;

void main() {
	const float ca=.12;

	vec2 uv=gl_FragCoord.xy/Resolution;
	float dst=ca*length(.5-uv)*sqrt(2.);
	gl_FragColor=vec4(
		texture(Main_Tex, uv+dst*R_Off).r,
		texture(Main_Tex, uv+dst*G_Off).g,
		texture(Main_Tex, uv+dst*B_Off).b,
		1
	);
}