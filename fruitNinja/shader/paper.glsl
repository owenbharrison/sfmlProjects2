#version 330

uniform vec2 Resolution;
uniform sampler2D MainTex;

#define PI 3.1415927

#define K 7
#define SIGMA .6

float gaussian(vec2 pos){
		float left=1./(2.*PI*SIGMA*SIGMA);
		float right=exp(-dot(pos, pos)/(2.*SIGMA*SIGMA));
		return left*right;
}

float hash12(vec2 p){
	vec3 p3=fract(p.xyx*.1031);
	p3+=dot(p3, p3.yzx+33.33);
	return fract((p3.x+p3.y)*p3.z);
}

vec2 hash22(vec2 p){
	vec3 p3=fract(vec3(p.xyx)*vec3(.1031, .1030, .0973));
	p3+=dot(p3, p3.yzx+33.33);
	return fract((p3.xx+p3.yz)*p3.zy);
}

void main(){
	gl_FragColor=vec4(.85+.05*hash12(gl_FragCoord.xy));
	
	vec2 noiseCoord=gl_FragCoord.xy+hash22(gl_FragCoord.xy)*.5;
	vec2 uv=noiseCoord/Resolution;
	uv.y=1.-uv.y;
	vec4 sum=vec4(0.);
	for(int i=0;i<2*K+1;i++){
		for(int j=0;j<2*K+1;j++){
			vec2 offset=vec2(i-K, j-K);
			float weight=gaussian(offset);
				
			vec2 offsetUV=offset/Resolution;
			vec4 col=texture(MainTex, uv+offsetUV);
			sum+=vec4(col.rgb, 1.)*weight;
		}
	}
	gl_FragColor.rgb*=sum.rgb/sum.a;
	
	float sz=30.;
	float val=mod(gl_FragCoord.y, 30.);
	float ft=step(gl_FragCoord.y, Resolution.y-2.*sz);
	ft*=smoothstep(2.5, 0., val);
	gl_FragColor.rgb*=1.-vec3(ft, .6*ft, 0.);
	
	float fl=smoothstep(2.5, 0., abs(gl_FragCoord.x-3.*sz));
	gl_FragColor.rgb*=1.-vec3(0., .7*fl, .5*fl);
	
	uv*=1.-uv.yx;
	float vig=uv.x*uv.y*30.;
	gl_FragColor*=pow(vig, .12);
}