/************************************************************
 * Handout: rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
                                 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
              sample code "example3.cpp" of Chapter 4.
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
   by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-yjc-new.h".)

 * Extensively modified by Yi-Jen Chiang for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Yi-Jen Chiang to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rotating cube are drawn.

** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned to each vertex and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/
#include "Angel-yjc.h"
#include <string>
#include <fstream>


using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

typedef Angel::vec4 color4;
typedef Angel::vec4 point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program53;       /*shader53*/
GLuint program42;       /*shader42*/

GLuint sphere_buffer;   /* vertex buffer object id for cube */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;
GLuint shadow_buffer;

GLuint segment = 0;

GLfloat angle_seg1 = (sqrt(73) / (2 * M_PI * 1)) * 360;
GLfloat angle_seg2 = (sqrt(97) / (2 * M_PI * 1)) * 360;
GLfloat angle_seg3 = (sqrt(50) / (2 * M_PI * 1)) * 360;

// Projection transformation parameters
GLfloat  fovy = 55.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 13.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int beginAnimation = 0;
int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'

int cubeFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int sphereFlag = 0; // 0, wireframe sphere, toggled by mouse menu
int shadowFlag = 1; // 1: have a shadow, 0: no shadow 
int flat_smooth = 0; // flat is 0, smooth is 1;
int spotLight_lightSource = 0; // spotLight:0, pointSource:1

int lightFlag = 0;

int triangle_num;
 //const int cube_NumVertices = 36; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)



mat4 rotate_record=mat4(vec4(1,0,0,0),
	vec4(0,1,0,0),
	vec4(0,0,1,0),
	vec4(0,0,0,1));
GLfloat angle_record = 0;
vec3 rotate_axis;



mat4 shadow = mat4(
	vec4(12, 14, 0, 0),
	vec4(0, 0, 0, 0),
	vec4(0, 3, 12, 0),
	vec4(0, -1, 0, 12)
	);

#if 0
point3 cube_points[100]; 
color3 cube_colors[100];
#endif

#if 1
point3 sphere_points[4000];
color4 sphere_colors[4000];
vec3 sphere_flat_normals[4000];
vec3 sphere_smooth_normals[4000];

#endif

point3 shadow_points[4000];
color4 shadow_colors[4000];

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 floor_flat_normals[floor_NumVertices];
vec3 floor_smooth_normals[floor_NumVertices];

#if 1
const int axis_NumVertices = 6;
point3 axis_points[6];
color4 axis_color[6];
#endif

point3 **sphere_vertices;


point3 axis_vertices[4] = {
	point3(0.0, 0.0, 0.0),
	point3(5.0, 0.0, 0.0),
	point3(0.0, 0.0, 10.0),
	point3(0.0, 5.0, 0.0)
};


//Vertices of a plain centered at origin, sides aligned with axes
point3 vertices[4] = {
	point3( 5.0, 0.0, 8.0 ),
	point3( 5.0, 0.0, -4.0 ),
	point3(-5.0, 0.0, -4.0 ),
	point3(-5.0, 0.0, 8.0 )
};
// RGBA colors
color4 vertex_colors[6] = {
	color4(1.0, 0.84, 0.0, 1.0), //
	color4(0.0, 1.0, 0.0, 1.0), //
	color4(1.0, 0.0, 0.0, 1.0), //red
	color4(1.0, 0.0, 1.0, 1.0), //magenta
	color4(0.0, 0.0, 1.0, 1.0), //blue
	color4(0.25,0.25,0.25, 0.65)
	//color4(1,0.25,0.25, 0.65)
};


color4 global_ambient(1.0, 1.0, 1.0, 1.0);

