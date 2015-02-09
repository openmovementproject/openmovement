// IMU Demo
// Dan Jackson, 2011-13

#define REPOSITION_ORIGIN
//#define DRAW_AXES

#define YAW_OFFSET 90.0f

//#define THREADED    // Background update thread

#define BETA -1.0f

#define INTEGRATE

#ifdef INTEGRATE
#include "azimuth/integrator.h"
integrator_t velocityIntegrator[3] = {0};
integrator_t positionIntegrator[3] = {0};
float velocity[3] = {0};
float position[3] = {0};
#endif


// Includes
#ifdef _WIN32
//#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable: 4996)  // Disable warning C4996 - deprecated functions
#include <windows.h>
#define _USE_MATH_DEFINES       // For M_PI
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define usleep(_t) Sleep((_t)/1000)
#endif

#define GLUT_NO_WARNING_DISABLE
#ifdef _WIN32
  #include <GL/glut.h>
  #include <GL/glu.h>
  #include "glext.h"
  //#include "resource.h"
  #define uglGetProcAddress(x) wglGetProcAddress(x)
#elif defined(__X11__)
  #include <GL/glut.h>
  #include <GL/glu.h>
  #include <GL/glx.h>
  #include <GL/glxext.h>
  #define uglGetProcAddress(x) (*glXGetProcAddressARB)((const GLubyte*)(x))
#elif defined(__APPLE__)
  #include <OpenGL/glut.h>
  #include <OpenGL/glu.h>
  #include <OpenGL/glext.h>
  //#define uglGetProcAddress(x)
#endif



/* Cross-platform alternatives */
#ifdef _WIN32

    /* Mutex */
	#define mutex_t HANDLE
    #define mutex_init(mutex, attr_ignored) ((*(mutex) = CreateMutex(attr_ignored, FALSE, NULL)) == NULL)
    #define mutex_lock(mutex) (WaitForSingleObject(*(mutex), INFINITE) != WAIT_OBJECT_0)
    #define mutex_unlock(mutex) (ReleaseMutex(*(mutex)) == 0)
    #define mutex_destroy(mutex) (CloseHandle(*(mutex)) == 0)
    
#else

    /* Mutex */
    #include <pthread.h>
	#define mutex_t       pthread_mutex_t
    #define mutex_init    pthread_mutex_init
    #define mutex_lock    pthread_mutex_lock
    #define mutex_unlock  pthread_mutex_unlock
    #define mutex_destroy pthread_mutex_destroy

#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "azimuth/azimuth.h"
#include "resource.h"

#include "azimuth/display.h"
image_t image = {0};
display_t imageDisplay = {0};

#ifndef __cplusplus
#define bool char
#define true 1
#define false 0
//#define const
#endif

#define DEG2RAD ((float)M_PI / 180.0f)
#define RAD2DEG (180.0f / (float)M_PI)
static const char *windowTitle = "Inertial Measurement Unit Demo";
static char initialCommand[128] = {0};
#define DEFAULT_COMMAND "\r\nRATE M 1 80\r\nRATE X 1 100\r\nMODE 2\r\nDATAMODE 1\r\nSTREAM \\#\r\n"
static bool isFullScreen = false, isCursor = true;
static int restoreWindow[4] = { 0, 0, 1024, 768 };
static int lastModifiers = 0;
static int lastButtons = 0, lastCursorX = 0, lastCursorY = 0;
static int hitTestX = 0, hitTestY = 0;
static bool renderToDevice = false;
static int ditherIndex = 4;
static bool edgeEnhance = true;
static bool showGraph = false;
#ifdef INTEGRATE
static bool showOffset = false;
#endif

//#define TIMER_RATE (1000/30)
static float viewportSize = 0.98f;
#define NAME_NONE -1


static char *inputFile = "!";

static unsigned int floorTexture = 0;
static unsigned int subHull = 0, subFront = 0, subBack = 0, subFins = 0;
static unsigned int sphereMapTexture = 0;

static azimuth_t azimuth = {0};


// Multitexturing
static PFNGLMULTITEXCOORD1FARBPROC      glMultiTexCoord1fARB     = NULL;
static PFNGLMULTITEXCOORD2FARBPROC      glMultiTexCoord2fARB     = NULL;
static PFNGLMULTITEXCOORD3FARBPROC      glMultiTexCoord3fARB     = NULL;
static PFNGLMULTITEXCOORD4FARBPROC      glMultiTexCoord4fARB     = NULL;
static PFNGLACTIVETEXTUREARBPROC        glActiveTextureARB       = NULL;
static PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB = NULL;


typedef struct
{
    // private
    int maxSize;
    int numSeries;
    int size;
    int offset;
    float scale;
    float *values;
} graph_t;

#ifdef THREADED
mutex_t mutex;
#endif
char displayChanged = 0;    // device display updated
char changed = 0;       // need to re-render

#define GRAPH_COUNT 4
graph_t graphs[GRAPH_COUNT];

void GraphCreate(graph_t *graph, int maxSize, int numSeries, float scale)
{
    graph->maxSize = maxSize;
    graph->numSeries = numSeries;
    graph->size = 0;
    graph->offset = 0;
    graph->values = (float *)malloc(sizeof(float) * numSeries * maxSize);
    graph->scale = scale;
}

void GraphAddPoint(graph_t *graph, float *values)
{
    float *p = graph->values + (graph->offset * graph->numSeries);
    memcpy(p, values, sizeof(float) * graph->numSeries);
    graph->offset = (graph->offset + 1) % graph->maxSize;
    if (graph->size < graph->maxSize) { graph->size++; }
}



/* Loads a bottom-up 24-bit BGR888 bitmap as top-down RGBX8888 in memory */
void *loadImage(const char *filename, int *outWidth, int *outHeight)
{
    unsigned char bmpHeader[54];          /* BMP_HEADERSIZE = 54 */
    unsigned long bfOffBits, biWidth, biHeight, biCompression;
    unsigned short biPlanes, biBitCount;
    unsigned int sourceWidth;
    unsigned char *buffer;
    unsigned char *line;
    unsigned char *src, *dest;
    int x, y;
    FILE *fp;

    /* Open file, read and check header */
    if (filename == NULL || filename[0] == '\0')    { printf("ERROR: loadImage() - file not specified\n"); return NULL; }
    if ((fp = fopen(filename, "rb")) == NULL)       { printf("ERROR: loadImage() - file couldn't be opened (file not found?): %s\n", filename); return NULL; }
    if (fread(bmpHeader, 1, sizeof(bmpHeader), fp) != sizeof(bmpHeader)) { printf("ERROR: loadImage() - problem reading header from file: %s\n", filename); fclose(fp); return NULL; }
    if (bmpHeader[0] != 'B' || bmpHeader[1] != 'M') { printf("ERROR: loadImage() - invalid header in file: %s\n", filename); fclose(fp); return NULL; }

    /* Extract important information from header (long winded endian-neutral) */
    bfOffBits     = ((unsigned long)bmpHeader[10] | (((unsigned long)bmpHeader[11]) << 8) | (((unsigned long)bmpHeader[12]) << 16) | (((unsigned long)bmpHeader[13]) << 24));
    biWidth       = ((unsigned long)bmpHeader[18] | (((unsigned long)bmpHeader[19]) << 8) | (((unsigned long)bmpHeader[20]) << 16) | (((unsigned long)bmpHeader[21]) << 24));
    biHeight      = ((unsigned long)bmpHeader[22] | (((unsigned long)bmpHeader[23]) << 8) | (((unsigned long)bmpHeader[24]) << 16) | (((unsigned long)bmpHeader[25]) << 24));
    biPlanes      = ((unsigned short)bmpHeader[26] | (((unsigned short)bmpHeader[27]) << 8));
    biBitCount    = ((unsigned short)bmpHeader[28] | (((unsigned short)bmpHeader[29]) << 8));
    biCompression = ((unsigned short)bmpHeader[30] | (((unsigned short)bmpHeader[31]) << 8));
    if (biPlanes != 1 || biCompression != 0 || biBitCount != 24 || (int)biHeight <= 0 || biWidth == 0) { printf("ERROR: loadImage() - unsupported format, (only support bottom-up 24-bit uncompressed), in file: %s\n", filename); fclose(fp); return NULL; }

    /* Create buffer for RGBX data */
    if ((buffer = (unsigned char *)malloc(4 * biWidth * biHeight)) == NULL) { printf("ERROR: loadImage() - memory allocation failed: %s\n", filename); fclose(fp); return NULL; }

    /* Read file, turn bottom-up BGR888 to top-down RGBX8888, go bottom up to not overwrite source data in same buffer */
    fseek(fp, bfOffBits, SEEK_SET);
    sourceWidth = ((((biWidth * biBitCount) + 31) & ~31) >> 3);  /* Lines stored to word-aligned boundaries */
    line = (unsigned char *)malloc(sourceWidth);
    for (y = (int)biHeight - 1; y >= 0; y--)
    {
        fread(line, 1, sourceWidth, fp);
        src = line;
        dest = buffer + biWidth * y * 4;
dest = buffer + biWidth * ((int)biHeight - 1 - y) * 4;    // Invert
        for (x = 0; x < (int)biWidth; x++)
        {
            dest[0] = src[2]; dest[1] = src[1]; dest[2] = src[0]; dest[3] = 0xff;
            dest += 4; 
            src += 3;
        }
    }

    /* Clean-up and return */
    free(line);
    fclose(fp);
    if (outWidth) { *outWidth = biWidth; }
    if (outHeight) { *outHeight = biHeight; }
    return buffer;
}


