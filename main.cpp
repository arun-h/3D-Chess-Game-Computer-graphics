#include <GL/glut.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

#define DIV4 0.25
#define DIV8 0.125
#define DIV16 0.0625
#define DIV32 0.03125

#define PICK_TOL 1
#define PICK_BUFFER_SIZE 256
unsigned int PickBuffer[PICK_BUFFER_SIZE];

#define max		7
#define CHESS_BOARD	0
#define BOTTOM_WOOD	1
#define SIDE_WOOD	2
#define WHITE_WOOD	3
#define BLACK_WOOD	4
#define WHITE_WOOD_C	5
#define BLACK_WOOD_C	6
	#define whiteAskari0	0
	#define whiteAskari1	1
	#define whiteAskari2	2
	#define whiteAskari3	3
	#define whiteAskari4	4
	#define whiteAskari5	5
	#define whiteAskari6	6
	#define whiteAskari7	7
	#define blackAskari0	8
	#define blackAskari1	9
	#define blackAskari2	10
	#define blackAskari3	11
	#define blackAskari4	12
	#define blackAskari5	13
	#define blackAskari6	14
	#define blackAskari7	15
	#define whiteFeel0	16
	#define whiteFeel1	17
	#define blackFeel0	18
	#define blackFeel1	19
	#define whiteHosan0	20
	#define whiteHosan1	21
	#define blackHosan0	22
	#define blackHosan1	23
	#define whiteTabya0	24
	#define whiteTabya1	25
	#define blackTabya0	26
	#define blackTabya1	27
	#define whiteMalek	28
	#define blackMalek	29
	#define whiteWazeer	30
	#define blackWazeer	31

using namespace std;

GLuint	texture[max];		// Storage for 3 textures.
struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};

int ImageLoad(const char *filename, Image *image)
{
	FILE *file;
	unsigned long size;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;          // number of planes in image (must be 1)
	unsigned short int bpp;             // number of bits per pixel (must be 24)
	char temp;                          // used to convert bgr to rgb color.
	// Make sure the file exists
	if ((file = fopen(filename, "rb"))==NULL)
	{
		printf("File Not Found : %s\n",filename);
		return 0;
	}
	// Skip to bmp header
	fseek(file,18, SEEK_CUR);
	// read width
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n",filename, image->sizeX);
	//read the height
	if ((i = fread(&image->sizeY,4,1,file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, image->sizeY);
	// calculate the size (assuming 24 bpp)
	size = image->sizeX * image->sizeY * 3;
	// read the planes
	if ((fread(&planes, 2, 1, file)) != 1) {
		printf("Error reading planes from %s. \n", filename);
		return 0;
	}
	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}
	// read the bpp
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s. \n", filename);
		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}
	// seek past the rest of the bitmap header
	fseek(file, 24, SEEK_CUR);
	// Read the data
	image->data = (char *) malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for colour-corrected image data");
		return 0;
	}
	if ((i = fread(image->data,size,1,file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}
	// reverse all of the colours bgr => rgb)
	for (i=0;i<size;i+=3) {
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
	}
	return 1;
}

void LoadGLTexture(const char *filename, int textNum)
{
	// Stores the texture
	Image *image1;

	// Allocate space for texture
	image1 = (Image *) malloc(sizeof(Image));
	if (image1 == NULL) {
		cout<<"Error allocating space for image"<<endl;
		exit(0);
	}

	if (!ImageLoad(filename, image1)) {
		exit(1);
	}

	// create Texture
	glGenTextures(1, &texture[textNum]);

	// texture 2 (linear scaling)
	glBindTexture(GL_TEXTURE_2D, texture[textNum]);   // 2d texture (x and y size)
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
}


void LoadGLTextures()
{
	LoadGLTexture("BWTop.bmp",CHESS_BOARD);
	LoadGLTexture("woodBottom.bmp",BOTTOM_WOOD);
	LoadGLTexture("wood.bmp",SIDE_WOOD);
	LoadGLTexture("Wwood.bmp",WHITE_WOOD);
	LoadGLTexture("Bwood.bmp",BLACK_WOOD);
	LoadGLTexture("WwoodChoice.bmp",WHITE_WOOD_C);
	LoadGLTexture("BwoodChoice.bmp",BLACK_WOOD_C);
}

int Board[8][8],RenderMode,charsPosition[32];
bool flipped=false;

bool whiteChar(int character)
{
	return ((character<=whiteAskari7 && character>=whiteAskari0)?true:((character<=blackAskari7)?false:((character<=whiteFeel1)?true:((character<=blackFeel1)?false:((character<=whiteHosan1)?true:((character<=blackHosan1)?false:((character<=whiteTabya1)?true:((character<=blackTabya1)?false:((character==whiteMalek)?true:((character<=blackMalek)?false:((character<=whiteWazeer)?true:false)))))))))));
}
void drawBoard(int chosen)
{
	if(RenderMode==GL_SELECT)
	for(int i=0;i<8;i++)for(int j=0;j<8;j++)
	{
		int num=8*i+j+32;
		glLoadName(num);
		glTranslatef(0.25*j,DIV32/4.0,0.25*i);
		glBegin(GL_QUADS);
		glVertex3f(-1,  0.125, -1);
		glVertex3f(-1,  0.125,  -1+0.25);
		glVertex3f(-1+0.25,  0.125,  -1+0.25);
		glVertex3f( -1+0.25,  0.125, -1);
		glEnd();
		glTranslatef(-0.25*j,-DIV32/4.0,-0.25*i);
	}
	if(RenderMode==GL_RENDER)
	{
		glBindTexture(GL_TEXTURE_2D, texture[SIDE_WOOD]);
		glBegin(GL_QUADS);
			// Front Face (note that the texture's corners have to match the quad's corners)
			glNormal3f( 0.0f, 0.0f, 1.0f);					// front face points out of the screen on z.
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -0.125f,  1.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -0.125f,  1.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  0.125f,  1.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  0.125f,  1.0f);	// Top Left Of The Texture and Quad
		glEnd();

		glBindTexture(GL_TEXTURE_2D, texture[SIDE_WOOD]);
		glBegin(GL_QUADS);
			// Back Face
			glNormal3f( 0.0f, 0.0f,-1.0f);					// back face points into the screen on z.
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -0.125f, -1.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  0.125f, -1.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  0.125f, -1.0f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -0.125f, -1.0f);	// Bottom Left Of The Texture and Quad
		glEnd();

		glBindTexture(GL_TEXTURE_2D, texture[CHESS_BOARD]);
		glBegin(GL_QUADS);
			// Top Face
			//glNormal3f( 0.0f, 1.0f, 0.0f);					// top face points up on y.
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  0.125f, -1.0f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  0.125f,  1.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  0.125f,  1.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  0.125f, -1.0f);	// Top Right Of The Texture and Quad
		glEnd();

		glBindTexture(GL_TEXTURE_2D, texture[BOTTOM_WOOD]);
		glBegin(GL_QUADS);
			// Bottom Face
			glNormal3f( 0.0f, -1.0f, 0.0f);					// bottom face points down on y.
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -0.125f, -1.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -0.125f, -1.0f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -0.125f,  1.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -0.125f,  1.0f);	// Bottom Right Of The Texture and Quad
		glEnd();

		glBindTexture(GL_TEXTURE_2D, texture[SIDE_WOOD]);
		glBegin(GL_QUADS);
			// Right face
			glNormal3f( 1.0f, 0.0f, 0.0f);					// right face points right on x.
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -0.125f, -1.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  0.125f, -1.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  0.125f,  1.0f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -0.125f,  1.0f);	// Bottom Left Of The Texture and Quad

			// Left Face
			glNormal3f(-1.0f, 0.0f, 0.0f);					// left face points left on x.
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -0.125f, -1.0f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -0.125f,  1.0f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  0.125f,  1.0f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  0.125f, -1.0f);	// Top Left Of The Texture and Quad

		glEnd();
	}
}