color4 light_ambient(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 light_specular(0.2, 0.2, 0.2, 1.0);

point4 light_direction(0.1, 0.0, -1.0, 0.0);

color4 ground_ambient(0.2, 0.2, 0.2, 1.0);
color4 ground_diffuse(0.0, 1.0, 0.0, 1.0);
color4 ground_specular(0.0, 0.0, 0.0, 1.0);

color4 sphere_ambient(0.2, 0.2, 0.2, 1.0);
color4 sphere_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_specular(1.0, 0.84, 0.0, 1.0);
float sphere_shininess = 125.0;


color4 light_ground_ambient = ground_ambient * light_ambient;
color4 light_ground_diffuse = ground_diffuse *  light_diffuse;
color4 light_ground_specular = ground_specular * light_specular;

color4 light_sphere_ambient = sphere_ambient * light_ambient;
color4 light_sphere_diffuse = sphere_diffuse * light_diffuse;
color4 light_sphere_specular = sphere_specular * light_specular;

color4 global_sphere_ambient = sphere_ambient * global_ambient;
color4 global_ground_ambient = ground_ambient * global_ambient;


/* *******************************part d**************************************/
color4 d_light_ambient(0.0, 0.0, 0.0, 1.0);
color4 d_light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 d_light_specular(1.0, 1.0, 1.0, 1.0);

point4 d_light_position(-14.0, 12.0, -3.0, 1.0);
point4 towards(-6.0, 0.0, -4.5, 1.0);

color4 lightSource_ground_AmbientProduct = ground_ambient * d_light_ambient;
color4 lightSource_ground_diffuseProduct = ground_diffuse * d_light_diffuse;
color4 lightSource_ground_specularProduct = ground_specular * d_light_specular;

color4 lightSource_sphere_AmbientProduct = sphere_ambient * d_light_ambient;
color4 lightSource_sphere_diffuseProduct = sphere_diffuse * d_light_diffuse;
color4 lightSource_sphere_specularProduct = sphere_specular * d_light_specular;

/******************************************************************************/
//----------------------------------------------------------------------------
int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// quad(): generate two triangles for each face and assign colors to the vertices,everytime it was revoked will complete a face
/*void quad( int a, int b, int c, int d )
{
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
    cube_colors[Index] = vertex_colors[b]; cube_points[Index] = vertices[b]; Index++;
    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;

    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;
    cube_colors[Index] = vertex_colors[d]; cube_points[Index] = vertices[d]; Index++;
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
}*/

void tria(int i) {

	vec4 u = sphere_vertices[i][1] - sphere_vertices[i][0];
	vec4 v = sphere_vertices[i][2] - sphere_vertices[i][1];

	vec3 normal = normalize(cross(u, v));


	sphere_colors[Index] = vertex_colors[0]; sphere_points[Index] = sphere_vertices[i][0];
	sphere_smooth_normals[Index] = normalize(sphere_vertices[i][0]); sphere_flat_normals[Index] = normal;
	shadow_colors[Index] = vertex_colors[5]; shadow_points[Index] = sphere_vertices[i][0];  Index++;

	sphere_colors[Index] = vertex_colors[0]; sphere_points[Index] = sphere_vertices[i][1];
	sphere_smooth_normals[Index] = normalize(sphere_vertices[i][1]); sphere_flat_normals[Index] = normal;
	shadow_colors[Index] = vertex_colors[5]; shadow_points[Index] = sphere_vertices[i][1];  Index++;

	sphere_colors[Index] = vertex_colors[0]; sphere_points[Index] = sphere_vertices[i][2];
	sphere_smooth_normals[Index] = normalize(sphere_vertices[i][2]); sphere_flat_normals[Index] = normal;
	shadow_colors[Index] = vertex_colors[5]; shadow_points[Index] = sphere_vertices[i][2];  Index++;
}	

//----------------------------------------------------------------------------
// generate 12 triangles: 36 vertices and 36 colors
/*void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}*/

void colorsphere() {
	for (int i = 0; i < triangle_num; i++) {
		tria(i);
	}
}

//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
	vec4 u = vertices[1] - vertices[0];
	vec4 v = vertices[2] - vertices[0];

	vec3 normal = normalize(cross(u, v));

	floor_colors[0] = vertex_colors[1]; floor_points[0] = vertices[0]; floor_flat_normals[0] = normal; floor_smooth_normals[0] = normal;
    floor_colors[1] = vertex_colors[1]; floor_points[1] = vertices[3]; floor_flat_normals[1] = normal; floor_smooth_normals[1] = normal;
    floor_colors[2] = vertex_colors[1]; floor_points[2] = vertices[2]; floor_flat_normals[2] = normal; floor_smooth_normals[2] = normal;

    floor_colors[3] = vertex_colors[1]; floor_points[3] = vertices[2]; floor_flat_normals[3] = normal; floor_smooth_normals[3] = normal;
    floor_colors[4] = vertex_colors[1]; floor_points[4] = vertices[1]; floor_flat_normals[4] = normal; floor_smooth_normals[4] = normal;
    floor_colors[5] = vertex_colors[1]; floor_points[5] = vertices[0]; floor_flat_normals[5] = normal; floor_smooth_normals[5] = normal;
}