unsigned int LoadTexture(const char* filename)
{
	int width, height;
	void *buffer;
	unsigned int textureId = 0;
	buffer = loadImage(filename, &width, &height);
	if (buffer == NULL) { return 0; }
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	free(buffer);
	return textureId;
}


// Colors
static unsigned char backgroundColor[]        = {0,0,0,0}; //{ 0x1f, 0x1f, 0x1f, 0x00 };
static unsigned char boardDarkColor[]         = { 0x4f, 0x4f, 0x4f, 0xff };
static unsigned char boardLightColor[]        = { 0x9f, 0x9f, 0x9f, 0xff };
static unsigned char borderColor[]            = { 0xff, 0xff, 0xff, 0xff };

static float borderSize = 0.0f;
static bool drawHitTest = false;

#define DEFAULT_BOARD_ANGLE 20.4f
#define DEFAULT_BOARD_DISTANCE 0.95f //1.33f
#define DEFAULT_EYE_SEP 0.03f
static float panX = 0.0f, panY = 0.0f, panZ = DEFAULT_BOARD_DISTANCE;
static float camLong = 0.0f, camLat = DEFAULT_BOARD_ANGLE, camTwist = 0.0f;
static float camFov = 65.0f;

static const int CHECKERBOARD = 10;



/*
// Old yaw, pitch, roll code

void TranslateX(t_position *p, float v)
{
	p->x += v * sinf(DEG2RAD * p->yaw - (float)M_PI/2)*-cosf(DEG2RAD * p->roll);
	p->z += v * cosf(DEG2RAD * p->yaw - (float)M_PI/2)*-cosf(DEG2RAD * p->roll);
	p->y += v * sinf(DEG2RAD * p->roll);
}

void TranslateY(t_position *p, float v)
{
	p->x += v * cosf(DEG2RAD * p->yaw - (float)M_PI/2) * -sinf(DEG2RAD * p->pitch);
	p->z += -v * sinf(DEG2RAD * p->yaw - (float)M_PI/2) * -sinf(DEG2RAD * p->pitch);
	p->y -= v * cosf(DEG2RAD * p->roll) * cosf(DEG2RAD * p->pitch);
}

void TranslateZ(t_position *p, float v)
{
	p->x += v * sinf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch);
	p->z += v * cosf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch);
	p->y -= v * sinf(DEG2RAD * p->pitch);
}

// Get 'nose' of object
// float v = 1.0f;
// float x = p->x + v * (sinf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch));
// float y = p->y + v * -sinf(DEG2RAD * p->pitch);
// float z = p->z + v * (cosf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch));

*/


typedef struct
{
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emission[4];
	float shininess[1];
} Material;


// Submarine Material
Material matSub = {
	{ 0.1f, 0.1f, 0.05f, 1.0f },		// Ambient
	{ 0.2f, 0.2f, 0.1f, 1.0f },			// Diffuse
	{ 0.2f,  0.2f,  0.05f, 1.0f },		// Specular
	{ 0.0f, 0.0f, 0.0f, 1.0f },			// Emission
	{ 0.1f }							// Shininess
};

// Submarine Propeller Material
Material matSubProp = {
	{ 0.1f, 0.1f, 0.1f, 1.0f },			// Ambient
	{ 0.3f, 0.3f, 0.3f, 1.0f },			// Diffuse
	{ 0.2f, 0.2f, 0.2f, 1.0f },			// Specular
	{ 0.0f, 0.0f, 0.0f, 1.0f },			// Emission
	{ 0.2f }							// Shininess
};

// Submarine Periscope Glass Material
Material matSubGlass = {
	{ 0.05f, 0.05f, 0.1f, 1.0f },			// Ambient
	{ 0.2f, 0.2f, 0.6f, 1.0f },			// Diffuse
	{ 0.2f, 0.2f, 0.7f, 1.0f },			// Specular
	{ 0.0f, 0.0f, 0.0f, 1.0f },			// Emission
	{ 0.3f }							// Shininess
};


// Start-up options
static bool showHelp = false;
static float eyeSep = DEFAULT_EYE_SEP;
static bool startFullscreen = false, startCursor = true, startSound = true, start = false, startStereo = false;

#define HEX_VALUE(_c) (((_c) >= '0' && (_c) <= '9') ? ((_c) - '0') : (((_c) >= 'a' && (_c) <= 'f') ? ((_c) + 10 - 'a') : (((_c) >= 'A' && (_c) <= 'F') ? ((_c) + 10 - 'A') : 0)))

bool parseColor(const char *colorString, unsigned char *color)
{
	if (color != NULL && colorString != NULL && colorString[0] != '\0')
	{
		const char *p = colorString;
		char bits = -1;                                                                     // IBGR
		int l;

		if (*p == '#') p++;

		if (strcasecmp(p, "black")   == 0)                                  { bits = 0x0; } // 0000
		if (strcasecmp(p, "maroon")  == 0)                                  { bits = 0x1; } // 0001
		if (strcasecmp(p, "green")   == 0)                                  { bits = 0x2; } // 0010
		if (strcasecmp(p, "olive")   == 0)                                  { bits = 0x3; } // 0011
		if (strcasecmp(p, "navy")    == 0)                                  { bits = 0x4; } // 0100
		if (strcasecmp(p, "purple")  == 0)                                  { bits = 0x5; } // 0101
		if (strcasecmp(p, "teal")    == 0)                                  { bits = 0x6; } // 0110
		if (strcasecmp(p, "gray")    == 0 || strcasecmp(p, "grey")    == 0) { bits = 0x7; } // 0111
		if (strcasecmp(p, "silver")  == 0)                                  { bits = 0x8; } // 1000
		if (strcasecmp(p, "red")     == 0)                                  { bits = 0x9; } // 1001
		if (strcasecmp(p, "lime")    == 0)                                  { bits = 0xa; } // 1010
		if (strcasecmp(p, "yellow")  == 0)                                  { bits = 0xb; } // 1011
		if (strcasecmp(p, "blue")    == 0)                                  { bits = 0xc; } // 1100
		if (strcasecmp(p, "magenta") == 0 || strcasecmp(p, "fuchsia") == 0) { bits = 0xd; } // 1101
		if (strcasecmp(p, "cyan")    == 0 || strcasecmp(p, "aqua")    == 0) { bits = 0xe; } // 1110
		if (strcasecmp(p, "white")   == 0)                                  { bits = 0xf; } // 1111
		if (bits != -1)
		{
			char val0 = ((bits & 0xf) == 0x7) ? 0xc0 : 0x00;
			char val1 = ((bits & 0x8) == 0x8) ? 0xff : 0x80;
			color[0] = ((bits >> 0) & 1) ? val1 : val0;
			color[1] = ((bits >> 1) & 1) ? val1 : val0;
			color[2] = ((bits >> 2) & 1) ? val1 : val0;
			color[3] = 0xff;
			return true;
		}

		l = strlen(p);
		if (l == 3 || l == 4 || l == 6 || l == 8)
		{
			color[0] = HEX_VALUE(*p); p++; if (l > 4) { color[0] = (color[0] << 4) | HEX_VALUE(*p); p++; } else { color[0] |= color[0] << 4; } 
			color[1] = HEX_VALUE(*p); p++; if (l > 4) { color[1] = (color[1] << 4) | HEX_VALUE(*p); p++; } else { color[1] |= color[1] << 4; }
			color[2] = HEX_VALUE(*p); p++; if (l > 4) { color[2] = (color[2] << 4) | HEX_VALUE(*p); p++; } else { color[2] |= color[2] << 4; }
			if (l == 4 || l == 8)
			{
				color[3] = HEX_VALUE(*p); p++; if (l > 4) { color[3] = (color[3] << 4) | HEX_VALUE(*p); p++; } else { color[3] |= color[3] << 4; }
			}
			else
			{
				color[3] = 0xff;
			}
			return true;
		}
	}
	return false;
}

// Set material properties
void setMaterial(int gl_face, Material *mat)
{
	glMaterialfv(gl_face, GL_AMBIENT,   mat->ambient);
	glMaterialfv(gl_face, GL_DIFFUSE,   mat->diffuse);
	glMaterialfv(gl_face, GL_SPECULAR,  mat->specular);
	glMaterialfv(gl_face, GL_EMISSION,  mat->emission);
	glMaterialfv(gl_face, GL_SHININESS, mat->shininess);
	return;
}

