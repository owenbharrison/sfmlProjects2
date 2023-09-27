uniform vec2 Resolution;
uniform sampler2D MainTex;

void main(){
	float sqSz=.5*min(Resolution.x, Resolution.y);

	vec2 ctr=.5*Resolution;
	vec2 pCtr=gl_FragCoord.xy-ctr;

	vec2 st=1-step(sqSz, abs(pCtr));
	if(st.x*st.y>0){
		vec2 uv=.5+.5*pCtr/sqSz;
		uv.y=1-uv.y;
		gl_FragColor=texture(MainTex, uv);
	} else gl_FragColor=vec4(vec3(.21), 1);
}