uniform float Size;
uniform float Time;
uniform vec2 Resolution;
uniform sampler2D MainTex;

#define ANIM_SPEED .125

float hash12(vec2 p){
	vec3 p3=fract(p.xyx*.1031);
	p3+=dot(p3, p3.yzx+33.33);
	return fract((p3.x+p3.y)*p3.z);
}

#define PI 3.1415927

float noise(vec2 st){
	vec2 i=floor(st);
	vec2 f=st-i;

	float a=hash12(i);
	float b=hash12(i+vec2(1, 0));
	float c=hash12(i+vec2(0, 1));
	float d=hash12(i+vec2(1));
	vec2 u=f*f*(3-2*f);

	return mix(a, b, u.x)+
			(c-a)*u.y*(1.-u.x)+
			(d-b)*u.x*u.y;
}

#define OCTAVES 4
float fbm(vec2 st){
	float value=0.;
	float amplitude=.5;
	float frequency=0.;
	for(int i=0;i<OCTAVES;i++){
		value+=amplitude*noise(st);
		st*=2;
		amplitude*=.5;
	}
	return value;
}

void main(){
	vec2 coord=gl_FragCoord.xy-.5*Resolution;
	vec2 uv=gl_FragCoord.xy/Resolution;
	float time=4*ANIM_SPEED*round(Time/ANIM_SPEED);
	
	//base blue
	gl_FragColor=vec4(.07, .3, .7, 1.);
	gl_FragColor+=.1-.2*fbm(8*uv);
	
	//wiggle
	float angle=2.*PI*fbm(uv*12.+time);
	float disp=.4+.5*fbm(time-4.*uv);
	coord+=disp*vec2(cos(angle), sin(angle));
	
	//white lines
	vec2 lines=abs(.5-fract(coord/Size));
	float val=smoothstep(.44, .5, max(lines.x, lines.y)+.015*hash12(coord));
	//every fifth line is brighter
	lines=abs(.5-fract(.2*coord/Size));
	val+=.67*smoothstep(.48, .5, max(lines.x, lines.y)+.005*hash12(coord));
	gl_FragColor=mix(gl_FragColor, vec4(.68, .7, 1, 1), val);
	
	//composite
	vec4 col=texture(MainTex, uv);
	gl_FragColor=mix(gl_FragColor, col, col.a);

	//vignette
	uv*=1.-uv.yx;
	gl_FragColor*=pow(24.*uv.x*uv.y, .17);
}