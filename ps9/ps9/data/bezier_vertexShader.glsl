attribute vec2 param;
//from GLSL :
varying vec4 vtx_values;

uniform mat4 projection,modelView;
uniform vec3 ctp[16];

vec3 CubicBezierCurve(vec3 ctp0,vec3 ctp1,vec3 ctp2,vec3 ctp3,float s)
{
	vec3 a[3];
	a[0]=ctp0*(1.0-s)+ctp1*s;
    a[1]=ctp1*(1.0-s)+ctp2*s;
    a[2]=ctp2*(1.0-s)+ctp3*s;
	vec3 b[2];
	b[0]=a[0]*(1.0-s)+a[1]*s;
    b[1]=a[1]*(1.0-s)+a[2]*s;
	return b[0]*(1.0-s)+b[1]*s;
}
vec3 CubicBezierSurface(vec3 ctp[16],float s,float t)
{
	vec3 a[4];
    a[0]=CubicBezierCurve(ctp[ 0],ctp[ 1],ctp[ 2],ctp[ 3],s);
    a[1]=CubicBezierCurve(ctp[ 4],ctp[ 5],ctp[ 6],ctp[ 7],s);
    a[2]=CubicBezierCurve(ctp[ 8],ctp[ 9],ctp[10],ctp[11],s);
    a[3]=CubicBezierCurve(ctp[12],ctp[13],ctp[14],ctp[15],s);
	return CubicBezierCurve(a[0],a[1],a[2],a[3],t);
}

void main()
{
    vec3 vertex=CubicBezierSurface(ctp,param.x,param.y);
    vtx_values = vec4(vertex,1.0);
    gl_Position=projection*modelView*vec4(vertex,1.0);
}
