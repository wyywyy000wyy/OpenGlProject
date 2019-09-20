#version 330 core
#define PI 3.1415926538

float PseudoRandom(vec2 xy)
{
	vec2 pos = fract(xy / 128.0f) * 128.0f + vec2(-64.340622f, -72.465622f);

	// found by experimentation
	return fract(dot(pos.xyx * pos.xyy, vec3(20.390625f, 60.703125f, 2.4281209f)));
}

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 WorldNormal;

uniform vec3 gLightPos;
uniform vec3 gLightColor;
uniform float gIrradiance = 1.0;
vec3 gLightDir = vec3(0,1.0f, 0);
float angle = 0.7f;
float InvRadius = 0.01f;
uniform vec3 gCameraPos;

uniform sampler2D gTexture;

uniform sampler2D gNormal;

uniform sampler2D gRoughness;

uniform sampler2D gMetallic;


vec3 GetNormal()
{
	//vec3 normal = texture(gNormal, TexCoords).xyz;
	vec3 tangentNormal = texture(gNormal, TexCoords).xyz * 2.0 - 1.0;
	vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);
	vec3 T = normalize(Q1*st2.t - Q2*st1.t);
	vec3 N = normalize(WorldNormal);

	vec3 B = normalize(cross(N, T));

	mat3 TBN = mat3(T,B, N);

	return normalize(TBN * tangentNormal);
	//return WorldNormal;// 
}

//SchlickFresnelReflectance
vec3 SchlickReflect(vec3 n, vec3 l, vec3 albedo, float metallic)
{
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	//return vec3(dot(n, l));
	//return F0 + (1 - F0)* pow((1 - max(dot(n, N),0)), 5);
	return F0 + (1 - F0)* pow((1 - max(dot(n, l),0)), 5);
}

vec3 SchlickReflectF90(vec3 n, vec3 l, vec3 albedo, float metallic, vec3 F90, float sharpness)
{
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	return F0 + (F90 - F0) * pow((1 - max(dot(n, l),0)), 1/sharpness);
}

//¦Õ
float CalcPhi(vec3 l, vec3 v, vec3 n)
{
	vec3 pl = n * dot(l, n);
	vec3 pv = n * dot(v, n);

	return dot(normalize(pl), normalize(pv));
}

//¦Ë(¦Õ) 1
float GaussianLambda(vec3 l, vec3 v, vec3 n)
{
	float phi = CalcPhi(l,v,n);
	return 1 - exp(-7.3 * pow(phi, 2));
}
float GinnekenLambda(vec3 l, vec3 v, vec3 n)
{
	float phi = CalcPhi(l,v,n) * 4.41;
	return phi / phi + 1;
}

float LambdaFuncForBeckmann( vec3 s,vec3 n , float roughness)
{
	float NoS = dot(n, s);
	float a = NoS / (roughness * sqrt(1 - pow(NoS, 2)));

	float a2 = pow(a, 2);

	float result = (1 - 1.259 * a + 0.396 * a2) / (3.535 * a + 2.181 * a2);

	return step(1.6, a) * result;
}

// ¦« (lambda) function
float LambdaFunc(vec3 v, vec3 n, float roughness)
{
	return 1;
}

//SmithFunction G2
float SmithFunction(vec3 l, vec3 v, vec3 m, float roughness)
{
	float p1 = max(dot(m, v),0);
	float p2 = 1 + LambdaFunc(v, m, roughness);

	float p3 = max(dot(m, l), 0);
	float p4 = 1 + LambdaFunc(l, m, roughness);

	return (p1/p2)*(p3/p4);
}

//Smith height-correlated masking-shadowing function
float SmithFunctionHeightCorrelated(vec3 l, vec3 v, vec3 m, float roughness)
{
	float p1 = max(dot(m, v), 0);
	float p2 = LambdaFunc(v, m, roughness);

	float p3 = max(dot(m, l), 0);
	float p4 = LambdaFunc(l, m, roughness);
	return p1 * p3 / (1 + p2 + p4);
}

//Smith G2 that combines direction and height correlation
float SmithFunction2(vec3 l, vec3 v, vec3 m, float roughness)
{
	float p1 = max(dot(m, v), 0);
	float p2 = LambdaFunc(v, m, roughness);

	float p3 = max(dot(m, l), 0);
	float p4 = LambdaFunc(l, m, roughness);

	float p5 = GinnekenLambda(l,v,m);

	return p1 * p3 / (1 + max(p2, p4) + p5*min(p2, p4));
}

