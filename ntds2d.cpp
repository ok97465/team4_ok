//---------------------------------------------------------------------------
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma hdrstop

#include "ntds2d.h"
#define RADPERDEG (asin(1.0f)/90.0f)
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
//---------------------------------------------------------------------------

#pragma package(smart_init)

#define PI 3.1415926535898
#define RADIUS 20.0
#define NUM_SPRITES 81
#define SPRITE_WIDTH   72
#define SPRITE_HEIGHT  72
#define SPRITE_CHANNELS 4

static GLuint AIR_TRACK_FRIEND;
static GLuint AIR_TRACK_HOSTILE;
static GLuint AIR_TRACK_UNKNOWN;
static GLuint SURFACE_TRACK_FRIEND ;
static GLuint TRACKHOOK;

static GLuint TextureSpites[NUM_SPRITES];
static GLuint TextureSpriteArray = 0;
static int NumSprites=0;

struct InstancingResources {
    bool initialized;
    GLuint vao;
    GLuint quadVBO;
    GLuint instanceVBO;
    GLuint program;
};

static InstancingResources gInstancing = {false};

// OpenGL extension function pointers for systems with only GL 1.1 headers
static bool gExtensionsLoaded = false;
static PFNGLCREATESHADERPROC           glCreateShader_ptr           = nullptr;
static PFNGLSHADERSOURCEPROC           glShaderSource_ptr           = nullptr;
static PFNGLCOMPILESHADERPROC          glCompileShader_ptr          = nullptr;
static PFNGLCREATEPROGRAMPROC          glCreateProgram_ptr          = nullptr;
static PFNGLATTACHSHADERPROC           glAttachShader_ptr           = nullptr;
static PFNGLLINKPROGRAMPROC            glLinkProgram_ptr            = nullptr;
static PFNGLDELETESHADERPROC           glDeleteShader_ptr           = nullptr;
static PFNGLUSEPROGRAMPROC             glUseProgram_ptr             = nullptr;
static PFNGLACTIVETEXTUREPROC          glActiveTexture_ptr          = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC     glGetUniformLocation_ptr     = nullptr;
static PFNGLUNIFORM1IPROC              glUniform1i_ptr              = nullptr;
static PFNGLGENVERTEXARRAYSPROC        glGenVertexArrays_ptr        = nullptr;
static PFNGLBINDVERTEXARRAYPROC        glBindVertexArray_ptr        = nullptr;
static PFNGLGENBUFFERSPROC             glGenBuffers_ptr             = nullptr;
static PFNGLBINDBUFFERPROC             glBindBuffer_ptr             = nullptr;
static PFNGLBUFFERDATAPROC             glBufferData_ptr             = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC    glVertexAttribPointer_ptr    = nullptr;
static PFNGLVERTEXATTRIBDIVISORPROC    glVertexAttribDivisor_ptr    = nullptr;
static PFNGLVERTEXATTRIBIPOINTERPROC   glVertexAttribIPointer_ptr   = nullptr;
static PFNGLDRAWARRAYSINSTANCEDPROC    glDrawArraysInstanced_ptr    = nullptr;
static PFNGLTEXIMAGE3DPROC             glTexImage3D_ptr             = nullptr;
static PFNGLTEXSUBIMAGE3DPROC          glTexSubImage3D_ptr          = nullptr;

#define glCreateShader           glCreateShader_ptr
#define glShaderSource           glShaderSource_ptr
#define glCompileShader          glCompileShader_ptr
#define glCreateProgram          glCreateProgram_ptr
#define glAttachShader           glAttachShader_ptr
#define glLinkProgram            glLinkProgram_ptr
#define glDeleteShader           glDeleteShader_ptr
#define glUseProgram             glUseProgram_ptr
#define glActiveTexture          glActiveTexture_ptr
#define glGetUniformLocation     glGetUniformLocation_ptr
#define glUniform1i              glUniform1i_ptr
#define glGenVertexArrays        glGenVertexArrays_ptr
#define glBindVertexArray        glBindVertexArray_ptr
#define glGenBuffers             glGenBuffers_ptr
#define glBindBuffer             glBindBuffer_ptr
#define glBufferData             glBufferData_ptr
#define glEnableVertexAttribArray glEnableVertexAttribArray_ptr
#define glVertexAttribPointer    glVertexAttribPointer_ptr
#define glVertexAttribDivisor    glVertexAttribDivisor_ptr
#define glVertexAttribIPointer   glVertexAttribIPointer_ptr
#define glDrawArraysInstanced    glDrawArraysInstanced_ptr
#define glTexImage3D             glTexImage3D_ptr
#define glTexSubImage3D          glTexSubImage3D_ptr