void axis() {
	axis_points[0] = axis_vertices[0]; axis_color[0] = vertex_colors[2];
	axis_points[1] = axis_vertices[1]; axis_color[1] = vertex_colors[2];

	axis_points[2] = axis_vertices[0]; axis_color[2] = vertex_colors[4];
	axis_points[3] = axis_vertices[2]; axis_color[3] = vertex_colors[4];

	axis_points[4] = axis_vertices[0]; axis_color[4] = vertex_colors[3];
	axis_points[5] = axis_vertices[3]; axis_color[5] = vertex_colors[3];
}

//------------------------------
// readFile
void readFile(const char* fileName) {
	int point_num;
	float x, y, z;

	ifstream myfile(fileName);
	cout << fileName << endl;

	if (myfile.is_open()) {

		myfile >> triangle_num;
		sphere_vertices = new point3*[triangle_num];

		for (int i = 0; i < triangle_num; i++) {
			myfile >> point_num;
			sphere_vertices[i] = new point3[point_num];

			for (int j = 0; j < point_num; j++) {
				myfile >> x >> y >> z;

				sphere_vertices[i][j] = point3(x, y, z);

				//cout << sphere_vertices[i][j].x << " " << sphere_vertices[i][j].y << " " << sphere_vertices[i][j].z << endl;
			}
		}
	}
	else {
		cout << "error" << endl;
	}
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
	char option;

	do {
		static string indication = "please input the file that you want to load, 'a' represents 'sphere.8', 'b' represents 'sphere.128, c represents 'sphere.256, d represents 'sphere.1024";
		cout << indication << endl;
		cin >> option;
		if (option == 'a') {
			readFile("sphere.8.txt");
		}
		else if (option == 'b') {
			readFile("sphere.128.txt");
		}
		else if (option == 'c') {
			readFile("sphere.256.txt");
		}
		else if (option == 'd') {
			readFile("sphere.1024.txt");
		}

		indication = "please input a or b or c or d";

	} while (option != 'a' && option != 'b' && option != 'c' && option != 'd');

    //colorcube();
	colorsphere();

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif

 // Create and initialize a vertex buffer object for cube, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);

#if 1
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * triangle_num + sizeof(color4) * 3 * triangle_num + sizeof(vec3) * 3 * triangle_num + sizeof(vec3) * 3 * triangle_num,
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * 3 * triangle_num, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * triangle_num, sizeof(color4) * 3 * triangle_num,
                    sphere_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3)*3*triangle_num+sizeof(color4)*3*triangle_num,  sizeof(vec3)*3*triangle_num,
					sphere_flat_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * triangle_num + sizeof(color4) * 3 * triangle_num + sizeof(vec3) * 3 * triangle_num, sizeof(vec3) * 3 * triangle_num,
		sphere_smooth_normals);
#endif

	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);

#if 1
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * triangle_num + sizeof(color4) * 3 * triangle_num,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * 3 * triangle_num, shadow_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * 3 * triangle_num, sizeof(color4) * 3 * triangle_num,
		shadow_colors);
#endif



    floor();
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_flat_normals) + sizeof(floor_smooth_normals),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_flat_normals),
		floor_flat_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_flat_normals), sizeof(floor_smooth_normals),
		floor_smooth_normals);


	axis();