void drawAskari(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);

	glTranslatef(x,y,z);		// draw in required position
	if(blackWhite)glRotatef(180,0,1,0);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
				glRotatef(90,1,0,0);
					glutSolidTorus(DIV16-0.05,DIV16,64,64);
				glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
				glRotatef(90,1,0,0);
					glutSolidTorus(DIV16-0.05,DIV16,64,64);
				glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);
		}
		//objects
		GLUquadricObj *AskariSphere=gluNewQuadric(),*AskariCylinder=gluNewQuadric(),*AskariCone=gluNewQuadric();
		//activate Texture
		gluQuadricTexture(AskariSphere,true);gluQuadricTexture(AskariCylinder,true);gluQuadricTexture(AskariCone,true);

		//head
		glTranslatef(0,DIV8,0);
		gluSphere(AskariSphere,DIV16-0.02,64,64);
		glTranslatef(0,-DIV8,0);
		//body
		glTranslatef(0,DIV8,0);
		glRotatef(90,1,0,0);
		gluCylinder(AskariCone,DIV32,DIV16,DIV8,64,64);
		glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV8,0);


		glRotatef(90,1,0,0);
		gluCylinder(AskariCylinder,DIV16,DIV16,DIV16,64,64);
		glRotatef(-90,1,0,0);
	if(blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);
}