static void LoadGLExtensions()
{
    if (gExtensionsLoaded) return;
#ifdef _WIN32
    #define LOAD_PROC(type, name) name = (type)wglGetProcAddress(#name);
#else
    #define LOAD_PROC(type, name) name = (type)glXGetProcAddressARB((const GLubyte*)#name);
#endif

    LOAD_PROC(PFNGLTEXIMAGE3DPROC, glTexImage3D);
    LOAD_PROC(PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);
    LOAD_PROC(PFNGLCREATESHADERPROC, glCreateShader);
    LOAD_PROC(PFNGLSHADERSOURCEPROC, glShaderSource);
    LOAD_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);
    LOAD_PROC(PFNGLCREATEPROGRAMPROC, glCreateProgram);
    LOAD_PROC(PFNGLATTACHSHADERPROC, glAttachShader);
    LOAD_PROC(PFNGLLINKPROGRAMPROC, glLinkProgram);
    LOAD_PROC(PFNGLDELETESHADERPROC, glDeleteShader);
    LOAD_PROC(PFNGLUSEPROGRAMPROC, glUseProgram);
    LOAD_PROC(PFNGLACTIVETEXTUREPROC, glActiveTexture);
    LOAD_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
    LOAD_PROC(PFNGLUNIFORM1IPROC, glUniform1i);
    LOAD_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    LOAD_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
    LOAD_PROC(PFNGLGENBUFFERSPROC, glGenBuffers);
    LOAD_PROC(PFNGLBINDBUFFERPROC, glBindBuffer);
    LOAD_PROC(PFNGLBUFFERDATAPROC, glBufferData);
    LOAD_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    LOAD_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    LOAD_PROC(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor);
    LOAD_PROC(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer);
    LOAD_PROC(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced);
    #undef LOAD_PROC
    gExtensionsLoaded = true;
}