void drawSphere2(float *position, float size)
{
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GL_TRUE); 
	//gluQuadricTexture(qobj, GL_TRUE); 
	//gluQuadricDrawStyle(qobj, GLU_SILHOUETTE);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
        glEnable(GL_COLOR_MATERIAL);
		glTranslatef(position[0], position[1], position[2]);
		gluSphere(qobj, size, 12, 6);
        glDisable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
	glPopAttrib();

	gluDeleteQuadric(qobj);
}


void RenderObject(azimuth_t *azimuth)
{
    int pass;

    for (pass = 0; pass <= 2; pass++)   // 0=shadow, 1=base, 2=reflection
    {
	    GLUquadricObj* qobj;
	    float sz = 0.2f;
	    float ang = 0.0f, rad = 0.5f, pang = 30.0f;
	    float peri_len = 1.3f, peri_ang = 0.0f;

        if (pass == 0)
        {
	        glPushAttrib(GL_ALL_ATTRIB_BITS);
	        glPushMatrix();
	        glDisable(GL_DEPTH_TEST);
	        glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            glColor3f(0.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.5f, 0.0f);
            glScalef(1.0f, 0.0f, 1.0f);
        }

	    ang = glutGet(GLUT_ELAPSED_TIME) / 1000.0f * 360.0f * 1;		// 1 Hz rotation

	    qobj = gluNewQuadric();
	    gluQuadricNormals(qobj, GL_TRUE); 

    #if 0
	    glPushAttrib(GL_ALL_ATTRIB_BITS);
	    glPushMatrix();

		    if (pass == 1)
		    {
			    float v = 0.25f * 4;

			    //float x = p->x + v * -(sinf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch));
			    //float y = p->y + v * sinf(DEG2RAD * p->pitch);
			    //float z = p->z + v * (cosf(DEG2RAD * p->yaw) * cosf(DEG2RAD * p->pitch));

			    //float x = p->x + v * p->gx;
			    //float y = p->y + v * p->gy;
			    //float z = p->z + v * p->gz;

			    // phi (roll), theta (pitch), psi (yaw)

			    glDisable(GL_LIGHTING);
			    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			    glLineWidth(2.0f);
			    glColor3f(1.0f, 1.0f, 1.0f);
			    glBegin(GL_LINES);
				    glVertex3f(0.0f, 0.0f, 0.0f);
				    glVertex3f(x, y, z);
			    glEnd();
		    }
		
	    glPopMatrix();
	    glPopAttrib();
    #endif

	    if (pass == 1)
	    {
		    gluQuadricTexture(qobj, GL_TRUE); 
	    }

	    glPushAttrib(GL_ALL_ATTRIB_BITS);
	    glPushMatrix();

            glEnable(GL_CULL_FACE);
            glDisable(GL_COLOR_MATERIAL);

		    if (pass == 1)
		    {
			    glEnable(GL_TEXTURE_2D);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			    glEnable(GL_TEXTURE_2D);
			    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
			    glDisable(GL_LIGHTING);
		    }
		    if (pass == 2)
		    {
			    glBindTexture(GL_TEXTURE_2D, sphereMapTexture);
			    glEnable(GL_TEXTURE_2D);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			    glEnable(GL_TEXTURE_GEN_S);
			    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			    glEnable(GL_TEXTURE_GEN_T);	
			    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

			    glEnable(GL_BLEND);
			    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			    glDisable(GL_LIGHTING);
			    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			    glEnable(GL_DEPTH_TEST);
			    glDepthMask(GL_FALSE);
			    glPolygonOffset(-0.0f, -1.0f);
			    glEnable(GL_POLYGON_OFFSET_FILL);
			    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);

			    glDisable(GL_LIGHTING);
		    }

		    if (pass != 2) { setMaterial(GL_FRONT_AND_BACK, &matSub); }

		    if (pass == 1)
		    {
			    float len = 0.30f, ofs = 0.0f;
			    glPushMatrix();
				    glPushAttrib(GL_ALL_ATTRIB_BITS);
					    glTranslatef(0.0f, -0.5f, 0.0f); 
					    //glDepthMask(GL_FALSE);
					    glDisable(GL_DEPTH_TEST);
					    glDisable(GL_TEXTURE_2D);
					    glDisable(GL_LIGHTING);
					    glLineWidth(3.0f);
					    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);

    #if 1
                        // Plot linear acceleration
                        if (pass != 0) { glColor4f(1.0f, 0.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(len * azimuth->linearAccel[0] + ofs, 0.0f, 0.0f); glEnd();
					    if (pass != 0) { glColor4f(0.0f, 1.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, len * azimuth->linearAccel[1] + ofs, 0.0f); glEnd();
					    if (pass != 0) { glColor4f(0.0f, 0.0f, 1.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, len * azimuth->linearAccel[2] + ofs); glEnd();
    #endif
    #if 0
                        // Plot screen-aligned acceleration
                        if (pass != 0) { glColor4f(1.0f, 0.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(len * azimuth->screen[0] + ofs, 0.0f, 0.0f); glEnd();
					    if (pass != 0) { glColor4f(0.0f, 1.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, len * azimuth->screen[1] + ofs, 0.0f); glEnd();
					    if (pass != 0) { glColor4f(0.0f, 0.0f, 1.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, len * azimuth->screen[2] + ofs); glEnd();
    #endif
				    glPopAttrib();
			    glPopMatrix();
		    }

//		    glTranslatef(offset[0], offset[1], offset[2]); 

		    if (pass == 1)
		    {
			    float rad = 0.02f;

			    glPushMatrix();
				    glScalef(0.30f, 0.30f, 0.30f);
                    glColor4f(1.0f, 0.0f, 0.0f, 1.0f); drawSphere2(azimuth->matrixRotation + 0, rad);  // PositionX
    				glColor4f(0.0f, 1.0f, 0.0f, 1.0f); drawSphere2(azimuth->matrixRotation + 4, rad);  // PositionY
    				glColor4f(0.0f, 0.0f, 1.0f, 1.0f); drawSphere2(azimuth->matrixRotation + 8, rad);  // PositionZ
			    glPopMatrix();

			    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); 
		    }

		    /*
		    glRotatef(-p->yaw, 0.0f, 1.0f, 0.0f); 
		    glRotatef(-p->pitch, 1.0f, 0.0f, 0.0f);      
		    glRotatef(p->roll, 0.0f, 0.0f, 1.0f); 
		    */

		    // Euler angles - order as in: http://www.varesano.net/blog/fabio/ahrs-sensor-fusion-orientation-filter-3d-graphical-rotating-cube
		    glRotatef(-azimuth->euler[2] * RAD2DEG, 0.0f, 0.0f, 1.0f);	// Z: phi (roll)
		    glRotatef(-azimuth->euler[1] * RAD2DEG, 1.0f, 0.0f, 0.0f);  // X: theta (pitch)
		    glRotatef(-azimuth->euler[0] * RAD2DEG, 0.0f, 1.0f, 0.0f);	// Y: psi (yaw)

		    //gluQuadricDrawStyle(qobj, GLU_SILHOUETTE);

		    if (pass != 2)
		    {
			    float len = 0.30f, ofs = 0.30f;
			    float gscale = 0.5f * (1.0f + azimuth->relAccel[1]);

			    // Draw acceleration relative to device
			    glPushAttrib(GL_ALL_ATTRIB_BITS);
				    glDisable(GL_TEXTURE_2D);
				    glDisable(GL_LIGHTING);
				    glLineWidth(3.0f);
				    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);

    #if 1
				    if (pass != 0) { glColor4f(1.0f, 0.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(len * azimuth->relAccel[0] + ofs, 0.0f, 0.0f); glEnd();
				    if (pass != 0) { glColor4f(0.0f, 1.0f, 0.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, len * azimuth->relAccel[1] + ofs, 0.0f); glEnd();
				    if (pass != 0) { glColor4f(0.0f, 0.0f, 1.0f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, len * azimuth->relAccel[2] + ofs); glEnd();
    #endif

				    glLineStipple(2, 0xAAAA); glEnable(GL_LINE_STIPPLE);
				    if (pass != 0) { glColor4f(0.7f, 0.7f, 0.7f, 1.0f); } glBegin(GL_LINES); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(azimuth->gravity[0] * gscale, azimuth->gravity[1] * gscale, azimuth->gravity[2] * gscale); glEnd();
			    glPopAttrib();
		    }

    // Draw gravity relative to device
    if (pass == 1)
    {
    glPushMatrix();
    glScalef(0.5f, 0.5f, 0.5f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); 
    drawSphere2(azimuth->gravity, 0.01f);
    glPopMatrix();
    }
            glScalef(0.1f, 0.1f, 0.1f);

		    glPushMatrix();
            {
#ifdef YAW_OFFSET
glRotatef(YAW_OFFSET, 0.0f, 1.0f, 0.0f);
#endif

		        glPushMatrix();

			        glTranslatef(0.0f, 0.0f, -1.0f);

			        // Body
			        if (pass == 1) { glBindTexture(GL_TEXTURE_2D, subHull); }
			        glPushMatrix();
				        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
				        gluCylinder(qobj, 1.0f, 1.0f, 2.0f, 12, 6);
			        glPopMatrix();

			        // Nose
			        if (pass == 1) { glBindTexture(GL_TEXTURE_2D, subFront); }
			        glTranslatef(0.0f, 0.0f, 2.0f);
			        glPushMatrix();
        //				glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
				        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
				        //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
				        //glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
				        gluSphere(qobj, 1.0f, 12, 6);
			        glPopMatrix();

			        // Back end bit
			        if (pass == 1) { glBindTexture(GL_TEXTURE_2D, subBack); }
			        glTranslatef(0.0f, 0.0f, -2.0f);
			        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			        gluDisk(qobj, 0.0f, 1.0f, 12, 6);

			        // Fin/Tail bit
			        if (pass == 1) { glBindTexture(GL_TEXTURE_2D, subFins); }
			        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
			        glTranslatef(0.0f, 0.0f, 1.0f);
			        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			        gluDisk(qobj, 0.0f, 1.5f, 12, 6);
			        // ... and top surface of fin/tail
			        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
			        gluDisk(qobj, 0.0f, 1.5f, 12, 6);

			        if (pass == 1) { glDisable(GL_TEXTURE_2D); }

			        if (pass != 2) { setMaterial(GL_FRONT_AND_BACK, &matSubProp); }

			        // Propeller
			        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
			        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
			        glTranslatef(0.0f, 0.0f, -1.0f);
			        glTranslatef(0.0f, 0.0f, -0.5f);

			        gluPartialDisk(qobj, 0.0f, rad, 3, 3, ang, pang);
			        gluPartialDisk(qobj, 0.0f, rad, 3, 3, ang+180, pang);
			        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
			        gluPartialDisk(qobj, 0.0f, rad, 3, 3, ang, pang);
			        gluPartialDisk(qobj, 0.0f, rad, 3, 3, ang+180, pang);

		        glPopMatrix();

		        if (pass != 2) { setMaterial(GL_FRONT_AND_BACK, &matSub); }
		        if (pass == 1) { glColor3f(1.0f, 1.0f, 0.3f); } // Sub color

		        // Rotate to vertical for periscope
		        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		        glRotatef(peri_ang, 0.0f, 0.0f, 1.0f);

		        gluCylinder(qobj, sz, sz, peri_len, 12, 6);
		        glTranslatef(0.0f, 0.0f, peri_len);
		        gluSphere(qobj, sz, 12, 6);
		        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		        gluCylinder(qobj, sz, sz, 0.4f, 12, 6);
		        glTranslatef(0.0f, 0.0f, 0.4f);

		        if (pass != 2) { setMaterial(GL_FRONT_AND_BACK, &matSubGlass); }
		        if (pass == 1) { glColor3f(0.3f, 0.3f, 1.0f); } // Glass color
		        gluDisk(qobj, 0.0f, sz, 12, 6);
            }
    	    glPopMatrix();

	    glPopMatrix();
	    glPopAttrib();

	    gluDeleteQuadric(qobj);

    glDisable(GL_POLYGON_OFFSET_FILL);


    if (pass == 0)
    {
	    glPopMatrix();
	    glPopAttrib();
    }
    }

	return;
}

// Perspective with min FOV in either X or Y
void myPerspective(GLdouble fov, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	float fw, fh;

	if (aspect >= 1.0f)
	{
		// Treat as vertical FOV
		fh = (float)(tan((fov / 2.0) * M_PI / 180.0) * zNear);
		fw = (float)(fh * aspect);
	}
	else
	{
		// Treat as horizontal FOV
		fw = (float)(tan((fov / 2.0) * M_PI / 180.0) * zNear);
		fh = (float)(fw / aspect);
	}
	glFrustum(-fw, fw, -fh, fh, zNear, zFar);
}

void drawTextCursor(void *font, float fx, float fy, int selStart, int selEnd, const char *string)
{
	int viewport[4];
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); 
	glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1], viewport[1] + viewport[3], -1.0f, 1.0f);

	{
		// Calculate width
		float x, y;
		float w = 0.0f;
		float h = 15.0f;
		float selStartX = 0.0f;
		float selEndX = 0.0f;
		int index = 0;
		const char *c;
		for (c = string; ; c++) 
		{
			float lastW = (float)glutBitmapWidth(font, *c == '\0' ? ' ' : *c);
			if ((index == selStart && selStart <= selEnd) || (index == selEnd && selStart > selEnd)) { selStartX = w; }
			if (index >= selStart && index < selEnd) { selEndX = w + lastW; }
			if (index >= selEnd && index < selStart) { selStartX = w + lastW; }
			index++;
			if (*c == '\0') { break; }
			w += lastW;
		}

		x = fx * viewport[2] - w / 2;
		y = (1.0f - fy) * viewport[3] - h / 2;

		// Draw text centered at position
		glRasterPos2f(x, y);
		for (c = string; *c != '\0'; c++) 
		{
			glutBitmapCharacter(font, *c);
		}

		if (selStart >= 0 && selEnd >= 0)
		{
			// Draw cursor/selection
			glEnable(GL_COLOR_LOGIC_OP);
			glLogicOp(GL_XOR);
			glRectf(x + selStartX - 1, y - 1, x + selEndX + 1, y + h + 1);
			glDisable(GL_COLOR_LOGIC_OP);
		}
	}

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
	return;
}

void drawText(void *font, float fx, float fy, const char *format, ...)
{
	char string[1024]; 
	va_list va;
	va_start(va, format);
	_vsnprintf(string, 2048, format, va);
	va_end(va);
	string[1023] = '\0'; 

	drawTextCursor(font, fx, fy, -1, -1, string);
}

static void drawCone(float x, float y, float z, float size)
{
	GLUquadric *qobj;
	glPushMatrix();
	glTranslatef(x, y, z);
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluCylinder(qobj, size, size / 2, 2 * size, 10, 4);
	glTranslatef(0.0f, 0.0f, 2 * size);
	gluDisk(qobj, 0.0f, size / 2, 10, 2);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}

static void drawSphere(float x, float y, float z, float size)
{
	GLUquadric *qobj;
	glPushMatrix();
	glTranslatef(x, y, z);
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluSphere(qobj, size, 8, 8);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}

static void drawCube(float x, float y, float z, float size)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	glutSolidCube(size);
	glPopMatrix();
}

static void fillRect(float x, float y, float w, float h)
{
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBegin(GL_QUADS);
	glVertex3f(x    , -0.5f, y    );
	glVertex3f(x + w, -0.5f, y    );
	glVertex3f(x + w, -0.5f, y + h);
	glVertex3f(x    , -0.5f, y + h);
	glEnd();
	glPopAttrib();
	glPopMatrix();
}

static void drawBoard()
{
	// Draw the scene
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

    glDisable(GL_LIGHTING);

	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	if (!drawHitTest)
	{
		int x, y;
		bool checkerboard = true;

		//glDepthMask(GL_FALSE);

		if (checkerboard)
		{
			for (y = 0; y < CHECKERBOARD; y++)
			{
				for (x = 0; x < CHECKERBOARD; x++)
				{
					float scale = 3.0f;
					float z = -0.5f;
					float x0 = (1.0f * (x + 0) / CHECKERBOARD) - 0.5f;
					float x1 = (1.0f * (x + 1) / CHECKERBOARD) - 0.5f;
					float y0 = (1.0f * (y + 0) / CHECKERBOARD) - 0.5f;
					float y1 = (1.0f * (y + 1) / CHECKERBOARD) - 0.5f;
					if ((x ^ y) & 1) {
						glColor3ubv(boardDarkColor);
					} else {
						glColor3ubv(boardLightColor);
					}

					glBegin(GL_QUADS);
						glTexCoord2d((x    ) * scale / CHECKERBOARD, (y    ) * scale / CHECKERBOARD); glVertex3f(x0, z, y0);
						glTexCoord2d((x + 1) * scale / CHECKERBOARD, (y    ) * scale / CHECKERBOARD); glVertex3f(x1, z, y0);
						glTexCoord2d((x    ) * scale / CHECKERBOARD, (y + 1) * scale / CHECKERBOARD); glVertex3f(x1, z, y1);
						glTexCoord2d((x + 1) * scale / CHECKERBOARD, (y + 1) * scale / CHECKERBOARD); glVertex3f(x0, z, y1);
					glEnd();

				}
			}
		}
		else
		{
			glColor3ubv(boardDarkColor);
			fillRect(0.0f, 0.0f, 1.0f, 1.0f);
		}

		//glDepthMask(GL_TRUE);
	}

	glPopMatrix();
	glPopAttrib();
}

static void drawUI(int *viewport)
{
	// Draw the scene
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

    if (showGraph)
    {
        int pass;
        for (pass = 0; pass <= 1; pass++)
        {
            int i, j;
            float graphHeight = (float)viewport[3] / GRAPH_COUNT;

            glEnable(GL_BLEND);
            if (pass == 0) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            else glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
            glLineWidth(pass == 0 ? 2.0f : 2.0f);

            for (i = 0; i < GRAPH_COUNT; i++)
            {
                // RGB
                unsigned char colors[][3] = { { 0xff, 0x00, 0x00 }, { 0x00, 0xff, 0x00 }, { 0x00, 0x00, 0xff } };
                graph_t *graph = &graphs[i];

                for (j = 0; j < graph->numSeries; j++)
                {
                    int k;
                    float ox = (float)viewport[2] - graph->size;
                    float oy = graphHeight * i + (graphHeight / 2);
                    float scale = -graph->scale;

                    if (pass == 0) { glColor4f(0.0f, 0.0f, 0.0f, 0.5f); } else { glColor3ubv(colors[j % (sizeof(colors) / sizeof(colors[0]))]); }
                    glBegin(GL_LINE_STRIP);
                        for (k = 0; k < graph->size; k++)
                        {
                            int o, ofs;
                            float x, y;

                            ofs = 0; if (graph->size >= graph->maxSize) { ofs = graph->offset; }
                            o = (ofs + k) % graph->maxSize;
                            y = graph->values[o * graph->numSeries + j];
    //y = sin((float)(graph->offset + k)/100 + (float)j/4);

                            //if (y < -graphHeight / 2) { y = -graphHeight / 2; }
                            //if (y >  graphHeight / 2) { y =  graphHeight / 2; }

                            x = ox + k;
                            y = oy + (y * scale * (graphHeight / 2));
                            glVertex2f(x, y);
                        }
                    glEnd();
                }
            }
        }
    }

	glPopMatrix();
	glPopAttrib();
}

static void displayWorld(int eye)
{
	int viewport[4];
	float w, h, aspect;

	// Save state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	// Get the viewport size
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	w = (float)viewport[2];
	h = (float)viewport[3];
	aspect = (float)w / (float)(h != 0.0f ? h : 1.0f);
	if (drawHitTest) { gluPickMatrix(hitTestX, glutGet(GLUT_WINDOW_HEIGHT) - hitTestY, 1, 1, viewport); }

	// Perspective projection
	//gluPerspective(60.0f, aspect, 0.1f, 100.0f);
	//glFrustum(-w / 4, w / 4, -h / 4 * (1.0f/aspect), h / 4 * (1.0f/aspect), 0.0f, 1000.0f);

	//myPerspective(90.0f, aspect, 0.1f, 10.0f);
	myPerspective(camFov, aspect, 0.1f, 10.0f);

	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);

	// Clear the model-view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LIGHTING);

	// Use the default GL Light settings
	glEnable(GL_LIGHT0);

	//glEnable(GL_COLOR_MATERIAL);

	// Stereo eye separation
	glTranslatef(eye * eyeSep / 2.0f, 0.0f, 0.0f);

//	glTranslatef(0.0f, 0.0f, -(1.0f/ASPECT_RATIO)/2);

	// Camera View
	glTranslatef(panX, panY, -panZ); 
	glRotatef(-camTwist, 0.0f, 0.0f, 1.0f );      
	glRotatef(camLat, 1.0f, 0.0f, 0.0f ); 
	glRotatef(-camLong, 0.0f, 1.0f, 0.0f );      
#ifdef YAW_OFFSET
glRotatef(-YAW_OFFSET, 0.0f, 1.0f, 0.0f );      
#endif

	//glTranslatef(-0.5f, -0.5f, -0.5f);

	//glScalef(1.0f, -1.0f, 1.0f);
	drawBoard();

    // Lock around access to data structure
//    ReaderLockMutex();


	{
		float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
		float diffuse[4] = {0.5f, 0.5f, 0.5f, 1};
		float specular[4] = {1, 1, 1, 1};
		//float position[4] = {0, 0, 0, 1};
		//float direction[4] = {0, 0, -1};
		//exponent = 0;
		//cutoff = 180;
		//constant_attenuation = 1;
		//linear_attenuation = 0;
		//quadratic_attenuation = 0;
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	}

    // TODO: Draw object
    {
#ifdef INTEGRATE
        // (towards, up, left)
        if (showOffset)
        {
            glTranslatef(position[0], position[1], position[2]);
        }
#endif
        RenderObject(&azimuth);
    }

    // Unlock - finished reading from data structure
//    ReaderUnlockMutex();

	// Restore state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();



	// Draw 2D scene

	// Save state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	// Get the viewport size
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	w = (float)viewport[2];
	h = (float)viewport[3];
	aspect = (float)w / (float)(h != 0.0f ? h : 1.0f);
	if (drawHitTest) { gluPickMatrix(hitTestX, glutGet(GLUT_WINDOW_HEIGHT) - hitTestY, 1, 1, viewport); }

	// Orthographic projection
	//glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1], viewport[1] + viewport[3], -1.0f, 1.0f);
	{
		float left = 0.0f, right = 1.0f;
		float bottom = 0.0f, top = 1.0f;
		float zNear = -1.0f, zFar = 1.0f;
		if (aspect >= 1.0f)
		{ 
			float aw = (aspect - 1) * (right - left);
			left -= aw / 2;
			right += aw / 2;
		}
		else
		{
			float ah = ((1.0f / aspect) - 1) * (top - bottom);
			bottom -= ah / 2;
			top += ah / 2;
		}
		glOrtho(left, right, bottom, top, zNear, zFar);
	}
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Clear the model-view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Restore state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

static void displayUI()
{
	int viewport[4];

	// Save state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	// Get the viewport size
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	if (drawHitTest) { gluPickMatrix(hitTestX, glutGet(GLUT_WINDOW_HEIGHT) - hitTestY, 1, 1, viewport); }
	// Orthographic projection
	//glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glOrtho(0.0f, viewport[2], viewport[3], 0.0f, -1.0f, 1.0f);

	// Clear the model-view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw scene
	drawUI(viewport);

	// Restore state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

static void display(void)
{
#ifdef THREADED
    mutex_lock(&mutex);
#endif

	// Clear the buffer
	if (!drawHitTest)
	{
		glClearColor(backgroundColor[0] / 255.0f, backgroundColor[1] / 255.0f, backgroundColor[2] / 255.0f, backgroundColor[3] / 255.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
	}

	if (glutGet(GLUT_WINDOW_STEREO))
	{
		glDrawBuffer(GL_BACK_LEFT);
		displayWorld(1);
		glDrawBuffer(GL_BACK_RIGHT);
		displayWorld(-1);
		glDrawBuffer(GL_BACK);
	} else {
		displayWorld(0);
	}
	displayUI();

	glFinish();

	if (!drawHitTest)
	{
        if (renderToDevice)
        {
	        int viewport[4];
            int x, y;

	        glGetIntegerv(GL_VIEWPORT, viewport);
            x = viewport[2] / 2 + viewport[0] - (image.width/2);
            y = viewport[3] / 2 + viewport[1] - (image.height/2);

            glReadBuffer(GL_BACK);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(x, y, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.buffer);

            if (edgeEnhance) { ImageEdgeEnhance(&image, 1); }

            {
                int ditherTypes[] = { 0, 1, 2, 3, 4, 8 };
                ImageDither(&image, ditherTypes[ditherIndex]);
            }

#if 1
            {
	            glPushAttrib(GL_ALL_ATTRIB_BITS);
	            glMatrixMode(GL_PROJECTION);
	            glPushMatrix();
	            glLoadIdentity(); 
	            glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1], viewport[1] + viewport[3], -1.0f, 1.0f);

                // Copy image back to buffer
                glPixelZoom(1.0f, 1.0f);
                glDrawBuffer(GL_BACK);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glRasterPos2i(x, y);
                glDrawPixels(image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.buffer);

	            glPopMatrix();
	            glMatrixMode(GL_MODELVIEW);
	            glPopAttrib();
            }
#endif

            ImageToDisplay(&image, &imageDisplay, IMAGE_TO_DISPLAY_INVERT);
        
            displayChanged = 1;  // DisplayToDevice(&azimuth, &imageDisplay);
        }


		glFlush();
		glutSwapBuffers();
	}

#ifdef THREADED
    mutex_unlock(&mutex);
#endif
}


static int findNameAt(int x, int y)
{
	#define selectBufferSize 128
	unsigned int selectBuffer[selectBufferSize];
	int closest = 0;
	bool found = false;

	hitTestX = x; hitTestY = y;

	glSelectBuffer(selectBufferSize, selectBuffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(NAME_NONE);
	//glLoadName(0);

	drawHitTest = true;
	display();
	drawHitTest = false;

	{
		int hits = glRenderMode(GL_RENDER);
		int s = 0;
		int closestZ = 0;
		int i;
		for (i = 0; i < hits; i++)
		{
			int num = selectBuffer[s++];
			int minZ = selectBuffer[s++];
			int maxZ = selectBuffer[s++];
			int n;
			for (n = 0; n < num; n++)
			{
				int name = selectBuffer[s++];
				if ((i == 0 && n == 0) || minZ <= closestZ)
				{
					found = true;
					closest = name;
					closestZ = minZ;
				}
			}
		}
	}

	return found ? closest : NAME_NONE;
}

static void setFullScreen(bool fullScreen)
{
	if (!isFullScreen)
	{
		restoreWindow[0] = glutGet(GLUT_WINDOW_X);
		restoreWindow[1] = glutGet(GLUT_WINDOW_Y);
		restoreWindow[2] = glutGet(GLUT_WINDOW_WIDTH);
		restoreWindow[3] = glutGet(GLUT_WINDOW_HEIGHT);
		if (fullScreen) {
			isFullScreen = true;
			glutFullScreen();
		}
	}
	else if (isFullScreen && !fullScreen)
	{
		isFullScreen = false;
		glutPositionWindow(restoreWindow[0], restoreWindow[1]);
		glutReshapeWindow(restoreWindow[2], restoreWindow[3]);
	}
}

static void setCursor(bool cursor)
{
	if (cursor)
	{
		//if (lastModifiers & GLUT_ACTIVE_SHIFT)
		//{
		//	glutSetCursor(GLUT_CURSOR_CYCLE);
		//}
		//else
		{
			//glutSetCursor(GLUT_CURSOR_INHERIT);
			glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		}
	}
	else 
	{
		glutSetCursor(GLUT_CURSOR_NONE);
	}
	isCursor = cursor;
}

static void reshape(int w, int h)
{
	float viewport[4];

	// Set viewport to whole of screen
	viewport[0] = 0.0f; viewport[1] = 0.0f;
	viewport[2] = (float)w; viewport[3] = (float)h;

	// Set viewport
	glViewport((int)viewport[0], (int)viewport[1], (int)viewport[2], (int)viewport[3]);

	//  glutPostRedisplay();  // prevents maximize scramble
	display();  // for viewing while resizing
}

/*
static void nudge(float x, float y, float z, float yaw, float pitch, float roll)
{
	float scale = 360.0f;
	t_vector accel;
	t_vector gyro;

	accel.x = 0.0f; // position.gx + x; 
	accel.y = 0.0f; // position.gy + y; 
	accel.z = 0.0f; // position.gz + z;

	gyro.x = roll * scale; 
	gyro.y = pitch * scale; 
	gyro.z = yaw * scale;

	updateIMU(&position, &accel, &gyro);

	//position.yaw += y;
	//position.pitch += p;
	//position.roll += r;
}
*/

static void keyboard(unsigned char key, int x, int y)
{
	bool changed = true;
	lastModifiers = glutGetModifiers();
	if (key == 27)
	{
		if (isFullScreen) 
		{
			setFullScreen(false);
			return;
		}
		else
		{
			exit(0);
		}
	}
	if (key == 32)
	{
		//position.x = 0.0f;
		//position.y = 0.0f;
		//position.z = 0.0f;
		//position.yaw = 0.0f;
		//position.pitch = 0.0f;
		//position.roll = 0.0f;
        AzimuthReset(&azimuth);
	}
	if (key == 'f' || key == 'F')
	{
		setFullScreen(!isFullScreen);
	}
	if (key == 'c' || key == 'C')
	{
		setCursor(!isCursor);
	}
	if ((key == '-' || key == '_') && panZ > 0.1f) { panZ -= 0.01f; printf("PAN-Z: %.2f\n", panZ); changed = true; }
	if ((key == '+' || key == '=') && panZ < 5.0f) { panZ += 0.01f; printf("PAN-Z: %.2f\n", panZ); changed = true; }
	if (key == '[' && camLat > -90.0f) { camLat -= 0.5f; printf("CAMERA-LAT: %.2f\n", camLat); changed = true; }
	if (key == ']' && camLat < 0.0f) { camLat += 0.5f; printf("CAMERA-LAT: %.2f\n", camLat); changed = true; }
	if (key == ',' || key == '<') { camLong -= 0.5f; printf("CAMERA-LONG: %.2f\n", camLong); changed = true; }
	if (key == '.' || key == '>') { camLong += 0.5f; printf("CAMERA-LONG: %.2f\n", camLong); changed = true; }
	if (key == '{' && camFov > 10.0f) { camFov -= 0.5f; printf("CAMERA-FOV: %.2f\n", camFov); changed = true; }
	if (key == '}' && camFov < 120.0f) { camFov += 0.5f; printf("CAMERA-FOV: %.2f\n", camFov); changed = true; }

	if (lastModifiers & GLUT_ACTIVE_SHIFT)
	{
		//if (key == 'w') { nudge(0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 0.0f); }    // Forward along Z
		//if (key == 's') { nudge(0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f); }    // Backward along Z
		//if (key == 'a') { nudge( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); }    // Forward along X
		//if (key == 'd') { nudge(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); }    // Backward along X
		//if (key == 'w') { nudge(0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f); }    // Forward along Y
		//if (key == 's') { nudge(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f); }    // Backward along Y
	}
	else
	{
		//if (key == 'w') { nudge(0.0f, 0.0f, 0.0f, 0.0f,  1.0f, 0.0f); }    // Pitch up
		//if (key == 's') { nudge(0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f); }    // Pitch down
		//if (key == 'a') { nudge(0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f); }    // Yaw left
		//if (key == 'd') { nudge(0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f); }    // Yaw right
		//if (key == 'q') { nudge(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f); }    // Roll left
		//if (key == 'e') { nudge(0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  1.0f); }    // Roll right
	}

	if (key == 13)
	{
		if (lastModifiers & GLUT_ACTIVE_ALT)
		{
			setFullScreen(!isFullScreen);
		}
	}
	if (changed) { glutPostRedisplay(); }
}

static void keyboardUp(unsigned char key, int x, int y)
{
	lastModifiers = glutGetModifiers();
}

static void special(int key, int x, int y)
{
	bool changed = false;
	lastModifiers = glutGetModifiers();
	if (key == GLUT_KEY_F1)
	{
        renderToDevice = !renderToDevice;
	}
	if (key == GLUT_KEY_F2)
	{
        ditherIndex = (ditherIndex + 1) % 6;     // 0,1,2,3,4,8
	}
	if (key == GLUT_KEY_F3)
	{
        edgeEnhance = !edgeEnhance;
	}
	if (key == GLUT_KEY_F11)
	{
		setFullScreen(!isFullScreen);
	}
	if (key == GLUT_KEY_F4)
	{
		setCursor(!isCursor);
	}
	if (key == GLUT_KEY_F5)
	{
		showGraph = !showGraph;
	}
	if (key == GLUT_KEY_F6)
	{
#ifdef INTEGRATE
		showOffset = !showOffset;
#endif
	}
	if (key == GLUT_KEY_F12)
	{
		exit(0);
	}

	setCursor(isCursor);

	if (changed) { glutPostRedisplay(); }
}

static void specialUp(int key, int x, int y)
{
	lastModifiers = glutGetModifiers();
}

static void mouse(int button, int state, int x, int y)
{
	bool changed = false;
	int newButtons = lastButtons;
	if (state == GLUT_DOWN) { newButtons |= (1 << button); }
	else if (state == GLUT_UP) { newButtons &= ~(1 << button); }
	lastCursorX = x; lastCursorY = y;

	// button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON || button == GLUT_RIGHT_BUTTON
	lastModifiers = glutGetModifiers();

	//if (!(lastModifiers & GLUT_ACTIVE_SHIFT))
	{
	//	if (state == GLUT_DOWN)
		{
	//		int hit = findNameAt(x, y);
			//if (hit == NAME_XXX)
		}
	}

	lastButtons = newButtons;
	if (changed) { glutPostRedisplay(); }
}

static void motion(int x, int y)
{
	bool changed = false;
	float dx = (float)x - lastCursorX, dy = (float)y - lastCursorY;

	bool b0 = (lastButtons & (1 << GLUT_LEFT_BUTTON)) != 0;
	bool b1 = (lastButtons & (1 << GLUT_MIDDLE_BUTTON)) != 0;
	bool b2 = (lastButtons & (1 << GLUT_RIGHT_BUTTON)) != 0;

	//if (lastModifiers & GLUT_ACTIVE_SHIFT)
	{
		if (b1 || (b0 && b2))
		{
			panX += dx * 0.001f;
			panY -= dy * 0.001f;
			changed = true;
		}
		else
		{
			if (b0)
			{
				camLong -= dx * 0.6f;
				camLat += dy * 0.6f;
				changed = true;
			}
			if (b2)
			{
				//camTwist += dx * 0.03f;
				panZ -= dy * 0.001f;
				changed = true;
			}
		}
		if (changed) { printf("CAMERA: @(%0.2f, %0.2f, %0.2f) (%0.2f, %0.2f, %0.2f) FOV=%0.2f\n", panX, panY, panZ, camLat, camLong, camTwist, camFov); }
	}


	lastCursorX = x; lastCursorY = y;

	if (isCursor) 
	{
		if (lastButtons != 0 && lastModifiers & GLUT_ACTIVE_SHIFT)
		{
			glutSetCursor(GLUT_CURSOR_INFO);
		}
		else
		{
			int hit = findNameAt(x, y);
			//glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		}
	}

	if (changed) { glutPostRedisplay(); }
}

static void passiveMotion(int x, int y)
{
	motion(x, y);
}

#ifdef TIMER_RATE
static void timer(int id)
{ 
	static unsigned long previous = 0;
	unsigned long current = glutGet(GLUT_ELAPSED_TIME);
	unsigned long elapsed = previous ? current - previous : 0;
	bool changed = false;

	previous = current;

//objectPosition.z += 0.01;
//changed = true;

	//clock_t now = clock();
	//int duration = (int)((float)(now - status.openTime) * 1000.0f / CLOCKS_PER_SEC);

	if (changed) { glutPostRedisplay(); }
	glutTimerFunc(TIMER_RATE, timer, 0);
}
#endif

static void updated(azimuth_t *az, void *reference)
{
    float mag[3];
    float extra[3];

#ifdef THREADED
    mutex_lock(&mutex);
#endif

    mag[0] = azimuth.mag[0] - ((azimuth.magMax[0] + azimuth.magMin[0]) / 2);
    mag[1] = azimuth.mag[1] - ((azimuth.magMax[1] + azimuth.magMin[1]) / 2);
    mag[2] = azimuth.mag[2] - ((azimuth.magMax[2] + azimuth.magMin[2]) / 2);

    // Add graph points
    GraphAddPoint(&graphs[0], azimuth.accel); graphs[0].scale = 1.0f / 4.0f; 
    GraphAddPoint(&graphs[1], azimuth.gyro);  graphs[1].scale = 1.0f / 90.0f * 2.0f; 
    GraphAddPoint(&graphs[2], mag);           graphs[2].scale = 1.0f / 40.0f; 

    extra[0] = ((float)azimuth.temperature - 200.0f) / 100.0f;
    extra[1] = ((float)azimuth.pressure - azimuth.pressureOffset) / 30.0f;
    extra[2] = (((float)azimuth.battery) / 4300.0f * 2) - 1.0f;
    GraphAddPoint(&graphs[3], extra);         graphs[3].scale = 1.0f; 

#ifdef INTEGRATE
    {
        int i;
        float accelMagnitude;
        static int stationarySamples = 0;

        accelMagnitude = (float)sqrt(azimuth.linearAccel[0] * azimuth.linearAccel[0] + azimuth.linearAccel[1] * azimuth.linearAccel[1] + azimuth.linearAccel[2] * azimuth.linearAccel[2]);
        if (accelMagnitude < 0.05f)
        {
            stationarySamples++;
            if (stationarySamples > 10)
            {
                // re-zero
                for (i = 0; i < 3; i++) { IntegratorInit(&velocityIntegrator[i]); IntegratorInit(&positionIntegrator[i]); }
            }
        }
        else
        {
            stationarySamples = 0;
        }

        // integrate
        for (i = 0; i < 3; i++)
        {
            float accel =  azimuth.linearAccel[i] * 9.81f / azimuth.frequency;  // In m/s^2
if (accelMagnitude < 0.05f) { accel = 0.0f; }
            IntegratorAdd(&velocityIntegrator[i], accel);
            velocity[i] = IntegratorGet(&velocityIntegrator[i]) / azimuth.frequency;
            IntegratorAdd(&positionIntegrator[i], velocity[i]);

            #define MAX_RANGE 0.4f
            if (positionIntegrator[i].value >  MAX_RANGE) { positionIntegrator[i].value =  MAX_RANGE; }
            if (positionIntegrator[i].value < -MAX_RANGE) { positionIntegrator[i].value = -MAX_RANGE; }

            position[i] = IntegratorGet(&positionIntegrator[i]);
        }

        //printf("(%+6.2f,%+6.2f,%+6.2f); ", azimuth.linearAccel[0], azimuth.linearAccel[1], azimuth.linearAccel[2]);
        //printf("(%+6.2f,%+6.2f,%+6.2f); ", velocity[0], velocity[1], velocity[2]);
        //printf("(%+6.2f,%+6.2f,%+6.2f); ", position[0], position[1], position[2]);
        //printf("\n");
    }
#endif

    if (displayChanged)
    {
        static int lastUpdate = 0;
	    int time = glutGet(GLUT_ELAPSED_TIME);
        if (lastUpdate == 0 || (time - lastUpdate) > 1000/30)
        {
            displayChanged = 0;
            DisplayToDevice(&azimuth, &imageDisplay);
            lastUpdate = time;
        }
    }

    changed = 1;

#ifdef THREADED
    mutex_unlock(&mutex);
#endif
}

static void idle(void)
{
#ifndef THREADED
    // TODO: Remove this hack (threaded read?)
if (1) for (;;)
    {
        unsigned long long start = TicksNow();
        unsigned long long elapsed;

        if (!AzimuthPoll(&azimuth)) { break; }

        updated(&azimuth, NULL);

        elapsed = TicksNow() - start;
        if (elapsed > 8) 
        { break; }
    }
#endif
    if (changed)
    {
        changed = 0;
        glutPostRedisplay();
    }
    else
    {
        usleep(10 * 1000);
    }
}

static int init(int argc, char *argv[])
{
}

/*
static void readerCallback(char *line)
{
    #define MAX_TOKENS 32
    int numTokens;
    char *tokens[MAX_TOKENS] = {0};
    char *start, *p, c;

    if (line == NULL || line[0] == '\0') { return; }
    while (*line == ' ') line++;            // Remove any leading whitespace

//	printf("READ: %s\n", line);

    numTokens = 0;
    start = line;
    p = line;
    do
    {
        c = *p;
        if (c == '\0' || c == ',')      // End of token
        {
            *p = '\0';                  // Null-terminate token
            tokens[numTokens] = start;
            numTokens++;
            start = p + 1;
        }
        p++;
    } while (c != '\0');

    // Process line
    {
        int firstToken = 0;

        // Skip first token?
        if (tokens[0][0] == '$' && tokens[0][1] == 'A' && tokens[0][2] == 'G' && tokens[0][3] == 'M') { firstToken = 1; }  // $AGM
        else if (tokens[0][4] == '/') { firstToken = 1; }  // YYYY/MM/DD hh:mm:ss.00

        if (numTokens - firstToken == 6 || numTokens - firstToken == 9 || numTokens - firstToken > 9)
        {

// Debug out
//fprintf(stderr, "INPUT:  %f,%f,%f,%f,%f,%f\n", accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z);
//fprintf(stderr, "INPUT:  %+1.2f,%+1.2f,%+1.2f -> ", accel[0], accel[1], accel[2]);

// HACK: Needs this offset to correct the z axis
#if 0
#define SCALE(min, max, value) ((value - ((max + min) / 2)) / (max - min) * 2)
accel.z = SCALE(-0.810f, 1.180f, accel.z);
accel.y = SCALE(-0.975f, 1.040f, accel.y);
accel.x = SCALE(-1.025f, 1.015f, accel.x);
#endif

//fprintf(stderr, "%-1.2f,%-1.2f,%-1.2f\n", accel.x, accel.y, accel.z);

			// Update the IMU
			//updateIMU(&position, &gyro&, accel);

#ifdef REPOSITION_ORIGIN
position[0] = 0.0f; position[1] = 0.0f; position[2] = 0.0f;
#endif

// Debug out
//fprintf(stderr, "OUTPUT: %f,%f,%f,%f,%f,%f\n", position.x, position.y, position.z, position.roll, position.pitch, position.yaw);

            // Unlock - finished writing to data structure
//            ReaderUnlockMutex();

            // Redisplay
//           	glutPostRedisplay();
        }
		else
		{
			printf("ERROR-PARSING: %s\n", line);
		}
    }

	return;
}
*/


static int startup()
{
	char *glVendor = NULL, *glRenderer = NULL, *glVersion = NULL, *glExtensions = NULL;

	glVendor = strdup((const char *)glGetString(GL_VENDOR));
	glRenderer = strdup((const char *)glGetString(GL_RENDERER));
	glVersion = strdup((const char *)glGetString(GL_VERSION));
	glExtensions = strdup((const char *)glGetString(GL_EXTENSIONS));

	printf("INFO: GL_VENDOR=%s\n", glVendor);
	printf("INFO: GL_RENDERER=%s\n", glRenderer);
	printf("INFO: GL_VERSION=%s\n", glVersion);
	//printf("INFO: GL_EXTENSIONS=%s\n", glExtensions);

    if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_ARB_multitexture") != NULL && strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_texture_env_combine") != NULL)
    {
        //glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTexelUnits);
        glMultiTexCoord1fARB     = (PFNGLMULTITEXCOORD1FARBPROC)     uglGetProcAddress("glMultiTexCoord1fARB");
        glMultiTexCoord2fARB     = (PFNGLMULTITEXCOORD2FARBPROC)     uglGetProcAddress("glMultiTexCoord2fARB");
        glMultiTexCoord3fARB     = (PFNGLMULTITEXCOORD3FARBPROC)     uglGetProcAddress("glMultiTexCoord3fARB");
        glMultiTexCoord4fARB     = (PFNGLMULTITEXCOORD4FARBPROC)     uglGetProcAddress("glMultiTexCoord4fARB");
        glActiveTextureARB       = (PFNGLACTIVETEXTUREARBPROC)       uglGetProcAddress("glActiveTextureARB");
        glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) uglGetProcAddress("glClientActiveTextureARB");
    }
    else
    {
        printf("ERROR: Required GL extensions (multi-texturing) not supported.\n");
//		return -1;
    }

	// Seed random number generator
	srand((unsigned int)time(NULL));
	// Initial settings
	setFullScreen(startFullscreen);
	setCursor(startCursor);

	floorTexture = LoadTexture("data/desk.bmp");
	subHull = LoadTexture("data/sub-hull.bmp");
	subFront = LoadTexture("data/sub-front.bmp");
	subBack = LoadTexture("data/sub-back.bmp");
	subFins = LoadTexture("data/sub-fins.bmp");
	sphereMapTexture = LoadTexture("data/spheremap3.bmp");

	return true;
}

static void stop(void)
{
    AzimuthClose(&azimuth);
#ifdef THREADED
    mutex_destroy(&mutex);
#endif
}

int main(int argc, char *argv[])
{
	int i;

	printf("IMU Demo  Inertial Measurement Unit Demonstration\n");
	printf("V1.26     Dan Jackson, 2011-2013\n");
	printf("\n");

	glutInit(&argc, argv);

    strcpy(initialCommand, DEFAULT_COMMAND);

	for (i = 1; i < argc; i++)
	{
		if (strcasecmp(argv[i], "--help") == 0 || strcasecmp(argv[i], "-help") == 0 || strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "-?") == 0 || strcasecmp(argv[i], "/h") == 0 || strcasecmp(argv[i], "/?") == 0)
		{
			showHelp = true;
		}
		else if (strcasecmp(argv[i], "-in") == 0)
		{
			inputFile = argv[++i];
		}
		else if (strcasecmp(argv[i], "-nostream") == 0)
		{
            strcpy(initialCommand, "");
		}
		else if (strcasecmp(argv[i], "-command") == 0 || strcasecmp(argv[i], "-init") == 0)
		{
            char *p = argv[++i];
            strcpy(initialCommand, p);
		}
		else if (strcasecmp(argv[i], "-f") == 0)
		{
			startFullscreen = true;
			printf("NOTE: Starting full-screen\n");
		} 
		else if (strcasecmp(argv[i], "-c") == 0)
		{
			startCursor = false;
			printf("NOTE: Starting with hidden cursor\n");
		} 
		else if (strcasecmp(argv[i], "-stereo") == 0)
		{
			startStereo = true;
			printf("NOTE: Setting stereo mode\n");
		}
		else if (strcasecmp(argv[i], "-eyeSep") == 0) { eyeSep = (float)atof(argv[++i]); printf("NOTE: Setting stereo eye sep: %f\n", eyeSep); }
		else if (strcasecmp(argv[i], "-panX") == 0) { panX = (float)atof(argv[++i]); printf("NOTE: Setting camera pan-x: %f\n", panX); }
		else if (strcasecmp(argv[i], "-panY") == 0) { panY = (float)atof(argv[++i]); printf("NOTE: Setting camera pan-y: %f\n", panY); }
		else if (strcasecmp(argv[i], "-panZ") == 0) { panZ = (float)atof(argv[++i]); printf("NOTE: Setting camera pan-z: %f\n", panZ); }
		else if (strcasecmp(argv[i], "-camLat") == 0) { camLat = (float)atof(argv[++i]); printf("NOTE: Setting camera latitude: %f\n", camLat); }
		else if (strcasecmp(argv[i], "-camLong") == 0) { camLong = (float)atof(argv[++i]); printf("NOTE: Setting camera longitude: %f\n", camLong); }
		else if (strcasecmp(argv[i], "-camTwist") == 0) { camTwist = (float)atof(argv[++i]); printf("NOTE: Setting camera twist: %f\n", camTwist); }
		else if (strcasecmp(argv[i], "-border") == 0)
		{
			borderSize = (float)atof(argv[++i]);
			printf("NOTE: Setting border size to %f\n", borderSize);
		}
		else if (strcasecmp(argv[i], "-colBackground") == 0)
		{
			printf("NOTE: Setting background color\n");
			if (!parseColor(argv[++i], backgroundColor)) { printf("ERROR: Problem parsing color value: %s\n", argv[i]); }
		}
		else if (strcasecmp(argv[i], "-colBoardDark") == 0)
		{
			printf("NOTE: Setting board-dark color\n");
			if (!parseColor(argv[++i], boardDarkColor)) { printf("ERROR: Problem parsing color value: %s\n", argv[i]); }
		}
		else if (strcasecmp(argv[i], "-colBoardLight") == 0)
		{
			printf("NOTE: Setting board-light color\n");
			if (!parseColor(argv[++i], boardLightColor)) { printf("ERROR: Problem parsing color value: %s\n", argv[i]); }
		}
		else if (strcasecmp(argv[i], "-colBorder") == 0)
		{
			printf("NOTE: Setting border color\n");
			if (!parseColor(argv[++i], borderColor)) { printf("ERROR: Problem parsing color value: %s\n", argv[i]); }
		}
		else 
		{
			printf("ERROR: Unknown parameter: %s\n", argv[i]);
			showHelp = true;
		}

	}

	if (showHelp)
	{
		printf("\n");
		printf("Parameters:\n");
        printf("  -init \"\\r\\nRATE X 1\\r\\nMODE 2\\r\\nSTREAM 3\\r\\n\"   Initialization command\n");
        printf("  -in \\\\.\\COM123          Manually specify serial port, '*' auto-detect CWA\n");
        printf("  -nostream               Don't send default 'stream' command\n");
        printf("  -delay <n>              Input delay\n");
		printf("  -help                   Displays this help screen\n");
		printf("  -f                      Full-screen\n");
		printf("  -c                      Hide cursor\n");
		printf("  -stereo                 Attempt stereo graphics mode\n");
		printf("  -eyeSep 0.03            Set stereo eye position (default: %0.2f)\n", DEFAULT_EYE_SEP);
		printf("  -pan{X|Y|Z} v           3D board position\n");
		printf("  -cam{Lat|Long|Twist} v  3D camera setting\n");
		printf("  -border 0               Extend counter border (0=off, 1=on, other=size)\n");
		printf("  -colBackground #1f1f1f  Background color*\n");
		printf("  -colBoardDark #000000   Checkerboard dark color*\n");
		printf("  -colBoardLight #0f0f0f  Checkerboard light color*\n");
		printf("  -colBorder #ffffff      Border color*\n");
		printf("\n");
		printf("  * Colors specified by long hex RGB (#RRGGBB), short form (#RGB), or name:\n");
		printf("      black,maroon,green,olive,navy,purple,teal,grey,\n");
		printf("      silver,red,lime,yellow,blue,magenta,cyan,white\n");
		printf("\n");
		return 1;
	}

	if (startStereo)
	{
		glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STEREO); 
		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) 
		{
			printf("WARNING: Stereo display mode not available\n");
			glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
		}
	}
	else
	{
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
	}
	if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE))
	{
		printf("ERROR: Display mode not available (RGB/depth/double-buffered)\n");
		return 1;
	}

	glutInitWindowSize(restoreWindow[2], restoreWindow[3]);
	glutInitWindowPosition(restoreWindow[0], restoreWindow[1]); 
	glutCreateWindow(windowTitle);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