// create axis;
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_color), 
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_color), 
					axis_color);

 // Load shaders and create a shader program (to be used in display())
	program42 = InitShader("vshader42.glsl", "fshader42.glsl");

    program53 = InitShader("vshader53.glsl", "fshader53.glsl");
	
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.52, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}

//----------------------------------------------------------------------------
// drawlines

void drawLine(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program42, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program42, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_LINE_STRIP, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}


//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, int program)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(sizeof(point3) * num_vertices) ); 
    // the offset is the (total) size of the previous vertex attribute array(s)

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);

	if (!flat_smooth) {
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices));
	}
	else {
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices));
	}


    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}



void SetUp_Lighting_Uniform_Vars(int flag, mat4 mv_lookAt) // flag = 1 represents sphere, flag = 2 represents ground
{

	glUniform1i(glGetUniformLocation(program53, "flag"), spotLight_lightSource);

	if (flag == 1) {
		glUniform4fv(glGetUniformLocation(program53, "AmbientProduct"),
			1, light_sphere_ambient);
		glUniform4fv(glGetUniformLocation(program53, "DiffuseProduct"),
			1, light_sphere_diffuse);
		glUniform4fv(glGetUniformLocation(program53, "SpecularProduct"),
			1, light_sphere_specular);
		glUniform4fv(glGetUniformLocation(program53, "globalAmbientProduct"),
			1, global_sphere_ambient);
		glUniform4fv(glGetUniformLocation(program53, "LightDirection"),
			1, light_direction);

		glUniform4fv(glGetUniformLocation(program53, "lightSourceAmbientProduct"),
			1, lightSource_sphere_AmbientProduct);
		glUniform4fv(glGetUniformLocation(program53, "lightSourceDiffuseProduct"),
			1, lightSource_sphere_diffuseProduct);
		glUniform4fv(glGetUniformLocation(program53, "lightSourceSpecularProduct"),
			1, lightSource_sphere_specularProduct);


		// The Light Position in Eye Frame
		vec4 light_position_eyeFrame = mv_lookAt * d_light_position;
		glUniform4fv(glGetUniformLocation(program53, "LightPosition"),1, light_position_eyeFrame);
		vec4 towards_eyeFrame = mv_lookAt * towards;
		glUniform4fv(glGetUniformLocation(program53, "Towards"), 1, towards_eyeFrame);

		//glUniform1f(glGetUniformLocation(program53, "ConstAtt"),const_att);
		//glUniform1f(glGetUniformLocation(program53, "LinearAtt"),linear_att);
		//glUniform1f(glGetUniformLocation(program53, "QuadAtt"),quad_att);

		glUniform1f(glGetUniformLocation(program53, "Shininess"), sphere_shininess);
	}
	else if (flag == 2) {
		glUniform4fv(glGetUniformLocation(program53, "AmbientProduct"),
			1, light_ground_ambient);
		glUniform4fv(glGetUniformLocation(program53, "DiffuseProduct"),
			1, light_ground_diffuse);
		glUniform4fv(glGetUniformLocation(program53, "SpecularProduct"),
			1, light_ground_specular);
		glUniform4fv(glGetUniformLocation(program53, "globalAmbientProduct"),
			1, global_ground_ambient);
		glUniform4fv(glGetUniformLocation(program53, "LightDirection"),
			1, light_direction);

		glUniform4fv(glGetUniformLocation(program53, "lightSourceAmbientProduct"),
			1, lightSource_ground_AmbientProduct);
		glUniform4fv(glGetUniformLocation(program53, "lightSourceDiffuseProduct"),
			1, lightSource_ground_diffuseProduct);
		glUniform4fv(glGetUniformLocation(program53, "lightSourceSpecularProduct"),
			1, lightSource_ground_specularProduct);

		// The Light Position in Eye Frame
		vec4 light_position_eyeFrame = mv_lookAt * d_light_position;
		glUniform4fv(glGetUniformLocation(program53, "LightPosition"), 1, light_position_eyeFrame);
		vec4 towards_eyeFrame = mv_lookAt * towards;
		glUniform4fv(glGetUniformLocation(program53, "Towards"), 1, towards_eyeFrame);


		//glUniform1f(glGetUniformLocation(program53, "ConstAtt"),const_att);
		//glUniform1f(glGetUniformLocation(program53, "LinearAtt"),linear_att);
		//glUniform1f(glGetUniformLocation(program53, "QuadAtt"),quad_att);
		//glUniform1f(glGetUniformLocation(program53, "Shininess"), sphere_shininess);

		
	}
	
	
}