void drawWazeer(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);
	glTranslatef(x,y,z);		// draw in required position
	if(blackWhite)glRotatef(180,0,1,0);
		GLUquadricObj *WazeerSphere=gluNewQuadric(),*WazeerCylinder=gluNewQuadric(),*WazeerCone=gluNewQuadric();
		gluQuadricTexture(WazeerSphere,true);gluQuadricTexture(WazeerCylinder,true);gluQuadricTexture(WazeerCone,true);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.1,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.1,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);

			glDisable(GL_TEXTURE_2D);
			glTranslatef(0,DIV4+0.07125,0);
				glRotatef(90,1,0,0);
					glTranslatef(0,0.04,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0,-0.04,0);
					glTranslatef(0.04,0,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.04,0,0);
					glTranslatef(0.03,0.03,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.03,-0.03,0);
					glTranslatef(0,-0.04,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0,0.04,0);
					glTranslatef(-0.04,0,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.04,0,0);
					glTranslatef(-0.03,-0.03,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.03,0.03,0);
					glTranslatef(0.03,-0.03,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.03,0.03,0);
					glTranslatef(-0.03,0.03,0);
						gluCylinder(WazeerCylinder,0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.03,-0.03,0);
			//glutSolidTorus(DIV32-0.001,DIV16-0.01,4,4);
				glRotatef(-90,1,0,0);
			glTranslatef(0,-DIV4-0.07125,0);
			glEnable(GL_TEXTURE_2D);
		}
		glTranslatef(0,DIV4,0);
			gluSphere(WazeerSphere,DIV16,64,64);
		glTranslatef(0,-DIV4,0);

		glTranslatef(0,DIV4,0);
			glRotatef(90,1,0,0);
				gluCylinder(WazeerCone,DIV32,DIV16,DIV4,64,64);
			glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV4,0);

		glRotatef(90,1,0,0);
			gluCylinder(WazeerCylinder,DIV16,DIV16,DIV8,64,64);
		glRotatef(-90,1,0,0);
	if(blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);
}

void drawMalek(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);
	glTranslatef(x,y,z);		// draw in required position
	if(blackWhite)glRotatef(180,0,1,0);
		GLUquadricObj *WazeerSphere=gluNewQuadric(),*WazeerCylinder=gluNewQuadric(),*WazeerCone=gluNewQuadric();
		gluQuadricTexture(WazeerSphere,true);gluQuadricTexture(WazeerCylinder,true);gluQuadricTexture(WazeerCone,true);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV8-0.1,DIV8,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV8-0.1,DIV8,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);

			//crown spikes
			glDisable(GL_TEXTURE_2D);
			glTranslatef(0,DIV4+0.06125,0);
				glRotatef(90,1,0,0);
					glTranslatef(0,0.04,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0,-0.04,0);
					glTranslatef(0.04,0,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.04,0,0);
					glTranslatef(0.03,0.03,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.03,-0.03,0);
					glTranslatef(0,-0.04,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0,0.04,0);
					glTranslatef(-0.04,0,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.04,0,0);
					glTranslatef(-0.03,-0.03,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.03,0.03,0);
					glTranslatef(0.03,-0.03,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(-0.03,0.03,0);
					glTranslatef(-0.03,0.03,0);
						gluCylinder(WazeerCylinder,DIV32/4.0,DIV32/4.0,DIV32,64,64);
					glTranslatef(0.03,-0.03,0);
				glRotatef(-90,1,0,0);
			glTranslatef(0,-DIV4-0.06125,0);
			glEnable(GL_TEXTURE_2D);

		}

		glTranslatef(0,DIV4,0);
			gluSphere(WazeerSphere,DIV16,64,64);
		glTranslatef(0,-DIV4,0);

		glTranslatef(0,DIV4,0);
			glRotatef(90,1,0,0);
				gluCylinder(WazeerCone,DIV32,DIV8,DIV4,64,64);
			glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV4,0);

		glRotatef(90,1,0,0);
			gluCylinder(WazeerCylinder,DIV8,DIV8,DIV8,64,64);
		glRotatef(-90,1,0,0);
	if(blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);
}


void drawTabya(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);
	glTranslatef(x,y,z);		// draw in required position
	if(blackWhite)glRotatef(180,0,1,0);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(/*DIV16-0.05*/DIV32,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.05,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);
		}
		GLUquadricObj *AskariSphere=gluNewQuadric(),*AskariCylinder=gluNewQuadric(),*AskariCone=gluNewQuadric();
		gluQuadricTexture(AskariSphere,true);gluQuadricTexture(AskariCylinder,true);gluQuadricTexture(AskariCone,true);


		glTranslatef(0,DIV8,0);
			glRotatef(90,1,0,0);
				gluCylinder(AskariCone,DIV16-0.01,DIV16,DIV8,64,64);
				glTranslatef(0,0,-DIV16);
					gluDisk(AskariSphere,0,DIV16,64,64);
					gluCylinder(AskariSphere,DIV16,DIV16,DIV16,64,64);
				glTranslatef(0,0,DIV16);
				gluDisk(AskariSphere,0,DIV16,64,64);
			glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV8,0);


		glRotatef(90,1,0,0);
			gluCylinder(AskariCylinder,DIV16,DIV16,DIV16,64,64);
		glRotatef(-90,1,0,0);
	if(blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);
}