//---------------------------------------------------------------------------
int MakeAirplaneImages(void)
{
    LoadGLExtensions();
	bool hasAlpha=false;
	const char filename[] = "..\\..\\Symbols\\sprites-RGBA.png";

	int width = 0;
	int height = 0;
	int nrChannels=0;
	GLubyte *SpriteImage;
	GLubyte SpriteTexture[SPRITE_WIDTH][SPRITE_HEIGHT][SPRITE_CHANNELS];
	int x,y;

	NumSprites=0;
    //stbi_set_flip_vertically_on_load(true);
	SpriteImage= stbi_load(filename, &width, &height,&nrChannels,0);
	if (SpriteImage == NULL)
	{
		ShowMessage("Unable to load png file");
		return(0);
	}

	//ShowMessage("Image loaded "+IntToStr(width)+" "+IntToStr(height) + " channnels " +IntToStr(nrChannels));
	if (nrChannels==4) {
     hasAlpha=true;
	}
        glGenTextures(NUM_SPRITES, TextureSpites);
        glGenTextures(1, &TextureSpriteArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, TextureSpriteArray);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, hasAlpha ? GL_RGBA : GL_RGB,
                     SPRITE_WIDTH, SPRITE_HEIGHT, NUM_SPRITES, 0,
                     hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	for (int row = 0; row < 11; row++)
	{
	 for (int col = 0; col < 8; col++)
      {
		for (int x = 0; x < SPRITE_WIDTH; ++x)
         {
		  for (int y = 0; y < SPRITE_HEIGHT; ++y)
           {
			int index = (((y+row*SPRITE_HEIGHT)* width + x+(col*SPRITE_WIDTH))) * SPRITE_CHANNELS;
            SpriteTexture[x][y][0]= SpriteImage[index];
            SpriteTexture[x][y][1]= SpriteImage[index+1];
            SpriteTexture[x][y][2]= SpriteImage[index+2];
            SpriteTexture[x][y][3]= SpriteImage[index+3];
           }
         }

        glBindTexture(GL_TEXTURE_2D, TextureSpites[NumSprites]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, SPRITE_WIDTH,
                                   SPRITE_HEIGHT, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
                                   SpriteTexture);

        glBindTexture(GL_TEXTURE_2D_ARRAY, TextureSpriteArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, NumSprites,
                        SPRITE_WIDTH, SPRITE_HEIGHT, 1,
                        hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
                        SpriteTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_2D, 0);
	  NumSprites++;
	  if (NumSprites==NUM_SPRITES)
		{
		  stbi_image_free(SpriteImage);
		  glEnable(GL_TEXTURE_2D);
		  glShadeModel(GL_FLAT);
		  return(NumSprites);
		}
    }
  }
  stbi_image_free(SpriteImage);
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);
  return(NumSprites);
}
//---------------------------------------------------------------------------
void MakeAirTrackFriend(void)
{
 GLuint i;
 GLfloat cosine, sine;
 AIR_TRACK_FRIEND=glGenLists(1);
 glNewList(AIR_TRACK_FRIEND, GL_COMPILE);
 glPointSize(3.0);
 glLineWidth(2.0);
#if 1
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_POINT_SMOOTH );
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

 glBegin(GL_LINE_STRIP);
 for(i = 0; i <= 50; i++)
  {
   cosine = cos(i *  PI / 50.0)*RADIUS;
   sine = sin(i * PI/50.0)*RADIUS;
   glVertex2f(cosine, sine);
  }
 glEnd();

 glBegin(GL_POINTS);
 glVertex2f(0.0, 0.0);
 glEnd();
 glEndList();
}
//---------------------------------------------------------------------------
void MakeAirTrackHostile(void)
{
 AIR_TRACK_HOSTILE=glGenLists(1);
 glNewList(AIR_TRACK_HOSTILE, GL_COMPILE);
 glPointSize(3.0);
 glLineWidth(2.0);

 glBegin(GL_LINE_STRIP);
 glVertex2f(-10.0, 0.0);
 glVertex2f(0.0, 10.0);
 glVertex2f(10.0, 0.0);
 glEnd();

 glBegin(GL_POINTS);
 glVertex2f(0.0, 0.0);
 glEnd();
 glEndList();
}
//---------------------------------------------------------------------------
void MakeAirTrackUnknown(void)
{
 AIR_TRACK_UNKNOWN=glGenLists(1);
 glNewList(AIR_TRACK_UNKNOWN, GL_COMPILE);
 glPointSize(3.0);
 glLineWidth(2.0);

 glBegin(GL_LINE_STRIP);
 glVertex2f(-10.0, 0.0);
 glVertex2f(-10.0,10.0);
 glVertex2f(10.0, 10.0);
 glVertex2f(10.0, 0.0);
 glEnd();

 glBegin(GL_POINTS);
 glVertex2f(0.0, 0.0);
 glEnd();
 glEndList();
}
//---------------------------------------------------------------------------
void MakePoint(void)
{
 GLuint i;
 GLfloat cosine, sine;
 SURFACE_TRACK_FRIEND=glGenLists(1);
 glNewList(SURFACE_TRACK_FRIEND, GL_COMPILE);
 glPointSize(3.0);
 glLineWidth(3.0);
#if 1
 glEnable( GL_LINE_SMOOTH );
 glEnable( GL_POINT_SMOOTH );
 glEnable (GL_BLEND);
 glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

 glBegin(GL_LINE_STRIP);
 for(i = 0; i < 100; i++)
  {
   cosine = cos(i *2* PI / 100.0)*RADIUS;
   sine = sin(i * 2* PI/100.0)*RADIUS;
   glVertex2f(cosine, sine);
  }
 glEnd();

 glBegin(GL_LINE_STRIP);
 for(i = 0; i < 100; i++)
  {
   cosine = cos(i *2* PI / 100.0)*2.0;
   sine = sin(i * 2* PI/100.0)*2.0;
   glVertex2f(cosine, sine);
  }
 glEnd();

 glEndList();

}
//---------------------------------------------------------------------------
void MakeTrackHook(void)
{
 GLuint i;
 GLfloat cosine, sine;
 TRACKHOOK=glGenLists(1);
 glNewList(TRACKHOOK, GL_COMPILE);
 glPointSize(8.0);
 glLineWidth(10.0);
#if 1
 glEnable( GL_LINE_SMOOTH );
 glEnable( GL_POINT_SMOOTH );
 glEnable (GL_BLEND);
 glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

 glBegin(GL_LINE_STRIP);
 for(i = 0; i < 100; i++)
  {
   cosine = cos(i *2* PI / 100.0)*RADIUS*3;
   sine = sin(i * 2* PI/100.0)*RADIUS*3;
   glVertex2f(cosine, sine);
  }
 glEnd();
 glEndList();
}
 //---------------------------------------------------------------------------
void DrawAirplaneImage(float x, float y,float scale,float heading,int imageNum)
{
   glPushMatrix();
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, TextureSpites[imageNum]);
   glShadeModel(GL_FLAT);
   glTranslated(x,y,0.0);
   glRotatef(-heading-90.0, 0,0,1);
   glBegin(GL_QUADS);

   glTexCoord2f(1.0, 1.0);
   glVertex2f(36.0*scale,36.0*scale);      // top right

   glTexCoord2f(0.0, 1.0);
   glVertex2f(-36.0*scale, 36.0*scale);    // top left

   glTexCoord2f(0.0, 0.0);
   glVertex2f(-36.0*scale, -36.0*scale);  // bottom left

   glTexCoord2f(1.0, 0.0);
   glVertex2f(36.0*scale, -36.0*scale);    // bottom right

   glEnd();
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