//----------------------------------------------------------------------------
void display(void)
{
	GLuint  model_view53;  // shader53
	GLuint  projection53;  // shader53
	GLuint  projection42;  // shader42
	GLuint  model_view42;  // shader42

	mat4 sphere_rotate;

	mat4  p = Perspective(fovy, aspect, zNear, zFar);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	model_view42 = glGetUniformLocation(program42, "model_view");
	projection42 = glGetUniformLocation(program42, "projection");

	glUseProgram(program42);
	glUniformMatrix4fv(projection42, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
	
	//glUseProgram(program53); // Use the shader program53, shader53;

	model_view53 = glGetUniformLocation(program53, "model_view");
	projection53 = glGetUniformLocation(program53, "projection");

	glUseProgram(program53);
	glUniformMatrix4fv(projection53, 1, GL_TRUE, p);
	
	
	

	/*---  Set up and pass on Projection matrix to the shader ---*/
	
	//glUniformMatrix4fv(projection42, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	/*---  Set up and pass on Model-View matrix to the shader ---*/
	// eye is a global variable of vec4 set to init_eye and updated by keyboard()
	vec4    at(0.0, 0.0, 0.0, 1.0);
	vec4    up(0.0, 1.0, 0.0, 0.0);

	mat4  mv = LookAt(eye, at, up);
	mat4 lookAt_mv = LookAt(eye, at, up);
	mat4  mv_floor;

#if 0 // The following is to verify the correctness of the function NormalMatrix():
	// Commenting out Rotate() and un-commenting mat4WithUpperLeftMat3() 
	// gives the same result.
	mv = Translate(-1.0, -0.5, 0.0) * mv * Scale(1.4, 1.4, 1.4)
		* Rotate(angle, 0.0, 0.0, 2.0);
	// * mat4WithUpperLeftMat3(NormalMatrix(Rotate(angle, 0.0, 0.0, 2.0), 1));
#endif
#if 1 // The following is to verify that Rotate() about (0,2,0) is RotateY():
	  // Commenting out Rotate() and un-commenting RotateY()
	  // gives the same result.
	if (segment == 0) {
		float distance = angle * 2 * M_PI / 360;
		vec3 rotate_beg(-4, 1, 4);
		vec3 rotate_up(0, 1, 0);
		vec3 rotate_head(3, 0, -8);
		vec3 translation(rotate_beg.x + distance * rotate_head.x / sqrt(73), rotate_beg.y + distance * rotate_head.y / sqrt(73),
			rotate_beg.z + distance * rotate_head.z / sqrt(73));
		rotate_axis = cross(rotate_up, rotate_head);
		sphere_rotate = Translate(translation) * Scale(1.0, 1.0, 1.0)
			* Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z)*rotate_record;
		mv = mv * sphere_rotate;
	}
	else if (segment == 1) {
		float distance = angle * 2 * M_PI / 360;
		vec3 rotate_beg(-1, 1, -4);
		vec3 rotate_up(0, 1, 0);
		vec3 rotate_head(4, 0, 9);
		vec3 translation(rotate_beg.x + distance * rotate_head.x / sqrt(97), rotate_beg.y + distance * rotate_head.y / sqrt(97),
			rotate_beg.z + distance * rotate_head.z / sqrt(97));
		rotate_axis = cross(rotate_up, rotate_head);
		sphere_rotate = Translate(translation) * Scale(1.0, 1.0, 1.0)
			* Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z)*rotate_record;
		mv = mv * sphere_rotate;
	}
	else if (segment == 2) {
		float distance = angle * 2 * M_PI / 360;
		vec3 rotate_beg(3, 1, 5);
		vec3 rotate_up(0, 1, 0);
		vec3 rotate_head(-7, 0, -1);
		vec3 translation(rotate_beg.x + distance * rotate_head.x / sqrt(50), rotate_beg.y + distance * rotate_head.y / sqrt(50),
			rotate_beg.z + distance * rotate_head.z / sqrt(50));
		rotate_axis = cross(rotate_up, rotate_head);
		sphere_rotate = Translate(translation) * Scale(1.0, 1.0, 1.0)
			* Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z)*rotate_record;
		mv = mv * sphere_rotate;
	}
	// * RotateY(angle);
