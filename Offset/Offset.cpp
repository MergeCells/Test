#include "stdafx.h"
#include <gl/freeglut.h>
#include <math.h>

#include "BasicDef.h"

// STL�f�[�^�̃t�@�C�����D
//#define STL_FILE "sample.stl"
#define STL_FILE "blade_AP203.STL"

// ���f�����ώ@���鎋�_�ʒu�D
#define PHI 30.0
#define THETA 60.0
double phi = PHI;
double theta = THETA;

// �E�B���h�E�̏����ʒu�Ə����T�C�Y�D
#define INIT_X_POS 128
#define INIT_Y_POS 128
#define INIT_WIDTH 768
#define INIT_HEIGHT 768

// �z�肳���摜�̍ő�T�C�Y�D
#define MAX_WIDTH 2048
#define MAX_HEIGHT 1024

// �摜�̏c���T�C�Y�D
unsigned int window_width, window_height;

// ����~���`�𑽖ʑ̋ߎ�����ۂ̕������D
#define DIV 64

// �\�����[�h�D
#define DISPLAY_MODEL 0
#define DISPLAY_OFFSET 1
unsigned int display_mode = DISPLAY_MODEL;

// �I�t�Z�b�g���a�D
double radius = 1.0;

// �}�E�X�����D
int mouse_old_x, mouse_old_y;
bool motion_p;

// �_�D
extern double point[MAX_NUM_POINTS][3];
extern unsigned int num_points;

// �ӁD
extern unsigned int edge[MAX_NUM_EDGES][2];
extern unsigned int num_edges;

// �O�p�`�|���S���̒��_�D
extern unsigned int triangle[MAX_NUM_TRIANGLES][3];
extern unsigned int num_triangles;

// �摜�̃f�v�X�l�ƐF�f�[�^�D
GLfloat depth_data[MAX_WIDTH * MAX_HEIGHT];
GLubyte color_data[MAX_WIDTH * MAX_HEIGHT][4];

// �I�t�Z�b�g�ʏ�̓_�Q�̍��W�f�[�^�ƁC�Ή�����_�C�ӁC�|���S��
// �̃C���f�b�N�X�f�[�^�D
double offset_point[MAX_WIDTH * MAX_HEIGHT][3];
unsigned int offset_index[MAX_WIDTH * MAX_HEIGHT];

// STL�t�@�C���̓ǂݍ��݁D
extern bool loadSTLFile(const char* STL_file);

// ��{�I�Ȑ}�`�v�Z�D
extern void cross(double vec0[], double vec1[], double vec2[]);
extern void normVec(double vec[]);
extern void normal(double p0[], double p1[], double p2[], double normal[]);

// ��{�I�ȕ\���֐��D
extern void defineViewMatrix(double phi, double theta, unsigned int width, unsigned int height, double margin);
extern void displayModel(void);
extern void pixelCoordToModelCoord(int h, int v, float d, double point[]);

// �����̕`��D
void displaySphere(double radius)
// double radius; ���̔��a�D
{
	unsigned int i, j;
	double da, x0, y0, z0, x1, y1, z1, r0, r1, c, s;
	da = (2 * PI / DIV);
	for (i = 0; i < (DIV / 4 + 1); i++) {
		z0 = radius * cos(da * i);
		r0 = radius * sin(da * i);
		z1 = radius * cos(da * (i + 1));
		r1 = radius * sin(da * (i + 1));
		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= DIV; j++) {
			c = cos(da * (j % DIV));
			s = sin(da * (j % DIV));
			x0 = r0 * c;
			y0 = r0 * s;
			glNormal3d(x0 / radius, y0 / radius, z0 / radius);
			glVertex3d(x0, y0, z0);
			x1 = r1 * c;
			y1 = r1 * s;
			glNormal3d(x1 / radius, y1 / radius, z1 / radius);
			glVertex3d(x1, y1, z1);
		}
		glEnd();
	}
}

// �~���`�̕`��D
void displayCylinder(double start[], double end[], double radius)
// double start[], end[]; �~���`�̗��[�_�D
// double radius; �~���`�̔��a�D
{
	int i;
	double x_axis[3], y_axis[3], z_axis[3], ref[3], tmp[3];
	double da, c, s, x, y, z;

	// �~���`�̒��S����z���Ƃ��郍�[�J�����W�n�̐ݒ�D
	for (i = 0; i < 3; i++)
		z_axis[i] = end[i] - start[i];
	normVec(z_axis);
	ref[X] = 1.0;
	ref[Y] = ref[Z] = 0.0;
	cross(z_axis, ref, tmp);

	// z_axis��ref�����s�ȃP�[�X�D
	if ((fabs(tmp[X]) < EPS) && (fabs(tmp[Y]) < EPS) && (fabs(tmp[Z]) < EPS)) {
		ref[Y] = 1.0;
		ref[X] = ref[Z] = 0.0;
		cross(z_axis, ref, tmp);
	}
	cross(tmp, z_axis, x_axis);
	normVec(x_axis);
	cross(z_axis, x_axis, y_axis);

	// �~���`���l�ӌ`�̘A���ŕ`���D
	da = (2 * PI / DIV);
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= DIV; i++) {
		c = cos(da * (i % DIV));
		s = sin(da * (i % DIV));
		x = radius * (c * x_axis[X] + s * y_axis[X]);
		y = radius * (c * x_axis[Y] + s * y_axis[Y]);
		z = radius * (c * x_axis[Z] + s * y_axis[Z]);
		glNormal3d(x / radius, y / radius, z / radius);
		glVertex3d(x + end[X], y + end[Y], z + end[Z]);
		glVertex3d(x + start[X], y + start[Y], z + start[Z]);
	}
	glEnd();
}

