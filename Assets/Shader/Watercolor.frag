// from shadertoy
#version 450

uniform mat4 viewMatrix;            //world coordinates to camera coordinate
uniform mat4 projectionMatrix;            //world coordinates to camera coordinate
uniform vec3 cameraPosition;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D uvs;
uniform sampler2D depth;

uniform sampler2D paper;
uniform sampler2D noise;
uniform sampler2D canvas;

uniform sampler2D textureDiffuse;
uniform sampler2D colorDiffuse;
uniform vec2 screenSize;

uniform bool textured;

uniform vec3 lightPosition = vec3(0, 10, 4); //light position in world coordinates
uniform vec4 lightColor = vec4(1, 1, 1, 1); //light color

out vec4 FragColor;


#define PI 3.14159265358979

vec4 getCol(vec2 pos)
{
    vec2 uv=pos/screenSize;
    vec4 c1 = texture(colorDiffuse,uv);
    vec4 c2 = vec4(.4); // gray on greenscreen
    float d = clamp(dot(c1.xyz,vec3(-0.5,1.0,-0.5)),0.0,1.0);
    return mix(c1,c2,1.8*d);
}

vec4 getCol2(vec2 pos)
{
    vec2 uv=pos/screenSize;
    vec4 c1 = texture(colorDiffuse,uv);
    vec4 c2 = vec4(1.5); // bright white on greenscreen
    float d = clamp(dot(c1.xyz,vec3(-0.5,1.0,-0.5)),0.0,1.0);
    return mix(c1,c2,1.8*d);
}

vec2 getGrad(vec2 pos,float delta)
{
    vec2 d=vec2(delta,0);
    return vec2(
        dot((getCol(pos+d.xy)-getCol(pos-d.xy)).xyz,vec3(.333)),
        dot((getCol(pos+d.yx)-getCol(pos-d.yx)).xyz,vec3(.333))
    )/delta;
}

vec2 getGrad2(vec2 pos,float delta)
{
    vec2 d=vec2(delta,0);
    return vec2(
        dot((getCol2(pos+d.xy)-getCol2(pos-d.xy)).xyz,vec3(.333)),
        dot((getCol2(pos+d.yx)-getCol2(pos-d.yx)).xyz,vec3(.333))
    )/delta;
}

vec4 getRand(vec2 pos) 
{
    vec2 uv=pos/screenSize;
    return texture(noise,uv);
}

float htPattern(vec2 pos)
{
    float p;
    float r=getRand(pos*.4/.7*1.).x;
  	p=clamp((pow(r+.3,2.)-.45),0.,1.);
    return p;
}

float getVal(vec2 pos, float level)
{
    return length(getCol(pos).xyz)+0.0001*length(pos-0.5*screenSize);
    return dot(getCol(pos).xyz,vec3(.333));
}
    
vec4 getBWDist(vec2 pos)
{
    return vec4(smoothstep(.9,1.1,getVal(pos,0.)*.9+htPattern(pos*.7)));
}

#define SampNum 24

#define N(a) (a.yx*vec2(1,-1))

void main(void)
{
    vec2 pos= ((gl_FragCoord.xy-screenSize.xy*.5)/screenSize.y*screenSize.y)+screenSize.xy*.5;
    vec2 pos2=pos;
    vec2 pos3=pos;
    vec2 pos4=pos;
    vec2 pos0=pos;
    vec3 col=vec3(0);
    vec3 col2=vec3(0);
    float cnt=0.0;
    float cnt2=0.;
    for(int i=0;i<1*SampNum;i++)
    {   
        // gradient for outlines (gray on green screen)
        vec2 gr =getGrad(pos, 2.0)+.0001*(getRand(pos ).xy-.5);
        vec2 gr2=getGrad(pos2,2.0)+.0001*(getRand(pos2).xy-.5);
        
        // gradient for wash effect (white on green screen)
        vec2 gr3=getGrad2(pos3,2.0)+.0001*(getRand(pos3).xy-.5);
        vec2 gr4=getGrad2(pos4,2.0)+.0001*(getRand(pos4).xy-.5);
        
        float grl=clamp(10.*length(gr),0.,1.);
        float gr2l=clamp(10.*length(gr2),0.,1.);

        // outlines:
        // stroke perpendicular to gradient
        pos +=.8 *normalize(N(gr));
        pos2-=.8 *normalize(N(gr2));
        float fact=1.-float(i)/float(SampNum);
        col+=fact*mix(vec3(1.2),getBWDist(pos).xyz*2.,grl);
        col+=fact*mix(vec3(1.2),getBWDist(pos2).xyz*2.,gr2l);
        
        // colors + wash effect on gradients:
        // color gets lost from dark areas
        pos3+=.25*normalize(gr3)+.5*(getRand(pos0*.07).xy-.5);
        // to bright areas
        pos4-=.5 *normalize(gr4)+.5*(getRand(pos0*.07).xy-.5);
        
        float f1=3.*fact;
        float f2=4.*(.7-fact); 
        col2+=f1*(getCol2(pos3).xyz+.25+.4*getRand(pos3*1.).xyz);
        col2+=f2*(getCol2(pos4).xyz+.25+.4*getRand(pos4*1.).xyz);
        
        cnt2+=f1+f2;
        cnt+=fact;
    }
    // normalize
    col/=cnt*2.5;
    col2/=cnt2*1.65;

    // outline + color
    col = clamp(clamp(col*.9+.1,0.,1.) * col2,0.,1.);
    // paper color and grain
    col = col*vec3(.93,0.93,0.85)
        * mix(texture(canvas, (gl_FragCoord.xy / screenSize)).xyz, vec3(1.2), 0.7)
        + 0.15*getRand(pos0*2.5).x; //+.015
    // vignetting
    float r = length((gl_FragCoord.xy-screenSize.xy*.5)/screenSize.x);
    float vign = 1.-r*r*r*r;

    // lighting
    vec4 vPosition = texture(positions, gl_FragCoord.xy / screenSize);
	vec3 position = vPosition.xyz;

	vec3 tNorm = texture(normals, gl_FragCoord.xy / screenSize).xyz;
	
	vec3 viewVec = normalize(cameraPosition.xyz - position);

	vec3 lightVec = normalize(lightPosition.xyz - position);

	vec3 reflecVec = reflect(-lightVec, tNorm);

    float spec = pow(max(dot(viewVec, reflecVec), 0.0), 32);
    vec3 specular = 0.5 * spec * lightColor.rgb;  
    
    float cosTheta = dot(tNorm, lightVec);
    float brightness = lightColor.z; // make it brighter

    // for specular highlight: FragColor = vec4(col * (cosTheta + brightness) + specular,1.0);
	FragColor = vec4(col * (cosTheta + brightness),1.0);
} 