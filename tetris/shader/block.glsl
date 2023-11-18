#version 330

#define SQRT2 1.4142136

uniform vec2 Resolution;
uniform sampler2D MainTex;

void main() {
	vec2 uv=gl_FragCoord.xy/Resolution;
	uv.y=1-uv.y;

	vec2 texSize=vec2(textureSize(MainTex, 0));
	float dist=length(.5-fract(uv*texSize))*SQRT2;
	float edge=smoothstep(dist, 1., .93);

	gl_FragColor=edge*texture(MainTex, uv);
}