void drawHosan(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);
	glTranslatef(x,y,z);		// draw in required position
	if(flipped ^ blackWhite)glRotatef(180,0,1,0);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(/*DIV16-0.05*/DIV32,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.05,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);
		}

		GLUquadricObj *AskariSphere=gluNewQuadric(),*AskariCylinder=gluNewQuadric(),*AskariCone=gluNewQuadric();
		gluQuadricTexture(AskariSphere,true);gluQuadricTexture(AskariCylinder,true);gluQuadricTexture(AskariCone,true);

	//neck
		glTranslatef(0,DIV8,0);
			glRotatef(90,1,0,0);
				glRotatef(15,0,1,0);

					glTranslatef(0,0,-DIV8);
							gluCylinder(AskariSphere,0,DIV16-0.01,DIV8,64,64);
					glTranslatef(0,0,DIV8);
				glRotatef(-15,0,1,0);

				glRotatef(-15,0,1,0);

					glTranslatef(0,0,-DIV8);
							gluCylinder(AskariSphere,0,DIV16-0.01,DIV8,64,64);
					glTranslatef(0,0,DIV8);
				glRotatef(15,0,1,0);
			glRotatef(-90,1,0,0);

			//eyes
			glDisable(GL_TEXTURE_2D);
						glTranslatef(DIV32,DIV32,DIV32+0.01);
							gluSphere(AskariSphere,DIV32/4.0,64,64);
						glTranslatef(-DIV32,-DIV16,-DIV32-0.01);
						glTranslatef(-DIV32,DIV16,DIV32+0.01);
							gluSphere(AskariSphere,DIV32/4.0,64,64);
						glTranslatef(DIV32,-DIV32,-DIV32-0.01);
			glEnable(GL_TEXTURE_2D);

			//end of mouth
			glTranslatef(0,-DIV32-0.01,DIV8-0.01);
				gluSphere(AskariSphere,DIV32,64,64);
			glTranslatef(0,DIV32+0.01,-DIV8+0.01);

			//face
			glRotatef(200,1,0,0);
				glTranslatef(0,0,-DIV8);
						gluCylinder(AskariSphere,DIV32,DIV16-0.01,DIV8,64,64);
				glTranslatef(0,0,DIV8);
			glRotatef(-200,1,0,0);

			glRotatef(90,1,0,0);
				gluCylinder(AskariCone,DIV16-0.01,DIV16,DIV8,64,64);
			glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV8,0);

		glRotatef(90,1,0,0);
		gluCylinder(AskariCylinder,DIV16,DIV16,DIV16,64,64);
		glRotatef(-90,1,0,0);
	if(flipped ^ blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);

}

void drawFeel(float x, float y, float z, bool blackWhite, bool chosen)
{
	glBindTexture(GL_TEXTURE_2D, texture[((blackWhite)?((chosen)?WHITE_WOOD_C:WHITE_WOOD):((chosen)?BLACK_WOOD_C:BLACK_WOOD))]);
	glTranslatef(x,y,z);		// draw in required position
	if(blackWhite)glRotatef(180,0,1,0);
		if(RenderMode==GL_RENDER)
		{
			//base
			glTranslatef(0,-0.0625,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.1,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.0625,0);
			glTranslatef(0,-0.033,0);
					glRotatef(90,1,0,0);
						glutSolidTorus(DIV16-0.1,DIV16,64,64);
					glRotatef(-90,1,0,0);
			glTranslatef(0,0.033,0);
		}

		GLUquadricObj *WazeerSphere=gluNewQuadric(),*WazeerCylinder=gluNewQuadric(),*WazeerCone=gluNewQuadric();
		gluQuadricTexture(WazeerSphere,true);gluQuadricTexture(WazeerCylinder,true);gluQuadricTexture(WazeerCone,true);

		glTranslatef(0,DIV4-0.02,0);
			gluSphere(WazeerSphere,DIV16-0.02,64,64);
		glTranslatef(0,-DIV4+0.02,0);

		glTranslatef(0,DIV4-0.02,0);
			glRotatef(90,1,0,0);
				gluCylinder(WazeerCone,DIV32,DIV16,DIV4-0.02,64,64);
			glRotatef(-90,1,0,0);
		glTranslatef(0,-DIV4+0.02,0);

		glRotatef(90,1,0,0);
			gluCylinder(WazeerCylinder,DIV16,DIV16,DIV8-0.02,64,64);
		glRotatef(-90,1,0,0);
	if(blackWhite)glRotatef(-180,0,1,0);
	glTranslatef(0-x,0-y,0-z);
}

