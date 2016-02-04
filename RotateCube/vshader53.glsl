/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpvenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec3 vPosition;
//in  vec4 vColor;
in  vec3 vNormal;

out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 globalAmbientProduct, AmbientProduct, DiffuseProduct, SpecularProduct;

uniform vec4 lightSourceAmbientProduct, lightSourceDiffuseProduct, lightSourceSpecularProduct;

uniform vec4 Towards;

uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform vec4 LightDirection;
uniform float Shininess;

uniform int flag;

vec4 vPosition4;


#define M_PI 3.14159265358979323846   // pi

vec4 distant_light_source() 
{
	vec3 pos = (model_view * vPosition4).xyz;

	vec3 L = normalize(-LightDirection); // l: from pt p to light light source
	vec3 E = normalize( -pos ); // v: direction from p to the viewer
	vec3 H = normalize( L + E ); // l + v

	vec3 N = normalize(Normal_Matrix * vNormal);

    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;

	

	float attenuation;


	attenuation = 1;

	vec4 ambient = AmbientProduct;

	float d = max( dot(L, N), 0.0 );
	vec4  diffuse = d * DiffuseProduct;

	float s = pow( max(dot(N, H), 0.0), Shininess );
	vec4  specular = s * SpecularProduct;

	if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}


    vec4 d_color = attenuation * (ambient + diffuse + specular);

	return d_color;
}

float distance(vec3 pt1, vec3 pt2) {
	vec3 subtract = pt2 - pt1;

	return sqrt (subtract.x * subtract.x + subtract.y * subtract.y + subtract.z * subtract.z);
}

vec4 point_source_source() 
{
	vec3 pos = (model_view * vPosition4).xyz;

	vec3 L = normalize( LightPosition.xyz - pos );
	vec3 E = normalize( -pos );
	vec3 H = normalize( L + E );

	vec3 N = normalize(Normal_Matrix * vNormal);

    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;

	float dist = distance(LightPosition.xyz, pos );

	float a = 2.0;
	float b = 0.01; 
	float c = 0.001;

	float attenuation = 1 / (a + b * dist + c * dist * dist);

	vec4 ambient = lightSourceAmbientProduct;

	float d = max( dot(L, N), 0.0 );
	vec4  diffuse = d * lightSourceDiffuseProduct;

	float s = pow( max(dot(N, H), 0.0), Shininess );
	vec4  specular = s * lightSourceSpecularProduct;

	if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}


    vec4 point_source_color =  attenuation * (ambient + diffuse + specular);

	return point_source_color;
}

vec4 spot_light_source() 
{

//vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	vec3 pos = (model_view * vPosition4).xyz;

	vec3 L = normalize( LightPosition.xyz - pos );
	vec3 E = normalize( -pos );
	vec3 H = normalize( L + E );

	vec3 N = normalize(Normal_Matrix * vNormal);

    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;

	float dist = distance(LightPosition.xyz, pos );

	float a = 2.0;
	float b = 0.01; 
	float c = 0.001;

	float attenuation = 1 / (a + b * dist + c * dist * dist);
	vec3 Lf = normalize( Towards.xyz - LightPosition.xyz );

	float cutoff = cos(M_PI/9);

	float actual = dot(-L,Lf);
	

	if ( actual < cutoff ) {
		attenuation = 0;
	} else {
		attenuation *= pow(actual, 15);
	}

	vec4 ambient = lightSourceAmbientProduct;

	float d = max( dot(L, N), 0.0 );
	vec4  diffuse = d * lightSourceDiffuseProduct;

	float s = pow( max(dot(N, H), 0.0), Shininess );
	vec4  specular = s * lightSourceSpecularProduct;

	if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}

    vec4 spot_light_color = attenuation * (ambient + diffuse + specular);

	return spot_light_color;
}


void main() {
	
	vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);


	gl_Position = projection * model_view * vPosition4;

	if (flag == 0) {
		color =  globalAmbientProduct + distant_light_source() + spot_light_source();
	} else {
		color =  globalAmbientProduct + distant_light_source() + point_source_source();
	}
}
