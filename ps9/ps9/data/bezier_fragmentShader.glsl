//from GLSL :
varying vec4 vtx_values;

vec4 RainbowColor(float t)
{
    // t=0     R 0  G 0  B 1
    // t=0.25  R 0  G 1  B 1
    // t=0.5   R 0  G 1  B 0
    // t=0.75  R 1  G 1  B 0
    // t=1.0   R 1  G 0  B 0
    float g=2.0-abs(0.5-t)/0.25;
    float r=4.0*t-2.0;
    float b=2.0-4.0*t;
    return vec4(r,g,b,1);
}

void main()
{
    gl_FragColor = RainbowColor(vtx_values.y);
}