void drawChars(bool initialize, int choice)
{
	float XX=-1+0.125,YY=0.25-DIV16,ZZ=-1+0.125;
	if(initialize)
	{
		Board[7][4]=whiteMalek;charsPosition[whiteMalek]=60;
		Board[0][4]=blackMalek;charsPosition[blackMalek]=4;
		Board[7][3]=whiteWazeer;charsPosition[whiteWazeer]=59;
		Board[0][3]=blackWazeer;charsPosition[blackWazeer]=3;
		Board[7][2]=whiteFeel0;charsPosition[whiteFeel0]=58;Board[7][5]=whiteFeel1;charsPosition[whiteFeel1]=61;
		Board[0][2]=blackFeel0;charsPosition[blackFeel0]=2;Board[0][5]=blackFeel1;charsPosition[blackFeel1]=5;
		Board[7][1]=whiteHosan0;charsPosition[whiteHosan0]=57;Board[7][6]=whiteHosan1;charsPosition[whiteHosan1]=62;
		Board[0][1]=blackHosan0;charsPosition[blackHosan0]=1;Board[0][6]=blackHosan1;charsPosition[blackHosan1]=6;
		Board[7][0]=whiteTabya0;charsPosition[whiteTabya0]=56;Board[7][7]=whiteTabya1;charsPosition[whiteTabya1]=63;
		Board[0][0]=blackTabya0;charsPosition[blackTabya0]=0;Board[0][7]=blackTabya1;charsPosition[blackTabya1]=7;
		for(int i=0;i<8;i++)
		{
			Board[6][i]=i;charsPosition[whiteAskari0+i]=6*8+i;
			Board[1][i]=i+8;charsPosition[blackAskari0+i]=1*8+i;
		}
		for(int i=2;i<6;i++)
			for(int j=0;j<8;j++)
				Board[i][j]=-1;
	}
	for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
		{
			if(Board[i][j]==-1)continue;
			glLoadName(Board[i][j]);
			if(Board[i][j]<16)	drawAskari(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]<8),(choice==Board[i][j]));
			else if(Board[i][j]<20)	drawFeel(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]<18),(choice==Board[i][j]));
			else if(Board[i][j]<24)	drawHosan(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]<22),(choice==Board[i][j]));
			else if(Board[i][j]<28)	drawTabya(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]<26),(choice==Board[i][j]));
			else if(Board[i][j]<30)	drawMalek(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]==28),(choice==Board[i][j]));
			else if(Board[i][j]<32)	drawWazeer(0.25*j+XX,YY,0.25*i+ZZ,(Board[i][j]==30),(choice==Board[i][j]));
		}
}
bool Debug=false,freee=false;

void flipBoard(int Board[][8])
{
	for(int i=0;i<4;i++)
	for(int j=0;j<8;j++)
	{
		int swap=Board[7-i][7-j];
		Board[7-i][7-j]=Board[i][j];
		charsPosition[swap]=8*i+j;
		Board[i][j]=swap;
		charsPosition[Board[7-i][7-j]]=8*(7-i)+7-j;
	}
}

bool fullDebugMode(char* message)
{
	if(strcmp(message,"c3cdebug")==0)
		Debug=!Debug;
	else if(strcmp(message,"c3cfreee")==0)
		freee=!freee;
	else if(strcmp(message,"c3cflipp")==0)
	{
		flipped=!flipped;
		return true;
	}
	return false;
}

int Max(int num1,int num2)
{
	return num1>num2?num1:num2;
}
int Min(int num1,int num2)
{
	return num1<num2?num1:num2;
}
void swap(int *num1,int *num2)
{
	int temp=*num1;
	*num1=*num2;
	*num2=temp;
}

bool turn=true;