// �X���u�`��̕`��D
void displaySlab(double p0[], double p1[], double p2[], double radius)
// double p0[], p1[], p2[]; �|���S���̎��͂�3�_�D�����v���������D
// double radius; �|���S���̃V�t�g�ʁD
{
	double nrml_vec[3];

	// �|���S���̖@�������̎Z�o�D
	normal(p0, p1, p2, nrml_vec);

	// ������ɃV�t�g�����|���S���D
	glNormal3dv(nrml_vec);
	glVertex3d(p0[X] + radius * nrml_vec[X], p0[Y] + radius * nrml_vec[Y], p0[Z] + radius * nrml_vec[Z]);
	glVertex3d(p1[X] + radius * nrml_vec[X], p1[Y] + radius * nrml_vec[Y], p1[Z] + radius * nrml_vec[Z]);
	glVertex3d(p2[X] + radius * nrml_vec[X], p2[Y] + radius * nrml_vec[Y], p2[Z] + radius * nrml_vec[Z]);

	// �������ɃV�t�g�����|���S���D
	glNormal3d(- nrml_vec[X], - nrml_vec[Y], - nrml_vec[Z]);
	glVertex3d(p2[X] - radius * nrml_vec[X], p2[Y] - radius * nrml_vec[Y], p2[Z] - radius * nrml_vec[Z]);
	glVertex3d(p1[X] - radius * nrml_vec[X], p1[Y] - radius * nrml_vec[Y], p1[Z] - radius * nrml_vec[Z]);
	glVertex3d(p0[X] - radius * nrml_vec[X], p0[Y] - radius * nrml_vec[Y], p0[Z] - radius * nrml_vec[Z]);
}

// �w�肵��ID�ɑΉ������F(r, g, b)��^���鏈���D
void IDToColor(unsigned int id)
// unsigned int id; ID�l�D
{
	GLubyte r, g, b;
	int r_id;
	r = (GLubyte)(id / 65536); // 65536 = 256 * 256�D
	r_id = id % 65536;
	g = (GLubyte)(r_id / 256);
	b = (GLubyte)(r_id % 256);
	glColor3ub(r, g, b);
}

// �F����Ή�����ID�𓾂鏈���D
unsigned int colorToID(GLubyte r, GLubyte g, GLubyte b)
{
	return(r * 65536 + g * 256 + b);
}

// ���̃f�B�X�v���[���X�g�D
GLuint sphere_list = 1;

// �I�t�Z�b�g�`��̎Z�o�D
void compOffset(unsigned int width, unsigned int height, double offset)
// unsigned int width, height; �E�B���h�E�̉𑜓x�D
// double offset; �I�t�Z�b�g�ʁD
{
	unsigned int i, v, h, shift;

	// ���e�s��̌v�Z�D���̂�^�ォ�猩���낷�D
	defineViewMatrix(270.0, 90.0, width, height, offset);

	// �t���[���o�b�t�@�̏������D
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// �Ɩ��̏����D
	glDisable(GL_LIGHTING);

	// �f�B�X�v���[���X�g�̐����D
	glNewList(sphere_list, GL_COMPILE);
	displaySphere(offset);
	glEndList();

	// �X���u�`��̕`��D
	glBegin(GL_TRIANGLES);
	for (i = 0; i < num_triangles; i++) {
		IDToColor(i);
		displaySlab(point[triangle[i][0]], point[triangle[i][1]], point[triangle[i][2]], offset);
	}
	glEnd();
	if (offset > 0.0) {

		// ���ʂ̕`��D
		for (i = 0; i < num_points; i++) {
			IDToColor(i + num_triangles);
			glPushMatrix();
			glTranslated(point[i][X], point[i][Y], point[i][Z]);
			glCallList(sphere_list);
			glPopMatrix();
		}

		// �~���`�̕`��D
		for (i = 0; i < num_edges; i++) {
			IDToColor(i + num_triangles + num_points);
			displayCylinder(point[edge[i][0]], point[edge[i][1]], offset);
		}
	}
	glFlush();

	// �f�B�X�v���[���X�g�̍폜�D
	glDeleteLists(sphere_list, 1);

	// �e�s�N�Z���̐F�f�[�^�ƃf�v�X�l���擾�D
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, color_data);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth_data); 
	
	// �Ή�������W�l�Ɠ_�C�ӁC�|���S���̃C���f�b�N�X���L�^�D
	for (v = 0; v < height; v++) {
		for (h = 0; h < width; h++) {
			shift = width * v + h;
			offset_index[shift] = colorToID(color_data[shift][0], color_data[shift][1], color_data[shift][2]);
			if (offset_index[shift] < (num_points + num_edges + num_triangles))
				pixelCoordToModelCoord(h, v, depth_data[shift], offset_point[shift]);
		}
	}
}