#endif
#if 0  // The following is to verify that Rotate() about (3,0,0) is RotateX():
	   // Commenting out Rotate() and un-commenting RotateX()
	   // gives the same result.
	mv = Translate(-1.0, -0.5, 0.0) * mv * Scale(1.4, 1.4, 1.4)
		* Rotate(angle, 3.0, 0.0, 0.0);
	// * RotateX(angle);
#endif
	/*if (!lightFlag) {
		glUseProgram(program42);
		glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	}
	else {
		glUseProgram(program53);
		glUniformMatrix4fv(model_view53, 1, GL_TRUE, mv);
	}*/

	if (sphereFlag == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (!lightFlag) {
		glUseProgram(program42);

		glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv);
		drawObj(sphere_buffer, 3 * triangle_num, program42);  // draw the sphere using shader42
	}
	else {
		SetUp_Lighting_Uniform_Vars(1, lookAt_mv);

		glUseProgram(program53);

		mat3 normal_matrix = NormalMatrix(mv, 1);

		glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);


		glUniformMatrix4fv(model_view53, 1, GL_TRUE, mv);
		drawObj(sphere_buffer, 3 * triangle_num, program53);  // draw the sphere using shader53
	}

	/**********************************************************************************************************/
	/********************************part b, do the decal operation********************************************/
	/**********************************************************************************************************/
	mv_floor = LookAt(eye, at, up) * Scale(1, 1, 1);
	 // GL_TRUE: matrix is row-major
	

	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDepthMask(GL_FALSE);

	if (!lightFlag) {
		glUseProgram(program42);
		glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv_floor);
		drawObj(floor_buffer, floor_NumVertices, program42);  // draw the floor
	}
	else {
		glUseProgram(program53);

		glUniformMatrix4fv(model_view53, 1, GL_TRUE, mv_floor);
		SetUp_Lighting_Uniform_Vars(2, lookAt_mv);

		mat3 normal_matrix = NormalMatrix(mv_floor, 1);

		glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

		drawObj(floor_buffer, floor_NumVertices, program53);
	}


	if (shadowFlag == 1) {
		mv = LookAt(eye, at, up);
		mv = mv * shadow * sphere_rotate;
		glUseProgram(program42);
		glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv);
		if (sphereFlag == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}	
		glDepthMask(GL_TRUE);

		drawObj(shadow_buffer, 3 * triangle_num, program42);
	}


    mv_floor =  LookAt(eye, at, up) * Scale (1, 1, 1);
    glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv_floor); // GL_TRUE: matrix is row-major
    if (floorFlag == 1) // Filled floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	//glUseProgram(program42);
    //drawObj(floor_buffer, floor_NumVertices, program42);  // draw the floor

	if (!lightFlag) {
		glUseProgram(program42);
		drawObj(floor_buffer, floor_NumVertices, program42);  // draw the floor
	}
	else {
		glUseProgram(program53);

		glUniformMatrix4fv(model_view53, 1, GL_TRUE, mv_floor);
		SetUp_Lighting_Uniform_Vars(2, lookAt_mv);

		mat3 normal_matrix = NormalMatrix(mv_floor, 1);

		glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

		drawObj(floor_buffer, floor_NumVertices, program53);
	}


	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	/********************************************************************************************************************/
	/***********************************part b, do the decal operation***************************************************/
	/********************************************************************************************************************/


	glUniformMatrix4fv(model_view42, 1, GL_TRUE, mv_floor); // GL_TRUE: matrix is row-major

	glUseProgram(program42); //shader42
	drawLine(axis_buffer, axis_NumVertices);  // draw the floor





    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{

	angle += 0.5;
	if ( segment == 0 && angle >= angle_seg1 ) {
		rotate_record = Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z) * rotate_record;
		angle = 0;
		segment = (segment+1) % 3;
	}
	else if (segment == 1 && angle >= angle_seg2) {
		rotate_record = Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z) * rotate_record;
		angle = 0;
		segment = (segment + 1) % 3;
	}
	else if (segment == 2 && angle >= angle_seg3) {
		rotate_record = Rotate(angle, rotate_axis.x, rotate_axis.y, rotate_axis.z) * rotate_record;
		angle = 0;
		segment = (segment + 1) % 3;
	}
	
  // angle += 1.0;    //YJC: change this value to adjust the cube rotation speed.
  glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;
		
	case 'b': beginAnimation = 1; animationFlag = 1;  glutIdleFunc(idle); break;
	   
        case 'c': case 'C': // Toggle between filled and wireframe cube
	    cubeFlag = 1 -  cubeFlag;   
            break;

        case 'f': case 'F': // Toggle between filled and wireframe floor
	    floorFlag = 1 -  floorFlag; 
            break;

	case ' ':  // reset to initial viewer/eye position
	    eye = init_eye;
	    break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	/*if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		exit(0);
	}*/
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1 && beginAnimation == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
	}
}