bool kingProtected(int from, int to)
{
	if(Debug)return true;
	int ifrom=from/8,jfrom=from-8*ifrom;
	int ito=to/8,jto=to-8*ito;
	int simulateBoard[8][8],simcharsPosition[32];
	bool white=true;
	for(int i=0;i<8;i++)
	for(int j=0;j<8;j++)
		simulateBoard[i][j]=Board[i][j];
	for(int i=0;i<32;i++)
		simcharsPosition[i]=charsPosition[i];
	int test=simulateBoard[ito][jto];
	simulateBoard[ito][jto]=simulateBoard[ifrom][jfrom];
	if(test!=-1) simcharsPosition[test]=-1;
	simcharsPosition[simulateBoard[ifrom][jfrom]]=ito*8+jto;
	simulateBoard[ifrom][jfrom]=-1;
	if(!whiteChar(simulateBoard[ito][jto])) white=false;


	//convert constants from blackPieces to whitePieces, c0 for Askari, c1 and c2 for types that have 1 piece or 2 pieces, respectively
	int c0=(white?0:-8),c1=(white?0:-1),c2=(white?0:-2);

	int Askari0=blackAskari0+c0,Askari7=blackAskari7+c0;
	int Wazeer=blackWazeer+c1,Malek=blackMalek+c1;
	int Feel0=blackFeel0+c2,Feel1=blackFeel1+c2,Hosan0=blackHosan0+c2,Hosan1=blackHosan1+c2,Tabya0=blackTabya0+c2,Tabya1=blackTabya1+c2;

	int Mali=simcharsPosition[white?whiteMalek:blackMalek]/8,Malj=simcharsPosition[white?whiteMalek:blackMalek]-Mali*8;

	//check for hosan in (-2,-1) (-1,-2) (+1,-2) (-2,+1) (+2,+1) (+1,+2) (-1,+2) (+2,-1)
	for(int i=0,inci=-2,incj=-1;i<8;i++)
	{
		int tempi=Mali+inci,tempj=Malj+incj;
		if(tempi<8 && tempj<8 && tempi+tempj==abs(tempi)+abs(tempj) && simulateBoard[tempi][tempj]==Hosan0 || simulateBoard[tempi][tempj]==Hosan1)
			return false; //king not protected
		if(i%2==0)swap(&inci,&incj);
		else inci=-inci;
	}

	{
		int otherMali=simcharsPosition[Malek]/8,otherMalj=simcharsPosition[Malek]-otherMali*8;
		if(abs(otherMali-Mali)<=1 && abs(otherMalj-Malj)<=1) return false;//king not protected
	}

	int begin=(white?1:6);
	if(flipped) begin=(white?6:1);
	//check for any part around the malek (+1,+1) (-1,-1) (+1,-1) (-1,+1) that can have a askari
	if((begin==6 && Mali+1<8) || (begin==1 && Mali-1>-1))
	{
		if(Malj+1<8)
		{
			test=simulateBoard[Mali+(begin==6?1:-1)][Malj+1];
			if(test<=Askari7 && test>=Askari0) return false;//king not protected
		}
		if(Malj-1>-1)
		{
			test=simulateBoard[Mali+(begin==6?1:-1)][Malj-1];
			if(test<=Askari7 && test>=Askari0) return false;//king not protected
		}
	}


	//check diagonal for any wazeer or feel
	//a tabya needs to be in places (+i,+0) (+0,+i) (-i,+0) (+0,-i) or wazeer
	//a feel needs to be in places (+i,+i) (-i,-i) (+i,-i) (-i,+i) or wazeer
	bool testing[]={true,true,true,true,true,true,true,true};
	for(int i=1;i<8;i++)
	{
		if(testing[0] && Mali+i<8)
		{
			test=simulateBoard[Mali+i][Malj];
			if(test==Wazeer || test==Tabya0 || test==Tabya1) return false;//king not protected
			else if(test!=-1) testing[0]=false;
		}
		if(testing[1] && Malj+i<8)
		{
			test=simulateBoard[Mali][Malj+i];
			if(test==Wazeer || test==Tabya0 || test==Tabya1) return false;//king not protected
			else if(test!=-1) testing[1]=false;
		}
		if(testing[2] && Mali-i>-1)
		{
			test=simulateBoard[Mali-i][Malj];
			if(test==Wazeer || test==Tabya0 || test==Tabya1) return false;//king not protected
			else if(test!=-1) testing[2]=false;
		}
		if(testing[3] && Malj-i>-1)
		{
			test=simulateBoard[Mali][Malj-i];
			if(test==Wazeer || test==Tabya0 || test==Tabya1) return false;//king not protected
			else if(test!=-1) testing[3]=false;
		}
		if(testing[4] && Mali+i<8 && Malj+i<8)
		{
			test=simulateBoard[Mali+i][Malj+i];
			if(test==Wazeer || test==Feel0 || test==Feel1) return false;//king not protected
			else if(test!=-1) testing[4]=false;
		}
		if(testing[5] && Mali+i<8 && Malj-i>-1)
		{
			test=simulateBoard[Mali+i][Malj-i];
			if(test==Wazeer || test==Feel0 || test==Feel1) return false;//king not protected
			else if(test!=-1) testing[5]=false;
		}
		if(testing[6] && Mali-i>-1 && Malj-i>-1)
		{
			test=simulateBoard[Mali-i][Malj-i];
			if(test==Wazeer || test==Feel0 || test==Feel1) return false;//king not protected
			else if(test!=-1) testing[6]=false;
		}
		if(testing[7] && Mali-i>-1 && Malj+i<8)
		{
			test=simulateBoard[Mali-i][Malj+i];
			if(test==Wazeer || test==Feel0 || test==Feel1) return false;//king not protected
			else if(test!=-1) testing[7]=false;
		}
	}

	return true;
}

