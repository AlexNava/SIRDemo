// GLSL Stereogram Demo
// Copyright 2008 by Alessandro Nava
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA


#include <SDL/SDL.h>
#include <stdio.h>
//#include <sys/file.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define XRES 1024
#define YRES 512
#define RENDER_WIDTH 1024 //MAKE SURE THAT THIS
#define RENDER_HEIGHT 512 //AND THIS ARE POWERS OF 2
#define ASPECT_RATIO 14.0/8.0
#define STRIP_WIDTH RENDER_WIDTH/8
#define STRIP_HEIGHT RENDER_HEIGHT
#define NEAR_PLANE 1.0
#define FAR_PLANE 3.0

int loadFileAsString(char *dest, char *fileName);
int testLoop(int mode);

typedef struct												// Create A Structure
{
	GLubyte	*imageData;										// Image Data (Up To 32 Bits)
	GLuint	bpp;											// Image Color Depth In Bits Per Pixel.
	GLuint	width;											// Image Width
	GLuint	height;											// Image Height
	GLuint	texID;											// Texture ID Used To Select A Texture
} TextureImage;												// Structure Name

bool loadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
{
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type=GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)
	int i;

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)

 	if(	texture->width	<=0	||								// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return false;										// Return False
	}

	texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	for(i=0; i<(int)(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs
  //printf("OK, loaded texture id %d\n",texture[0].texID);
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered

	if (texture[0].bpp==24)									// Was The TGA 24 Bits
	{
		type=GL_RGB;										// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;											// Texture Building Went Ok, Return True
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[]){
	glutInit(&argc, argv);

	testLoop(0);
	return 0;
}

//------------------------------------------------------------------------------
int loadFileAsString(char *dest, char *fileName){
	char *temp=dest;
	FILE *fp;
	printf("Loading %s; fopen retured ",fileName);
	fp=fopen(fileName,"r");
	printf("%d.\n",fp);
	while(!feof(fp))
		*(temp++)=fgetc(fp);
	fclose(fp);
	*(temp)='\0';
	return strlen(dest);
}

//------------------------------------------------------------------------------
int testLoop(int mode){
	SDL_Surface *screen;

    /* Initialize the SDL library */
    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Clean up on exit */
    atexit(SDL_Quit);

	/*
	switch(mode){
	case 0:
		printf("Initializing: 640x480xdesktop OpenGL, simplified shader\n");
		break;
	case 1:
		printf("Initializing: 640x480xdesktop OpenGL, simplified shader, antialiasing 4x\n");
		break;
	case 2:
		printf("Initializing: 640x480xdesktop OpenGL, shader 3.0\n");
		break;
	case 3:
		printf("Initializing: 640x480xdesktop OpenGL, shader 3.0, antialiasing 4x\n");
		break;
	}
	*/

	/*
	switch(mode){
	case 0:
	case 2:
		break;
	case 1:
	case 3:
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,1) ;
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4) ;
	}
	*/

    screen = SDL_SetVideoMode((mode==0?XRES:0), (mode==0?YRES:0), 0, SDL_OPENGL|(mode==0?0:SDL_FULLSCREEN));
    if ( screen == NULL ) {
        printf("Couldn't set video mode: %s\n",SDL_GetError());
        exit(1);
    }


	GLenum err = glewInit();
	if (GLEW_OK != err){
		// Problem: glewInit failed, something is seriously wrong.
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	printf("Using GLEW %s\n", glewGetString(GLEW_VERSION));

	if(GLEW_ARB_texture_float)
		printf("ARB FP texture supported\n");
	else{
		printf("ARB FP texture NOT supported\n");
        // ARB_tex_float not available on virtualbox, but glTexImage2D(...GL_FLOAT...) works
        //exit(-1);
	}

	if(GLEW_EXT_framebuffer_object)
		printf("FBO supported\n");
	else{
		printf("FBO NOT supported\n");
		exit(-1);
	}

	if(GLEW_EXT_framebuffer_multisample)
		printf("FB multisample supported\n");
	else
		printf("FB multisample NOT supported\n");

	if(GLEW_ARB_texture_non_power_of_two)
		printf("npot textures supported\n");
	else
		printf("npot textures NOT supported\n");


	GLint samples,buffers;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
	glGetIntegerv(GL_SAMPLES, &samples);

	char *vs,*fs;

	GLint v,f,p;

	vs=(char*)malloc(20000*sizeof(char));
	fs=(char*)malloc(20000*sizeof(char));
	printf("loaded vertex shader source: %d bytes\n",loadFileAsString(vs,"../data/stereo.vert"));
	printf("loaded fragment shader source: %d bytes\n",loadFileAsString(fs,"../data/stereo.frag"));

	const char * vv = vs;
	const char * ff = fs;


	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	p = glCreateProgram();

	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);

	glValidateProgram(p);
	int validated;
	glGetProgramiv(p,GL_VALIDATE_STATUS,&validated);
	if(validated)
		printf("program successfully validated!\n");
	char log[20000];
	glGetProgramInfoLog(p,20000,NULL,log);
	printf("program info log:\n%s\n",log);


	/*
	Create a framebuffer object
	*/

	GLuint fb[1];

	glGenFramebuffersEXT(1,fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fb[0]);

	/*
	Create 3 textures
	*/

	TextureImage textureToLoad[1];
	GLuint tex[3];

	glGenTextures(3,tex);
	glBindTexture(GL_TEXTURE_2D,tex[0]);// the depth texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,RENDER_WIDTH,RENDER_HEIGHT,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);

	loadTGA(textureToLoad,"../data/stereo-strip.tga");// the strip texture
	tex[1]=textureToLoad->texID;

	glBindTexture(GL_TEXTURE_2D,tex[2]);// the temp texture, that contains the distorted strips
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,STRIP_WIDTH,STRIP_HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,tex[0],0);
//	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_2D,tex[1],0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			printf("Yeah!\n");
		break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			printf("Bad formats\n");
			/* choose different formats */
		break;
		default:
			printf("SHIT SHIT SHIT!\n");
			/* programming error; will fail on all hardware */
	}


    gluPerspective(60.0,ASPECT_RATIO,NEAR_PLANE,FAR_PLANE);

    glClearColor(0.0,0.0,0.5,1);// we dont't care too much about this
    glClearDepth(3.0);
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glMatrixMode(GL_MODELVIEW);
    glTranslatef(0,0,-2.0);
	glRotatef(30,1,0,0);

    int startTime,oldTime,time;
    int frames=0;
	startTime=oldTime=time=SDL_GetTicks();

	SDL_WM_SetCaption("Stereogram test",NULL);
	glColor3f(1.0,1.0,1.0);

	bool quitRequest=false;
	float offset;
	while (!quitRequest){

		if(SDL_QuitRequested()){
			printf("MANUALLY STOPPED!\n");
			quitRequest=true;
		}
		//todo: add SDL event handling

		frames++;

		/***********************************************************************
		*                      Main Cycle starts here                          *
		***********************************************************************/

		/*
		Draw the teapot depth on a texture
		*/

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fb[0]);// draw to depth texture
	    glViewport(0,0,RENDER_WIDTH,RENDER_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glEnable(GL_DEPTH_TEST);
		glutSolidTeapot(0.55);

		glRotatef((time-oldTime)*10.0/1000.0,0,1,0);// rotate 10 degrees per second

		/*
		Draw the first strip as an offset version of the base noise texture
		*/

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);// draw to main framebuffer

	    glViewport(0,0,STRIP_WIDTH,STRIP_HEIGHT);
		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		glPushMatrix();// save the perspective camera
		glLoadIdentity();
		glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex[1]);// base strip texture on first texture unit

		offset = (rand()%256)/256.0;// animate the background ("static" effect :)
		glBegin(GL_QUADS);
			glTexCoord2f(offset+0.0,offset+1.0);
			glVertex2f(0.0,1.0);
			glTexCoord2f(offset+1.0,offset+1.0);
			glVertex2f(1.0,1.0);
			glTexCoord2f(offset+1.0,offset+0.0);
			glVertex2f(1.0,0.0);
			glTexCoord2f(offset+0.0,offset+0.0);
			glVertex2f(0.0,0.0);
		glEnd();


		for(int i=1;i<=7;i++){

			/*
			Copy the pixels of the previous strip on a texture (tex[2])
			*/

			glBindTexture(GL_TEXTURE_2D, tex[2]);//distorted strip texture
			glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,(i-1)*STRIP_WIDTH,0,STRIP_WIDTH,STRIP_HEIGHT,0);


			/*
			Use the obtained texture to render the successive strip.
			*/

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tex[0]);// depth texture on second texture unit


		    glViewport(i*STRIP_WIDTH,0,STRIP_WIDTH,STRIP_HEIGHT);
			glLoadIdentity();
			glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);

			glUseProgram(p);// Enable shader
			glUniform1i(glGetUniformLocation(p,"noiseTexture"),0);// texture unit for the color
			glUniform1i(glGetUniformLocation(p,"depthTexture"),1);// texture unit for the depth
			glUniform1f(glGetUniformLocation(p,"nearPlane"),NEAR_PLANE);// OpenGL "near" plane
			glUniform1f(glGetUniformLocation(p,"farPlane"),FAR_PLANE);// OpenGL "far" plane

			glBegin(GL_QUADS);
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0,1.0);
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(i-1)/7.0,1.0);
				glVertex2f(0.0,1.0);
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0,1.0);
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB,i/7.0,1.0);
				glVertex2f(1.0,1.0);
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0,0.0);
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB,i/7.0,0.0);
				glVertex2f(1.0,0.0);
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0,0.0);
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB,(i-1)/7.0,0.0);
				glVertex2f(0.0,0.0);
			glEnd();

			glUseProgram(0);// Disable shader, back to fixed pipeline

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
		}

		/*
		Draw arrows on top of the screen to help aligning the image
		*/


		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);

		glViewport(0,0,XRES,YRES);
		glLoadIdentity();
		glOrtho(0.0,8.0,0.0,1.0,-1.0,1.0);

		glBegin(GL_TRIANGLES);
		for(int i=0;i<8;i++){
			glVertex2f(i+0.45,0.975);
			glVertex2f(i+0.55,0.975);
			glVertex2f(i+0.50,0.95);
		}
	 	glEnd();

		glPopMatrix();// restore the perspective camera