// �i�q��̓_�Q�Œ�`���ꂽ�Ȗʂ̕`��D
void displayGridSurface(unsigned int width, unsigned int height)
// unsigned int width, height; �i�q�̉𑜓x�D
{
	unsigned int i, j, shift;
	double nrml_vec[3];

	// �t���[���o�b�t�@�̏������D
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// �Ɩ��̓_���D
	glEnable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	for (i = 0; i < (height - 1); i++) {
		for (j = 0; j < (width - 1); j++) {
			shift = width * i + j;

			// �אڂ���4�̊i�q�̑S�ĂŃI�t�Z�b�g�ʂ̃f�[�^���v�Z����Ă���D
			if ((offset_index[shift] < (num_points + num_edges + num_triangles))
				&& (offset_index[shift + 1] < (num_points + num_edges + num_triangles))
				&& (offset_index[shift + width] < (num_points + num_edges + num_triangles))
				&& (offset_index[shift + width + 1] < (num_points + num_edges + num_triangles))) {

				// 1���ڂ̃|���S���D
				normal(offset_point[shift], offset_point[shift + 1], offset_point[shift + width], nrml_vec);
				glNormal3dv(nrml_vec);
				glVertex3dv(offset_point[shift]);
				glVertex3dv(offset_point[shift + 1]);
				glVertex3dv(offset_point[shift + width]);

				// 2���ڂ̃|���S���D
				normal(offset_point[shift + 1], offset_point[shift + width + 1], offset_point[shift + width], nrml_vec);
				glNormal3dv(nrml_vec);
				glVertex3dv(offset_point[shift + 1]);
				glVertex3dv(offset_point[shift + width + 1]);
				glVertex3dv(offset_point[shift + width]);
			}
		}
	}
	glEnd();
	glFlush();
}

// �\���D
void display(void)
{
	switch (display_mode) {
		case DISPLAY_MODEL:
			defineViewMatrix(phi, theta, window_width, window_height, radius);
			displayModel();
			break;
		case DISPLAY_OFFSET:
			defineViewMatrix(phi, theta, window_width, window_height, radius);
			displayGridSurface(window_width, window_height);
			break;
		default:
			break;
	}
}

// �E�B���h�E�T�C�Y�̕ύX�D
void resize(int width, int height)
{

	// �E�B���h�E�T�C�Y�̎擾�D
	window_width = width;
	window_height = height;

	// �E�B���h�E�T�C�Y���ς��ƃI�t�Z�b�g�v�Z�̌��ʂ͕s���ƂȂ�D
	display_mode = DISPLAY_MODEL;
}

// �L�[�����D
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'q':
		case 'Q':
		case '\033':
			exit(0);

			// STL�t�@�C���̓ǂݍ��݁D
		case 'r':
		case 'R':
			if (!loadSTLFile(STL_FILE))
				break;
			display_mode = DISPLAY_MODEL;
			glutPostRedisplay();
			break;

			// STL���f���̕\���D
		case 'd':
		case 'D':
			display_mode = DISPLAY_MODEL;
			glutPostRedisplay();
			break;

			// �I�t�Z�b�g�ʂ̐����D
		case 'c':
		case 'C':
			compOffset(window_width, window_height, radius);
			display_mode = DISPLAY_OFFSET;
			glutPostRedisplay();
			break;

			// �I�t�Z�b�g�ʂ̕\���D
		case 'o':
		case 'O':
			display_mode = DISPLAY_OFFSET;
			glutPostRedisplay();
			break;
		default:
			break;
	}
}

// �}�E�X�̃{�^�������D
void mouse_button(int button, int state, int x, int y)
{
	if ((state == GLUT_DOWN) && (button == GLUT_LEFT_BUTTON))
		motion_p = true;
	else if (state == GLUT_UP)
		motion_p = false;
	mouse_old_x = x;
	mouse_old_y = y;
}

// �}�E�X�̈ړ������D
void mouse_motion(int x, int y)
{
	int dx, dy;
	dx = x - mouse_old_x;
	dy = y - mouse_old_y;
	if (motion_p) {
		phi -= dx * 0.2;
		theta += dy * 0.2;
	}
	mouse_old_x = x;
	mouse_old_y = y;
	glutPostRedisplay();
}

// OpenGL�֌W�̏����ݒ�D
void initGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_LIGHT0);
	glEnable(GL_CULL_FACE); // �J�����O��L���ɂ���D
	glCullFace(GL_BACK); // ���ʂ͖����D
}

int main(int argc, char *argv[])
{
	glutInitWindowPosition(INIT_X_POS, INIT_Y_POS);
	glutInitWindowSize(INIT_WIDTH, INIT_HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Offset");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse_button);
	glutMotionFunc(mouse_motion);
	initGL();
	glutMainLoop();
	return 0;
}