//place infront of it is empty
bool inFront(int i1, int j1, int i2, int j2)
{
	if(j1==j2)
	{
		for(int i=Min(i1,i2)+1;i<Max(i1,i2);i++)
		{cout<<"j1:"<<j1<<"|imin:"<<Min(i1,i2)<<"|imax:"<<Max(i1,i2)<<"|i:"<<i<<"|Board:"<<Board[i][j1]<<endl;
			if(Board[i][j1]!=-1) return true;}
	}
	else
	{
		if(i1==i2)
		{
			for(int j=Min(j1,j2)+1;j<Max(j1,j2);j++)
			{cout<<"i1:"<<i1<<"|jmin:"<<Min(j1,j2)<<"|jmax:"<<Max(j1,j2)<<"j:"<<j<<"|Board:"<<Board[i1][j]<<endl;
				if(Board[i1][j]!=-1) return true;}
		}
		else if(abs(i1-i2)==abs(j1-j2))
		{
			int inc=(i1<i2)?j1:j2;
			inc=(inc==Min(j1,j2))?1:-1;
			for(int i=Min(i1,i2)+1,j=((i1<i2)?j1:j2)+inc;i<Max(i1,i2);i++)
			{
				if(Board[i][j]!=-1) return true;
				j+=inc;
			}
		}
	}
	return false;
}

bool movePiece(int pieceName, int from, int to)
{

	int ifrom=from/8,jfrom=from-8*ifrom;
	int ito=to/8,jto=to-8*ito;
	if(pieceName<=blackAskari7)
	{
		int begin=(turn?6:1);
		if(flipped) begin=(turn?1:6);
		//if it's the side's turn
		if(!(pieceName<=whiteAskari7 ^ turn))
		{
			bool onlyOneMoveForward=(begin==1)?(ito-ifrom==1):(ito-ifrom==-1);
			bool twoMovesForward=(begin==1)?(ito-ifrom==2):(ito-ifrom==-2);
			if(
				(
					(// and the move is allowed and nothing exists infront of it and when moved the king is still protected, allow moving
						jto==jfrom
						&& (
							onlyOneMoveForward || (ifrom==begin && twoMovesForward)
						)
						&& !inFront(ifrom, jfrom, ito+((begin==1)?1:-1), jto)
					)
					|| (
						Board[ito][jto]!=-1
						&& (whiteChar(Board[ito][jto])^turn)
						&& onlyOneMoveForward
						&& abs(jto-jfrom)==1
					)
				)// and when moved the king is still protected, allow moving
				&& kingProtected(from,to)
			)
			return true;
		}
		else
			return false;
	}
	else
	{
		if(pieceName>=whiteHosan0 && pieceName<=blackHosan1)
		{
			//if it's the side's turn
			if(!(pieceName<=whiteHosan1 ^ turn)
				&& (// and the move is allowed
					(abs(ito-ifrom)==2 && abs(jto-jfrom)==1)
					|| (abs(ito-ifrom)==1 && abs(jto-jfrom)==2)
				)// and when moved the king is still protected, allow moving
			 	&& kingProtected(from,to)
			)
				return true;
			else
				return false;
		}
		else
		{
			if(pieceName==whiteWazeer || pieceName==blackWazeer)
			{
				//if it's the side's turn
				if(!(pieceName==whiteWazeer ^ turn)
					// and the move is allowed
					&& (ito==ifrom || jto==jfrom || abs(ito-ifrom)==abs(jto-jfrom))
					// and nothing exists infront of it
					&& !inFront(ifrom, jfrom, ito, jto)
					// and when moved the king is still protected, allow moving
					&& kingProtected(from,to)
				)
					return true;
				else
					return false;
			}
			else
			{
				if(pieceName>=whiteFeel0 && pieceName<=blackFeel1)
				{
					//if it's the side's turn
					if(!(pieceName<=whiteFeel1 ^ turn)
						// and the move is allowed
						&& (abs(ito-ifrom)==abs(jto-jfrom))
						// and nothing exists infront of it
						&& !inFront(ifrom, jfrom, ito, jto)
						// and when moved the king is still protected, allow moving
						&& kingProtected(from,to)
					)
						return true;
					else
						return false;
				}
				else
				{
					if(pieceName>=whiteTabya0 && pieceName<=blackTabya1)
					{
						//if it's the side's turn
						if(!(pieceName<=whiteTabya1 ^ turn)
							// and the move is allowed
							&& (ito==ifrom || jto==jfrom)
							// and nothing exists infront of it
							&& !inFront(ifrom, jfrom, ito, jto)
							// and when moved the king is still protected, allow moving
							&& kingProtected(from,to)
						)
							return true;
						else
							return false;
					}
					else if(pieceName==whiteMalek || pieceName==blackMalek)
					{
						//if it's the side's turn
						if(!(pieceName==whiteMalek ^ turn)
							// and the move is allowed
							&& (abs(ito-ifrom)<=1 && abs(jto-jfrom)<=1)
							// and when moved the king is still protected, allow moving
							&& kingProtected(from,to)
						)
							return true;
						else
							return false;
					}
				}
			}
		}
	}
}

bool firstTime=true;
int chosen=-1;
float Xmouse=0, Ymouse=0;
char message[]={'\0','\0','\0','\0','\0','\0','\0','\0'};