float BeckmannNDF(vec3 n, vec3 m, float roughness)
{
	float NoM = dot(n, m);
	float NoM2 =  pow(NoM, 2);
	float R2 = pow(roughness, 2);

	float p1 = step(0, NoM);
	float p2 = PI * R2 * pow(NoM, 4);
	float p3 = NoM2 - 1;
	float p4 = R2 * NoM2;

	return pow(p1/p2, p3/p4);
}

float BlinnPhongNDF(vec3 n, vec3 m, float ap)
{
	float NoM = dot(n, m);
	float ap2 = pow(ap, 2);

	float p1 = max(NoM, 0) * ap2;
	
	return max(NoM, 0) * (ap + 2) / (2 * PI) * pow(NoM, ap);
}

float GGXNDF(vec3 n, vec3 m, float ag)
{
	float NoM = dot(n, m);
	float ag2 = pow(ag, 2);

	float p1 = max(NoM, 0) * ag2;
	float p2 = PI * pow(1 + pow(NoM, 2) * (ag2 - 1), 2);

	return p1 / p2;
}

void CookTorrance()
{
	vec3 n = GetNormal();
	vec3 v = normalize(gCameraPos - WorldPos);
	vec3 l = normalize(gLightPos - WorldPos);
	vec3 h = normalize(v + l);
	vec3 albedo = texture(gTexture, TexCoords).xyz;
	vec3 m ;
	float roughness = texture(gRoughness, TexCoords).r;
	float metallic = texture(gMetallic, TexCoords).r;

	float N = BeckmannNDF(n, h, roughness);
	float G = SmithFunction2(l, v, h, roughness);
	vec3 F = SchlickReflect(h, l, albedo, metallic);

	float nol = dot(n, l);
	float nov = dot(n, v);

	vec3 spec = F * G * N / ( 4 * abs(nol) * abs(nov) );
}

void BlinnPhongBRDF()
{
	vec3 n = GetNormal();
	vec3 v = normalize(gCameraPos - WorldPos);
	vec3 l = normalize(gLightPos - WorldPos);
	vec3 h = normalize(v + l);
	vec3 albedo = texture(gTexture, TexCoords).xyz;
	vec3 m;
	float roughness = texture(gRoughness, TexCoords).r;
	float ap = 2 * pow(roughness, -2) - 2;
	float metallic = texture(gMetallic, TexCoords).r;

	float N = BeckmannNDF(n, h, ap);
	float G = SmithFunction2(l, v, h, ap);
	vec3 F = SchlickReflect(h, l, albedo, metallic);

	float nol = dot(n, l);
	float nov = dot(n, v);

	vec3 spec = F * G * N / (4 * abs(nol) * abs(nov));

}

float kFacing(vec3 l, vec3 v)
{
	return 0.5 + 0.5 * dot(l, v);
}

//OrenNayar (Lambertian micro-BRDF, Gaussian NDF, Torrance-Sparrow "V-cavity" mask-shadowing function)

//Disney Diffuse Model
void BurleyDiffuse()
{
	vec3 n = GetNormal();
	vec3 v = normalize(gCameraPos - WorldPos);
	vec3 l = normalize(gLightPos - WorldPos);
	vec3 h = normalize(v + l);
	vec3 albedo = texture(gTexture, TexCoords).xyz;

	vec3 m;
	float roughness = texture(gRoughness, TexCoords).r;
	float metallic = texture(gMetallic, TexCoords).r;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	float nol = dot(n, l);
	float NOL = max(nol, 0);
	float nov = dot(n, v);
	float NOV = max(nov, 0);
	float hol = dot(h, l);

	float a = 0;// sepcular roughness 
	float kss = 0; // user controlled
	float sqrtA = sqrt(a);
	

	float FSS90 = sqrtA * hol * hol;
	float FSS = (1 + (FSS90 - 1) * pow((1 - hol), 5)) / (1 + (FSS90 - 1) * pow((1 - nov), 5));
	float fss = (1 / (nol * nov) - 0.5) * FSS + 0.5;
	float FD90 = 0.5 + 2 * sqrtA*hol*hol;
	float fd = (1 + (FD90 - 1) * pow((1 - nol), 5)) * (1 + (FD90 - 1) * pow((1 - nov), 5));

	vec3 fDiff = NOL * NOV * albedo / PI * ((1 - kss) * fd + 1.25 * kss * fss);
}