#ifdef TIMER_RATE
	glutTimerFunc(TIMER_RATE, timer, 0);
#endif
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialUpFunc(specialUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);
	//glutEntryFunc(entry);

#ifdef _WIN32
	// Change window icon
	{
		HWND hWnd = WindowFromDC(wglGetCurrentDC());
		HICON hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(GLUT_ICON));
		if (hWnd && hIcon)
		{
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
	}
#endif

	if (!startup())
	{
		return 1;
	}

    ImageInit(&image, 128, 64);
    DisplayInit(&imageDisplay);

    for (i = 0; i < GRAPH_COUNT; i++)
    {
        GraphCreate(&graphs[i], 2048, 3, 1.0f);
    }

    AzimuthInit(&azimuth, inputFile, initialCommand, 100, BETA);
	azimuth.ahrs.mode = 1;		// 1 = Mayhony, -1 = Old Mayhony, other = Madgwick

#ifdef INTEGRATE
    for (i = 0; i < 3; i++)
    {
        IntegratorInit(&velocityIntegrator[i]);
        IntegratorInit(&positionIntegrator[i]);
    }
#endif

#ifdef THREADED
    mutex_init(&mutex, 0);
    AzimuthSetCallback(&azimuth, updated, NULL);
    AzimuthStartBackgroundPoll(&azimuth);
#endif

	atexit(stop);
	glutMainLoop();		// This function never returns (even at exit), using atexit() hook for shutdown code

	stop();

	return 0;
}

