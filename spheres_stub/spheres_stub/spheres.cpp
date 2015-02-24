#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#define NATOMS 1000
#define ROTATE 1

#define RENDER_SPHERES_INSTEAD_OF_VERTICES 0

float light0_position[4] = {1,1,1,0};
const float camera[] = {0.0,0.0,5.0};
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 800;
float angle = 0.72;
std::vector<std::vector<float> > atoms;
float cur_camera[] = {0,0,0};
GLuint  prog_hdlr;
GLint location_attribute_0,location_ogf_uniform_0;

float randMinusOneOne() {
	return (float)rand()/(float)RAND_MAX*(rand()>RAND_MAX/2?1:-1);
}

void render_scene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	cur_camera[0] = cos(angle)*camera[0]+sin(angle)*camera[2];
	cur_camera[1] = camera[1];
	cur_camera[2] = cos(angle)*camera[2]-sin(angle)*camera[0];
#if ROTATE
	angle+=0.01;
#endif
	glLoadIdentity();
	gluLookAt(cur_camera[0], cur_camera[1], cur_camera[2], 0.0,  0.0, 0.0, 0.0f, 1.0f, 0.0f);
#if !RENDER_SPHERES_INSTEAD_OF_VERTICES
	glUseProgram(prog_hdlr);

	GLfloat viewport[4];
	glGetFloatv(GL_VIEWPORT, viewport);
	glUniform4fv(location_ogf_uniform_0, 1, viewport);
	glBegin(GL_POINTS);
	for (int i=0; i<NATOMS; i++) {
		glColor3f(atoms[i][4], atoms[i][5], atoms[i][6]);
		glVertexAttrib1f(location_attribute_0, atoms[i][3]);
		glVertex3f(atoms[i][0], atoms[i][1], atoms[i][2]);
	}
	glEnd();
	glUseProgram(0);
#else
	for (int i=0; i<NATOMS; i++) {
		glColor3f(atoms[i][4], atoms[i][5], atoms[i][6]);
		glPushMatrix();
		glTranslatef(atoms[i][0], atoms[i][1], atoms[i][2]);
		glutSolidSphere(atoms[i][3], 32, 32);
		glPopMatrix();
	}
#endif
	glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {
	if (27==key) {
		exit(0);
	}
}

void changeSize(int w, int h) {
//	float ratio = (1.0*w)/(!h?1:h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glViewport(0, 0, w, h);
//	gluPerspective(45,ratio,1,100);
	glOrtho(-1,1,-1,1,-1,8);
	glMatrixMode(GL_MODELVIEW);
}

void printInfoLog(GLuint obj) {
	int log_size = 0;
	int bytes_written = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_size);
	if (!log_size) return;
	char *infoLog = new char[log_size];
	glGetProgramInfoLog(obj, log_size, &bytes_written, infoLog);
	std::cerr << infoLog << std::endl;
	delete [] infoLog;
}


bool read_n_compile_shader(const char *filename, GLuint &hdlr, GLenum shaderType) {
	std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (!is.is_open()) {
		std::cerr << "Unable to open file " << filename << std::endl;
		return false;
	}
	long size = is.tellg();
	char *buffer = new char[size+1];
	is.seekg(0, std::ios::beg);
	is.read (buffer, size);
	is.close();
	buffer[size] = 0;

	hdlr = glCreateShader(shaderType);
	glShaderSource(hdlr, 1, (const GLchar**)&buffer, NULL);
	glCompileShader(hdlr);
	std::cerr << "info log for " << filename << std::endl;
	printInfoLog(hdlr);
	delete [] buffer;
	return true;
}

void setShaders(GLuint &prog_hdlr, const char *vsfile, const char *fsfile) {
	GLuint vert_hdlr, frag_hdlr;
	read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
	read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);

	prog_hdlr = glCreateProgram();
	glAttachShader(prog_hdlr, frag_hdlr);
	glAttachShader(prog_hdlr, vert_hdlr);

	glLinkProgram(prog_hdlr);
	std::cerr << "info log for the linked program" << std::endl;
	printInfoLog(prog_hdlr);
}


int main(int argc, char **argv) {
	for (int i=0; i<NATOMS; i++) {
		std::vector<float> tmp;
		for (int c=0; c<4; c++) {
			tmp.push_back(randMinusOneOne()/2); // xyz
		}
		tmp[3] = (randMinusOneOne()+2.0)/16.0; // radius
		for (int c=4; c<7; c++) {
			tmp.push_back((float)rand()/(float)RAND_MAX); // rgb
		}
		atoms.push_back(tmp);
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutCreateWindow("");
	glClearColor(0.0,0.0,1.0,1.0);
	glutDisplayFunc(render_scene);
#if ROTATE
	glutIdleFunc(render_scene);
#endif
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glEnable(GL_TEXTURE_2D);

	glewInit();
#if !RENDER_SPHERES_INSTEAD_OF_VERTICES
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
		std::cout << "Ready for GLSL - vertex, fragment, and geometry units" << std::endl;
	else {
		std::cout << "No GLSL support" << std::endl;
		exit(1);
	}
	setShaders(prog_hdlr, "shaders/spheres.vs", "shaders/spheres.fs");

	location_attribute_0   = glGetAttribLocation(prog_hdlr, "attr");	// radius
	location_ogf_uniform_0 = glGetUniformLocation(prog_hdlr, "ogf_uniform_0"); // viewport

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
	glutMainLoop();
	return 0;
}