void draw()
{
  	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float width=glutGet(GLUT_WINDOW_WIDTH),height=glutGet(GLUT_WINDOW_HEIGHT);

	if(RenderMode==GL_SELECT)
	{
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT,viewport);
		gluPickMatrix(Xmouse,(height-Ymouse),PICK_TOL,PICK_TOL,viewport);
	}

	gluPerspective(12.0f,width/height,0.1f,100.0f);		// Calculate The Aspect Ratio Of The Window
  	glMatrixMode(GL_MODELVIEW);

	if(RenderMode==GL_SELECT)
	{
		glInitNames();
		glPushName(0xffffffff);
	}

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3f(1,1,1);
	if(firstTime){glTranslatef(0.0f,0.0f,-10);glRotatef(15,1,0,0);}

	drawBoard(chosen);
	drawChars(firstTime,chosen);

	firstTime=false;

	if(RenderMode==GL_RENDER)
	{
		glutSwapBuffers();
		glFlush();
	}
}

void mouse(int button, int state, int x, int y)
{
	//scroll down=button3, scroll up=button4
	if(state==GLUT_UP && (button==3 || button==4))
	{
		int angle;
		if(button==3) angle=5;
		else angle=-5;
		glRotatef(angle,1,0,0);
	}
	//right click=button2
	if(state==GLUT_UP && button==2)
	{
		glRotatef(-5,0,1,0);
	}
	//left click=button0
	if(state==GLUT_UP && button==0)
	{
		int prevChosen=chosen;
		Xmouse=x;Ymouse=y;
		RenderMode = GL_SELECT;
		glRenderMode(GL_SELECT);
		draw();
		RenderMode=GL_RENDER;
		int Nhits=glRenderMode(GL_RENDER);
		for(int i=0,index=0;i<Nhits;i++)
		{
			int nitems=PickBuffer[index++];
			index+=2;//skip zmin and zmax
			for(int j=0;j<nitems;j++)
				chosen=PickBuffer[index++];
		}
		if(chosen==prevChosen || chosen==-1)
		{
			if(chosen==-1)glRotatef(5,0,1,0);
			chosen=-1;
		}
		else
		{
			int newPosition=charsPosition[chosen],i=newPosition/8,ip=charsPosition[prevChosen]/8;
			if(chosen>31)
			{
				newPosition=chosen-32;
				i=newPosition/8;		//chosen=8*i+j+32 in function drawBoard in Chars.cpp, by this equation, j is removed
				//if the tiles were chosen, choose the object in that place
				chosen=Board[i][newPosition-i*8];
			}
			//if the place chosen was empty and an object was chosen previously and it's allowed to move in its place, move it and change the turn
			//if the place chosen wasn't empty, if the object chosen was chosen when it wasn't its turn and the previous object is allowed to move in its place
			if((chosen==-1 || (chosen!=-1 && whiteChar(chosen)!=turn)) && (prevChosen<=31 && prevChosen!=-1) && (freee || movePiece(prevChosen,charsPosition[prevChosen],newPosition)))
			{	//charsPosition[label]=i*8+j dividing by 8 and receiving in integer, i remains, then removing i*8=j
				Board[ip][charsPosition[prevChosen]-ip*8]=-1;
				Board[i][newPosition-i*8]=prevChosen;
				charsPosition[prevChosen]=newPosition;
				turn=!turn;
			}
		}
	}
	glutPostRedisplay();
}

void skb(int button, int x, int y)
{
	if(button==GLUT_KEY_UP) glRotatef(-5,1,0,0);
	else if(button==GLUT_KEY_DOWN) glRotatef(5,1,0,0);
	else if(button==GLUT_KEY_LEFT) glRotatef(-5,0,1,0);
	else if(button==GLUT_KEY_RIGHT) glRotatef(5,0,1,0);
	glutPostRedisplay();
}

void kb(unsigned char button, int x, int y)
{
	if((button>='a' && button<='z') || (button>='0' && button<='9'))
	{
		for(int i=1;i<8;i++)
			message[i-1]=message[i];
		message[7]=button;
		if(Debug)cout<<message<<endl;
		if(fullDebugMode(message))
		{
			flipBoard(Board);
			glutPostRedisplay();
		}
	}
}

void initialize()
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("3D Chess");

	glSelectBuffer(PICK_BUFFER_SIZE, PickBuffer);

	LoadGLTextures();							// Load the textures
	glEnable(GL_TEXTURE_2D);						// Enable texture mapping
	RenderMode = GL_RENDER;
	glClearColor(0.91f, 0.8f, 0.4f, 0.0f);					// This Will Clear The Background Color To Black
	glDepthFunc(GL_LESS);							// The Type Of Depth Test To Do
 	glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
  	glShadeModel(GL_SMOOTH);						// Enables Smooth Color Shading
}

int main(int argc,char** argv)
{
	glutInit(&argc, argv);
	initialize();
	glutDisplayFunc(draw);
	glutSpecialFunc(skb);
	glutMouseFunc(mouse);
	glutKeyboardFunc(kb);
	glutMainLoop();
	return 1;
}