static GLuint CompileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    return sh;
}

static GLuint CreateProgram(const char* vs, const char* fs)
{
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

void InitAirplaneInstancing()
{
    LoadGLExtensions();
    if(gInstancing.initialized) return;

    const char* vsSrc =
        "#version 330 core\n"
        "layout(location=0) in vec2 vert;\n"
        "layout(location=1) in vec2 uv;\n"
        "layout(location=2) in vec2 pos;\n"
        "layout(location=3) in float scale;\n"
        "layout(location=4) in float heading;\n"
        "layout(location=5) in int image;\n"
        "layout(location=6) in vec4 color;\n"
        "out vec2 Tex;\n"
        "out vec4 Color;\n"
        "flat out int Image;\n"
        "void main(){\n"
        "  float rad = radians(-heading - 90.0);\n"
        "  mat2 R = mat2(cos(rad), -sin(rad), sin(rad), cos(rad));\n"
        "  vec2 p = pos + R * (vert * scale * 36.0);\n"
        "  gl_Position = vec4(p, 0.0, 1.0);\n"
        "  Tex = uv;\n"
        "  Color = color;\n"
        "  Image = image;\n"
        "}\n";

    const char* fsSrc =
        "#version 330 core\n"
        "in vec2 Tex;\n"
        "in vec4 Color;\n"
        "flat in int Image;\n"
        "out vec4 Frag;\n"
        "uniform sampler2DArray spriteTex;\n"
        "void main(){\n"
        "  Frag = texture(spriteTex, vec3(Tex, Image)) * Color;\n"
        "}\n";

    gInstancing.program = CreateProgram(vsSrc, fsSrc);

    float quad[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
       -1.0f, 1.0f, 0.0f, 1.0f,
       -1.0f,-1.0f, 0.0f, 0.0f,
        1.0f,-1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &gInstancing.vao);
    glBindVertexArray(gInstancing.vao);

    glGenBuffers(1, &gInstancing.quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gInstancing.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    glGenBuffers(1, &gInstancing.instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gInstancing.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);

    size_t stride = sizeof(AirplaneInstance);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(AirplaneInstance, x));
    glVertexAttribDivisor(2,1);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(AirplaneInstance, scale));
    glVertexAttribDivisor(3,1);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(AirplaneInstance, heading));
    glVertexAttribDivisor(4,1);
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_INT, stride, (void*)offsetof(AirplaneInstance, imageNum));
    glVertexAttribDivisor(5,1);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(AirplaneInstance, color));
    glVertexAttribDivisor(6,1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gInstancing.initialized = true;
}

void DrawAirplaneImagesInstanced(const std::vector<AirplaneInstance>& instances)
{
    if(instances.empty()) return;
    if(!gInstancing.initialized)
        InitAirplaneInstancing();

    glBindVertexArray(gInstancing.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gInstancing.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size()*sizeof(AirplaneInstance), instances.data(), GL_STREAM_DRAW);

    glUseProgram(gInstancing.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, TextureSpriteArray);
    glUniform1i(glGetUniformLocation(gInstancing.program, "spriteTex"), 0);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, instances.size());

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
//---------------------------------------------------------------------------
void DrawAirTrackFriend(float x, float y)
 {
  glPushMatrix();
  glTranslated(x,y,0.0);
  glCallList(AIR_TRACK_FRIEND);
  glPopMatrix();
 }
 //---------------------------------------------------------------------------
void DrawAirTrackHostile(float x, float y)
 {
  glPushMatrix();
  glTranslated(x,y,0.0);
  glCallList(AIR_TRACK_HOSTILE);
  glPopMatrix();
 }
//---------------------------------------------------------------------------
void DrawAirTrackUnknown(float x, float y)
 {
  glPushMatrix();
  glTranslated(x,y,0.0);
  glCallList(AIR_TRACK_UNKNOWN);
  glPopMatrix();
 }
//---------------------------------------------------------------------------
 void DrawPoint(float x, float y)
 {
  glPushMatrix();
  glTranslated(x,y,0.0);
  glCallList(SURFACE_TRACK_FRIEND);
  glPopMatrix();
 }