/*
		OLD STUFF

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex[0]);
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex[1]);

		glUseProgram(p);
		glUniform1i(glGetUniformLocation(p,"depthTexture"),0);
		glUniform1i(glGetUniformLocation(p,"noiseTexture"),1);

		//offset=(rand()%256)/64.0;// animate the background ("static" effect :)
		glBegin(GL_QUADS);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0,1.0);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,offset+0.0,offset+1.0);
			glVertex2f(0.0,1.0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0,1.0);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,offset+8.0,offset+1.0);
			glVertex2f(8.0,1.0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,1.0,0.0);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,offset+8.0,offset+0.0);
			glVertex2f(8.0,0.0);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0,0.0);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,offset+0.0,offset+0.0);
			glVertex2f(0.0,0.0);
		glEnd();
		glUseProgram(0);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);

		glBegin(GL_TRIANGLES);
		for(int i=0;i<8;i++){
			glVertex2f(i+0.45,0.975);
			glVertex2f(i+0.55,0.975);
			glVertex2f(i+0.50,0.95);
		}
		glEnd();

		glPopMatrix();
*/
		oldTime=time;
		time=SDL_GetTicks();

	    SDL_GL_SwapBuffers();
	}
	printf("Done. Average framerate: %.2f frames per second.\n\n",frames*1000.0/(time-startTime));
	SDL_Quit();
	return 0;
}