//HammonDiffuse
void HammonDiffuse()
{

	vec3 n = GetNormal();
	vec3 v = normalize(gCameraPos - WorldPos);
	vec3 l = normalize(gLightPos - WorldPos);
	vec3 h = normalize(v + l);
	vec3 albedo = texture(gTexture, TexCoords).xyz;

	vec3 m;
	float roughness = texture(gRoughness, TexCoords).r;
	float metallic = texture(gMetallic, TexCoords).r;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	float nol = max(dot(n, l), 0);
	float nov = max(dot(n, v), 0);

	float fMulti = 0.3641 * roughness;
	float kFacing = 0.5 + 0.5 * dot(l, v);
	float noh = dot(n, h);
	float fCough = kFacing * (0.9 - 0.4*kFacing) * (0.5 + noh) / noh;

	float fSmooth = 21 / 20 * (1 - F0.r) *
		(1 - pow((1 - dot(n, l)), 5))*
		(1 - pow((1 - dot(n, v)), 5));

	vec3 diff = nol * nov * albedo / PI * ((1 - roughness) * fSmooth + roughness * fCough + albedo * fMulti);

}

vec3 GGX(vec3 l)
{
	vec3 n = GetNormal();
	vec3 v = normalize(gCameraPos - WorldPos);
	//vec3 l = normalize(gLightPos - WorldPos);
	vec3 h = normalize(v + l);
	vec3 albedo = texture(gTexture, TexCoords).xyz;

	vec3 m;
	float roughness = texture(gRoughness, TexCoords).r;
	float metallic = texture(gMetallic, TexCoords).r;
	metallic = 1.0f;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	float nol = max(dot(n, l), 0);
	float nov = max(dot(n, v), 0);

	float N = GGXNDF(n, h, roughness);
	vec3 F = SchlickReflect(h, l, albedo, metallic);
	float G = 0.5 / mix(2 * nol * nov,nol + nov, roughness);

	vec3 spec = F * G * N;

	float fMulti = 0.3641 * roughness;
	float kFacing = 0.5 + 0.5 * dot(l, v);
	float noh = dot(n, h);
	float fCough = kFacing * (0.9 - 0.4*kFacing) * (0.5 + noh) / noh;

	float fSmooth = 21 / 20 * (1 - F0.r) * 
		(1 - pow((1 - dot(n, l)), 5))*
		(1 - pow((1 - dot(n, v)), 5));

	vec3 diff = nol * nov * albedo / PI * ((1 - roughness) * fSmooth + roughness * fCough + albedo * fMulti);

	vec3 color = (diff + spec) * gLightColor * 1 *  nol * gIrradiance;
	//color = color / (color + vec3(1.0));
	//color = pow(color, vec3(1.0 / 2.2));
	
	//FragColor = vec4(color, 1.0);

	return color;
	//FragColor = vec4(color, 1.0);
}

vec3 fDiff(float F0, vec3 albedo, vec3 n, vec3 l, vec3 v)
{
	return 21 / (20 * PI) * (1 - F0) * albedo * (1 - pow(1 - max(dot(n, l), 0), 5) * (1 - pow(1 - max(dot(n, v), 0), 5)));
}



vec3 fDiff_KelemenAndSzirmayKalos(float F0, vec3 albedo, vec3 n, vec3 l, vec3 v)
{
	vec3 RspecL = albedo / PI * dot(n, v);
	vec3 RspecV = albedo / PI * dot(n, l);
	return RspecL;
}

void main()
{             
    
	vec3 normal = GetNormal();
	vec3 LightDir = gLightPos - WorldPos;

	float pss = max(dot(gLightDir, normal), 0.0);

	vec3 color = texture(gTexture, TexCoords).rgb;
	//color = color * pss;

	//FragColor = vec4(color, 1.0);

	vec3 color1 = GGX(gLightDir);
	float weight = dot(gLightDir, normalize( LightDir));
	float LightMask = pow(clamp(1 - dot(LightDir, LightDir) * InvRadius * InvRadius, 0.0f, 1.0f), 2);

	vec3 ambient = color * 0.3;

	LightMask *= (pow(clamp(weight - angle * angle, 0.0f, 1.0f), 2));

	//color = color / (color + vec3(1.0));
	vec3 finalColor = color1 * LightMask;
	finalColor = finalColor / (finalColor + vec3(1.0));
	finalColor = finalColor;

	FragColor = vec4(color, 1.0);
	FragColor = vec4(finalColor, 1.0);
}
