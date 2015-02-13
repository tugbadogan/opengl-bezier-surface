#define GLUI_FREEGLUT true

#include "Angel.h"
#include "GL/glui.h"
#include "SOIL.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define NI 5
#define NJ 4
#define RESOLUTIONI 10*NI
#define RESOLUTIONJ 10*NJ
#define checkImageWidth 64
#define checkImageHeight 64

static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
double inp[NI + 1][NJ + 1][3];
double outp[RESOLUTIONI][RESOLUTIONJ][3];

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };

const int NumVertices = 100000;
int mainWindow, settingsWindow;
double point_x, point_y, point_z;
int points_i = 1;
int points_j = 1;
int Index = 0;
int is_wireframe = 0;
int is_phong = 0;
int is_texture = 0;
int is_bump_texture = 0;
int is_environment_texture = 0;
int is_rotate = 0;
int is_bezier = 0;
int Axis = Xaxis;
int scale_number = 1;

GLuint wireframeLocation;
GLuint phongShadingLocation;
GLuint textureLocation;
GLuint bumpTextureLocation;
GLuint environmentTextureLocation;

GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;
point4 points[NumVertices];
vec3   normals[NumVertices];
vec2   textures[NumVertices];

//----------------------------------------------------------------------------

extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

//---------------------------------------------------------------------------
// Tutorial Reference: http://www.swiftless.com/tutorials/glsl/8_bump_mapping.html-
GLuint LoadTexture(const char * filename, int width, int height)
{
	GLuint texture;
	unsigned char * data;
	FILE * file;

	//The following code will read in our RAW file  
	file = fopen(filename, "rb");

	if (file == NULL) return 0;
	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);

	fclose(file);

	glGenTextures(1, &texture); //generate the texture with the loaded data  
	glBindTexture(GL_TEXTURE_2D, texture); //bind the texture to it’s array  

	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //set texture environment parameters  

	//And if you go and use extensions, you can use Anisotropic filtering textures which are of an  
	//even better quality, but this will do for now.  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Here we are setting the parameter to repeat the texture instead of clamping the texture  
	//to the edge of our shape.  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Generate the texture  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	free(data); //free the texture  

	return texture; //return whether it was successfull  
}