void demo_menu(int id) {
	switch (id) {
	case 1:
		exit(0);
		break;
	case 2:
		eye = init_eye;
		break;
	}
	glutIdleFunc(idle);
	animationFlag = 1;
	beginAnimation = 1;
}

void shadow_menu(int id) {
	switch (id) {
	case 1:
		shadowFlag = 1;
		break;
	case 2:
		shadowFlag = 0;
		break;
	}
}

void wire_menu(int id) {
	switch (id) {
	case 1:
		sphereFlag = 0;
		break;
	case 2:
		sphereFlag = 1;
		break;
	}
}


void light_menu(int id) {
	switch (id) {
	case 1:
		lightFlag = 1;
		break;
	case 2:
		lightFlag = 0;
		break;
	}
	
}

void shading_menu(int id) {
	switch (id) {
	case 1:
		flat_smooth = 0;
		break;
	case 2:
		flat_smooth = 1;
		break;
	}
}

void lightSource_menu(int id) {
	switch (id) {
	case 1:
		spotLight_lightSource = 0;
		break;
	case 2:
		spotLight_lightSource = 1;
		break;
	}
}


//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{ int err;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    // glutInitContextVersion(3, 2);
    // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Color Cube");

  /* Call glewInit() and error checking */
  err = glewInit();
  if (GLEW_OK != err)
  { printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
    exit(1);
  }
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	int sub_menu = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);

	int fill_menu = glutCreateMenu(wire_menu);
	glutAddMenuEntry("wired", 1);
	glutAddMenuEntry("filled", 2);

	int Lmenu = glutCreateMenu(light_menu);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);

	int Smenu = glutCreateMenu(shading_menu);
	glutAddMenuEntry("flat shading", 1);
	glutAddMenuEntry("smooth shading", 2);

	int Lsource = glutCreateMenu(lightSource_menu);
	glutAddMenuEntry("spot light", 1);
	glutAddMenuEntry("point source", 2);

	int main_menu = glutCreateMenu(demo_menu);
	glutAddMenuEntry("quit", 1);
	glutAddMenuEntry("Default View Point", 2);
	glutAddSubMenu("Wire Frame Sphere", fill_menu);
	glutAddSubMenu("shadow", sub_menu);
	glutAddSubMenu("Enable Lighting", Lmenu);
	glutAddSubMenu("shading", Smenu);
	glutAddSubMenu("light source", Lsource);
	
	glutAttachMenu(GLUT_LEFT_BUTTON);

    init();

    glutMainLoop();
    return 0;
}