//---------------------------------------------------------------------------
 void DrawTrackHook(float x, float y)
 {
  glPushMatrix();
  glTranslated(x,y,0.0);
  glCallList(TRACKHOOK);
  glPopMatrix();
 }
//---------------------------------------------------------------------------
 void  DrawRadarCoverage(float xc, float yc, float major, float minor)
 {
  glBegin(GL_TRIANGLE_FAN );
  glVertex2f( xc, yc); // Center.
  for( float a = 0;  a <= 360; a+=5 )
   {
	  float ang = (float) RADPERDEG*a;
	  float x = xc + (float) (major*cos( ang ));
	  float y = yc + (float) (minor*sin( ang ));
	  glVertex2f( x, y);
   }
  glEnd();
 }
//---------------------------------------------------------------------------
  void DrawLeader(float x1, float y1, float x2, float y2)
 {
   glBegin(GL_LINE_STRIP);
   glVertex2f(x1,y1);
   glVertex2f(x2,y2);
   glEnd();
 }
//---------------------------------------------------------------------------
 void ComputeTimeToGoPosition(float  TimeToGo,
							  float  xs, float  ys,
							  float  xv, float  yv,
							  float &xe, float &ye)
 {
  xe=xs+( xv/3600.00)*TimeToGo;
  ye=ys+( yv/3600.00)*TimeToGo;

 }
 //---------------------------------------------------------------------------
 void DrawLines(DWORD resolution, double xpts[],double ypts[])
{
  DWORD i;
	glBegin(GL_LINES);
	for (i=0; i<resolution; i++)
	{
		glVertex3f(xpts[i], ypts[i], 0.1f);
		glVertex3f(xpts[(i+1)%resolution], ypts[(i+1)%resolution], 0.1f);
	}
	glEnd();
}
 #if 0
//---------------------------------------------------------------------------
// SAVE ORIGINAL CODE   ** DO NOT DELETE 
//
  void DrawCovarianceEllipse(float c[][2], float cx,float cy, float sf)
  {
   float a,b,term1,term2,term3,term4;
   float major, minor, theta;
   //find direction of maximum variance
   a= 2.0*c[1][0];
   b= (c[0][0]-c[1][1]);
   if ((a==0.0)&& (b==0.0)) return;

   theta=0.5*atan2(a,b);
   //% find major and minor axis amplitudes
   term1=(c[0][0]+c[1][1]);
   term2=(c[0][0]-c[1][1]) *
			  (c[0][0]-c[1][1]) +
			  4.0*c[1][0]*c[1][0];
   if (term2<0.0) return;
   term2=sqrt(term2);
   term3=0.5*(term1+term2);
   term4=0.5*(term1-term2);
   if ((term3<0.0)|| (term4<0.0)) return;
   major=sqrt(term3);
   minor=sqrt(term4);
   theta*=DEGPERRAD;
   _DrawEllipse(theta, major*3, minor*3,cx,cy,sf );
 }
//---------------------------------------------------------------------------
// SAVE ORIGINAL CODE   ** DO NOT DELETE 
//
 //---------------------------------------------------------------------------
 void _DrawEllipse(float major_deg, float major_len, float minor_len, float cx,
				   float cy, float sf)
{
const int resolution = 100;
float xpts[resolution], ypts[resolution], newy[resolution], newx[resolution];
float theta, theta_step, major_rad;
int i;

	major_rad = major_deg * 3.1416f / 180.0f;
	theta_step = 2.0f * 3.1416f / (float)resolution;
	for (theta=0.0, i=0; i<resolution; i++, theta += theta_step)
	{
		xpts[i] = major_len * (float)cos(theta);
		ypts[i] = minor_len * (float)sin(theta);
	}
	for (i=0; i<resolution; i++)
	{
		newx[i] = (float)(cx + (xpts[i] * (float)cos(major_rad) - ypts[i] * (float)sin(major_rad)))*sf;
		newy[i] = (float)(cy + (xpts[i] * (float)sin(major_rad) + ypts[i] * (float)cos(major_rad)))*sf;
	}

	glBegin(GL_LINES);
	for (i=0; i<resolution; i++)
	{
		glVertex3f(newx[i], newy[i], 0.1f);
		glVertex3f(newx[(i+1)%resolution], newy[(i+1)%resolution], 0.1f);
	}
	//glVertex3f(newx[0], newy[0], 0.1f);
	//glVertex3f(newx[resolution/2], newy[resolution/2], 0.1f);
	//glVertex3f(newx[resolution/4], newy[resolution/4], 0.1f);
	//glVertex3f(newx[3*resolution/4], newy[3*resolution/4], 0.1f);
	glEnd();
}
 //---------------------------------------------------------------------------
#endif
 



 