// Reference: http://ogldev.atspace.co.uk/www/tutorial25/tutorial25.html
GLuint cubeTextureObj;
bool LoadCubeTexture()
{
	string m_fileNames[6] = { "sp3right.jpg",
		"sp3left.jpg",
		"sp3top.jpg",
		"sp3bot.jpg",
		"sp3front.jpg",
		"sp3back.jpg" };
	glGenTextures(1, &cubeTextureObj);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureObj);
	GLenum types[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
	
	for (unsigned int i = 0; i < 6; i++) {
		int width, height;
		unsigned char* image = SOIL_load_image(m_fileNames[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);

		glTexImage2D(types[i], 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return true;
}

void
exitScreen()
{
	exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------------

void
wireframe()
{
	is_wireframe = 1;
	is_phong = 0;
	is_texture = 0;
	is_bump_texture = 0;
	is_environment_texture = 0;
}

//----------------------------------------------------------------------------

void
gouraud_shading()
{
	is_wireframe = 0;
	is_phong = 0;
	is_texture = 0;
	is_bump_texture = 0;
	is_environment_texture = 0;
}

//----------------------------------------------------------------------------

void
phong_shading()
{
	is_wireframe = 0;
	is_phong = 1;
	is_texture = 0;
	is_bump_texture = 0;
	is_environment_texture = 0;
}

//----------------------------------------------------------------------------

void
parametricTextureMapping()
{
	is_wireframe = 0;
	is_phong = 0;
	is_bump_texture = 0;
	is_texture = 1;
	is_environment_texture = 0;
}

//----------------------------------------------------------------------------

void
bumpTexture()
{
	is_wireframe = 0;
	is_phong = 0;
	is_texture = 0;
	is_bump_texture = 1;
	is_environment_texture = 0;
}

//----------------------------------------------------------------------------

void
environmentTexture()
{
	is_wireframe = 0;
	is_phong = 0;
	is_texture = 0;
	is_bump_texture = 0;
	is_environment_texture = 1;
}
//----------------------------------------------------------------------------

void
rotateX()
{
	Axis = Xaxis;
	is_rotate = 1;
}

//----------------------------------------------------------------------------

void
rotateY()
{
	Axis = Yaxis;
	is_rotate = 1;
}

//----------------------------------------------------------------------------

void
rotateZ()
{
	Axis = Zaxis;
	is_rotate = 1;
}

//----------------------------------------------------------------------------

void
stopRotation()
{
	is_rotate = 0;
}

//----------------------------------------------------------------------------

void
zoomIn()
{
	scale_number += 1;
}

//----------------------------------------------------------------------------

void
zoomOut()
{
	if (scale_number > 1)
	{
		scale_number -= 1;
	}
}

//----------------------------------------------------------------------------
// Chapter05_Example3
void
quad(point4 a, point4 b, point4 c, point4 d)
{
	// Initialize temporary vectors along the quad's edge to
	//   compute its face normal 
	vec4 u = b - a;
	vec4 v = c - b;

	vec3 normal = normalize(cross(u, v));

	textures[Index] = vec2(-1.0, -1.0); normals[Index] = normal; points[Index] = a; Index++;
	textures[Index] = vec2(1.0, -1.0); normals[Index] = normal; points[Index] = b; Index++;
	textures[Index] = vec2(1.0, 1.0); normals[Index] = normal; points[Index] = c; Index++;
	textures[Index] = vec2(-1.0, -1.0); normals[Index] = normal; points[Index] = a; Index++;
	textures[Index] = vec2(1.0, 1.0); normals[Index] = normal; points[Index] = c; Index++;
	textures[Index] = vec2(-1.0, 1.0); normals[Index] = normal; points[Index] = d; Index++;
}

//----------------------------------------------------------------------------

void
addPoints(point4 a)
{
	points[Index] = a; Index++;
}

//----------------------------------------------------------------------------
// Reference: http://paulbourke.net/geometry/bezier/
GLfloat
BezierBlend(int k, GLfloat mu, int n)
{
	int nn, kn, nkn;
	GLfloat blend = 1;

	nn = n;
	kn = k;
	nkn = n - k;

	while (nn >= 1)
	{
		blend *= nn;
		nn--;
		if (kn > 1)
		{
			blend /= (GLfloat)kn;
			kn--;
		}
		if (nkn > 1)
		{
			blend /= (GLfloat)nkn;
			nkn--;
		}
	}
	if (k > 0)
		blend *= pow(mu, (GLfloat)k);
	if (n - k > 0)
		blend *= pow(1 - mu, (GLfloat)(n - k));

	return(blend);
}

//----------------------------------------------------------------------------
// Reference: http://paulbourke.net/geometry/bezier/
void
random_generate()
{
	Index = 0;
	int i, j, ki, kj;
	GLfloat mui, muj, bi, bj;
	srand(time(0));
	for (i = 0; i <= NI; i++)
	{
		for (j = 0; j <= NJ; j++)
		{
			inp[i][j][0] = i / 5.0 - 0.5;
			inp[i][j][1] = j / 5.0 - 0.5;
			inp[i][j][2] = (rand() % 10000) / 5000.0 - 1;
		}
	}

	for (i = 0; i<RESOLUTIONI; i++)
	{
		mui = i / (GLfloat)(RESOLUTIONI - 1);
		for (j = 0; j<RESOLUTIONJ; j++)
		{
			muj = j / (GLfloat)(RESOLUTIONJ - 1);
			outp[i][j][0] = 0;
			outp[i][j][1] = 0;
			outp[i][j][2] = 0;
			for (ki = 0; ki <= NI; ki++)
			{
				bi = BezierBlend(ki, mui, NI);
				for (kj = 0; kj <= NJ; kj++)
				{
					bj = BezierBlend(kj, muj, NJ);
					outp[i][j][0] += (inp[ki][kj][0] * bi * bj);
					outp[i][j][1] += (inp[ki][kj][1] * bi * bj);
					outp[i][j][2] += (inp[ki][kj][2] * bi * bj);
				}
			}
		}
	}

	//printf("LIST\n");

	/* Display the surface, in this case in OOGL format for GeomView */
	//1printf("{ = CQUAD\n");
	for (i = 0; i<RESOLUTIONI - 1; i++)
	{
		for (j = 0; j<RESOLUTIONJ - 1; j++)
		{
			point4 a = point4(outp[i][j][0], outp[i][j][1], outp[i][j][2], 1.0);
			point4 b = point4(outp[i][j + 1][0], outp[i][j + 1][1], outp[i][j + 1][2], 1.0);
			point4 c = point4(outp[i + 1][j][0], outp[i + 1][j][1], outp[i + 1][j][2], 1.0);
			point4 d = point4(outp[i + 1][j + 1][0], outp[i + 1][j + 1][1], outp[i + 1][j + 1][2], 1.0);

			/*if (i == 0 && j == 0)
			{
			cout << a.x << " " << a.y << " " << a.z << endl;
			cout << b.x << " " << b.y << " " << b.z << endl;
			cout << c.x << " " << c.y << " " << c.z << endl;
			cout << d.x << " " << d.y << " " << d.z << endl;
			}*/

			quad(a, c, d, b);
			// i = RESOLUTIONI;
			// j = RESOLUTIONJ;
		}
	}
	is_bezier = 1;
}

//----------------------------------------------------------------------------
// Reference: http://paulbourke.net/geometry/bezier/
void
bezierSurface()
{
	int i, j, ki, kj;
	GLfloat mui, muj, bi, bj;

	for (i = 0; i<RESOLUTIONI; i++)
	{
		mui = i / (GLfloat)(RESOLUTIONI - 1);
		for (j = 0; j<RESOLUTIONJ; j++)
		{
			muj = j / (GLfloat)(RESOLUTIONJ - 1);
			outp[i][j][0] = 0;
			outp[i][j][1] = 0;
			outp[i][j][2] = 0;
			for (ki = 0; ki <= NI; ki++)
			{
				bi = BezierBlend(ki, mui, NI);
				for (kj = 0; kj <= NJ; kj++)
				{
					bj = BezierBlend(kj, muj, NJ);
					outp[i][j][0] += (inp[ki][kj][0] * bi * bj);
					outp[i][j][1] += (inp[ki][kj][1] * bi * bj);
					outp[i][j][2] += (inp[ki][kj][2] * bi * bj);
				}
			}
		}
	}

	for (i = 0; i<RESOLUTIONI - 1; i++)
	{
		for (j = 0; j<RESOLUTIONJ - 1; j++)
		{
			point4 a = point4(outp[i][j][0], outp[i][j][1], outp[i][j][2], 1.0);
			point4 b = point4(outp[i][j + 1][0], outp[i][j + 1][1], outp[i][j + 1][2], 1.0);
			point4 c = point4(outp[i + 1][j][0], outp[i + 1][j][1], outp[i + 1][j][2], 1.0);
			point4 d = point4(outp[i + 1][j + 1][0], outp[i + 1][j + 1][1], outp[i + 1][j + 1][2], 1.0);

			/*if (i == 0 && j == 0)
			{
			cout << a.x << " " << a.y << " " << a.z << endl;
			cout << b.x << " " << b.y << " " << b.z << endl;
			cout << c.x << " " << c.y << " " << c.z << endl;
			cout << d.x << " " << d.y << " " << d.z << endl;
			}*/

			quad(a, c, d, b);
		}
	}
}

//----------------------------------------------------------------------------

void
determinePoints(double x, double y, double z)
{
	if (points_j <= RESOLUTIONJ)
	{
		inp[points_i][points_j][0] = x;
		inp[points_i][points_j][1] = y;
		inp[points_i][points_j][2] = z;
		cout << x << " " << y << " " << z << endl;
		points_j++;
	}
	else
	{
		points_j = 0;
		points_i++;
		inp[points_i][points_j][0] = x;
		inp[points_i][points_j][1] = y;
		inp[points_i][points_j][2] = z;
		cout << x << " " << y << " " << z << endl;
		points_j++;
	}
	addPoints(point4(x, y, z, 1.0));

}

//----------------------------------------------------------------------------

//  Parametric Texture Mapping: white-black squares
// Reference: http://www.cosc.brocku.ca/Offerings/3P98/course/OpenGL/RedBookExamples/checker.c
void
makeCheckImage(void)
{
	int i, j, c;

	for (i = 0; i < checkImageHeight; i++) {
		for (j = 0; j < checkImageWidth; j++) {
			c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals)+sizeof(textures), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), sizeof(textures), textures);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader_a3.glsl", "fshader_a3.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	GLint texture = glGetAttribLocation(program, "texture_coordinate");
	glEnableVertexAttribArray(texture);
	glVertexAttribPointer(texture, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)+sizeof(normals)));

	// Initialize shader lighting parameters
	point4 light_position(0.0, 0.0, -1.0, 0.0);
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);

	color4 material_ambient(1.0, 0.0, 1.0, 1.0);
	color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
	color4 material_specular(1.0, 0.8, 0.0, 1.0);
	float  material_shininess = 100.0;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);
	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);

	phongShadingLocation = glGetUniformLocation(program, "PhongShading");
	wireframeLocation = glGetUniformLocation(program, "Wireframe");
	textureLocation = glGetUniformLocation(program, "Texture");
	bumpTextureLocation = glGetUniformLocation(program, "BumpTexture");
	environmentTextureLocation = glGetUniformLocation(program, "EnvironmentTexture");

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");


	makeCheckImage();

	// Initialize texture objects
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	GLuint texture_bump = LoadTexture("colour_map.raw", 256, 256);
	GLuint normal_texture = LoadTexture("normal_map.raw", 256, 256);
	LoadCubeTexture();

	glActiveTexture(GL_TEXTURE0);
	int tex_location = glGetUniformLocation(program, "tex");
	glUniform1i(tex_location, 0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glActiveTexture(GL_TEXTURE1);
	int texture_location = glGetUniformLocation(program, "color_texture");
	glUniform1i(texture_location, 1);
	glBindTexture(GL_TEXTURE_2D, texture_bump);

	glActiveTexture(GL_TEXTURE2);
	int normal_location = glGetUniformLocation(program, "normal_texture");
	glUniform1i(normal_location, 2);
	glBindTexture(GL_TEXTURE_2D, normal_texture);

	glActiveTexture(GL_TEXTURE3);
	int cubemap_location = glGetUniformLocation(program, "cubemap_texture");
	glUniform1i(cubemap_location, 3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureObj);


	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate tha model-view matrixn
	const vec3 viewer_pos(0.0, 0.0, 2.0);
	mat4  model_view = (Translate(-viewer_pos) * RotateX(Theta[Xaxis]) * RotateY(Theta[Yaxis]) * RotateZ(Theta[Zaxis]) * Scale(scale_number, scale_number, scale_number));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniform1i(phongShadingLocation, is_phong);
	glUniform1i(wireframeLocation, is_wireframe);
	glUniform1i(textureLocation, is_texture);
	glUniform1i(bumpTextureLocation, is_bump_texture);
	glUniform1i(environmentTextureLocation, is_environment_texture);

	// Draw bezier surface
	if (is_bezier)
	{
		if (is_wireframe == 1)
		{
			// For wireframe: draw only lines
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals)+sizeof(textures), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), sizeof(textures), textures);
	}
	// Display points
	else
	{
		glDrawArrays(GL_POINTS, 0, Index);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	}
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		switch (button)
		{
			case GLUT_LEFT_BUTTON:
			{
				point_x = (x - 256) / 256.0;
				point_y = (256 - y) / 256.0;
				point_z = 0;
				break;
			}
			case GLUT_MIDDLE_BUTTON:
			{
				break;
			}
			case GLUT_RIGHT_BUTTON:
			{
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------

void
idle(void)
{
	int currentWindow = glutGetWindow();
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutSetWindow(settingsWindow);
	glutPostRedisplay();
	glutSetWindow(currentWindow);

	if (is_rotate == 1)
	{
		Theta[Axis] += 0.5;
		if (Theta[Axis] > 360.0)
		{
			Theta[Axis] -= 360.0;
		}
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'w':
		point_z += 0.1;
		break;
	case 's':
		point_z -= 0.1;
		break;
	case 'a':
		determinePoints(point_x, point_y, point_z);
		break;
	case 'b':
		Index = 0;
		is_bezier = 1;
		bezierSurface();
		break;
	}
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat aspect = GLfloat(width) / height;
	mat4  projection = Perspective(45.0, aspect, 0.5, 3.0);

	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//std::cout << glGetString(GL_VERSION) << std::endl;
	//getchar();
	mainWindow = glutCreateWindow("Assignment 3 - Bezier Surfaces");
	glewExperimental = GL_TRUE;

	cout << glGetString(GL_VERSION) << endl;
	glewInit();
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);

	// Menu
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(10, 10);
	glutInitWindowPosition(1024, 0);
	glutInitContextVersion(2, 1);
	glutInitContextProfile(GLUT_INIT_PROFILE);
	settingsWindow = glutCreateWindow("Settings");
	GLUI *glui = GLUI_Master.create_glui("GLUI");
	glui->set_main_gfx_window(settingsWindow);

	// Gouraud Shading
	glui->add_button("Gouraud Shading", 1, (GLUI_Update_CB)gouraud_shading);
	glui->add_separator();
	glui->add_separator();
	// Phong Shading
	glui->add_button("Phong Shading", 2, (GLUI_Update_CB)phong_shading);
	glui->add_separator();
	glui->add_separator();
	// Wireframe
	glui->add_button("Wireframe", 3, (GLUI_Update_CB)wireframe);
	glui->add_separator();
	glui->add_separator();
	// Parametric Texture Mapping
	glui->add_column(true);
	glui->add_button("Parametric Texture Mapping", 4, (GLUI_Update_CB)parametricTextureMapping);
	glui->add_separator();
	glui->add_separator();
	// Bump Texture Mapping
	glui->add_button("Bump Texture", 5, (GLUI_Update_CB)bumpTexture);
	glui->add_separator();
	glui->add_separator();
	// Environment Texture Mapping
	glui->add_button("Environment Texture", 6, (GLUI_Update_CB)environmentTexture);
	glui->add_separator();
	glui->add_separator();
	// Rotate X
	glui->add_column(true);
	glui->add_button("Rotate X", 7, (GLUI_Update_CB)rotateX);
	glui->add_separator();
	glui->add_separator();
	// Rotate Y
	glui->add_button("Rotate Y", 8, (GLUI_Update_CB)rotateY);
	glui->add_separator();
	glui->add_separator();
	// Rotate Z
	glui->add_button("Rotate Z", 9, (GLUI_Update_CB)rotateZ);
	glui->add_separator();
	glui->add_separator();
	// Rotate Z
	glui->add_button("Stop Rotation", 10, (GLUI_Update_CB)stopRotation);
	glui->add_separator();
	glui->add_separator();
	// Zoom in
	glui->add_column(true);
	glui->add_button("Zoom in", 11, (GLUI_Update_CB)zoomIn);
	glui->add_separator();
	glui->add_separator();
	// Zoom in
	glui->add_button("Zoom out", 12, (GLUI_Update_CB)zoomOut);
	glui->add_separator();
	glui->add_separator();
	// Random Bezier
	glui->add_column(true);
	glui->add_button("Random", 13, (GLUI_Update_CB)random_generate);
	glui->add_separator();
	glui->add_separator();
	// Exit
	glui->add_button("Quit", 14, (GLUI_Update_CB)exitScreen);
	glui->add_separator();
	glui->add_separator();

	glutMainLoop();
	return 0;
}