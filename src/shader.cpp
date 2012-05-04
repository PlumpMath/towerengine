
#include "towerengine.h"

void CShader::SetSource(const char *vertex, const char *fragment)
{
	vertex_src = vertex;
	fragment_src = fragment;
}

void CShader::CreateVertexShader(void)
{
	vertex_shader = CreateShader(GL_VERTEX_SHADER_ARB, vertex_src);
}

void CShader::CreateFragmentShader(void)
{
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER_ARB, fragment_src);
}

void CShader::CreateProgram(void)
{
	program = CreateShaderProgram(vertex_shader, fragment_shader);
}

void CShader::LinkProgram(void)
{
	LinkShaderProgram(program);
}

void CShader::BindShader(void)
{
	UseNoShader();

	glUseProgramObjectARB(program);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ResetUniforms();
}

GLenum CubeTex(int side)
{
	switch(side)
	{	
		case 0:	return GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
		case 1:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
		case 2:	return GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
		case 3:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
		case 4:	return GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
		case 5:	return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
	};

	return 0;
}

int CubeSide(GLenum tex)
{
	switch(tex)
	{	
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:	return 0; break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:	return 1; break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	return 2; break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	return 3; break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	return 4; break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	return 5; break;
	};

	return 0;
}

CVector CubeVecS(int side)
{
	switch(side)
	{
		case 0:	return Vec(1.0, 0.0, 0.0); break;
		case 1:	return Vec(-1.0, 0.0, 0.0); break;
		case 2:	return Vec(0.0, 1.0, 0.0); break;
		case 3:	return Vec(0.0, -1.0, 0.0); break;
		case 4:	return Vec(0.0, 0.0, 1.0); break;
		case 5:	return Vec(0.0, 0.0, -1.0); break;
	};

	return Vec(0.0, 0.0, 0.0);
}


void PrintGLInfoLog(const char *log_title, GLhandleARB handle)
{
	GLchar *string;
	GLint size;

	size = 0;
	glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &size);
	if(size > 1)
	{
		string = new GLchar [size];
		glGetInfoLogARB(handle, size, &size, string);
		printf("%s Info log:\n", log_title);
		printf("-----------\n%s\n-----------\n", string);
		delete [] string;
	}
}


GLhandleARB CreateShader(GLenum type, const char *src)
{
	GLhandleARB s;
	GLint len;
	
	s = glCreateShaderObjectARB(type);
	if(!s)
	{
		printf("Could not create shader!\n");
		return 0;
	}


	len = strlen(src);
	glShaderSourceARB(s, 1, &src, &len);

	glCompileShaderARB(s);
	PrintGLInfoLog("Compile", s);

	return s;
}

GLhandleARB CreateShaderProgram(GLhandleARB vertex_shader, GLhandleARB fragment_shader)
{
	GLhandleARB p;

	p = glCreateProgramObjectARB();
	glAttachObjectARB(p, vertex_shader);
	glAttachObjectARB(p, fragment_shader);

	return p;
}

void LinkShaderProgram(GLhandleARB program)
{
	glLinkProgramARB(program);
	PrintGLInfoLog("Link", program);
}

void UseNoShader(void)
{
	glUseProgramObjectARB(0);
}

GLuint LoadGLTexture(const char *filename, int *w, int *h, bool *transparent, int alpha_channel) // from http://r3dux.org/2010/11/single-call-opengl-texture-loader-in-devil/
{
	ILuint imageID;
	GLuint textureID;
	ILboolean success;
	ILenum error;
	ILubyte *data;
	ILint width, height;
	int i;

	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	success = ilLoadImage(filename);

	if (!success)
	{
		error = ilGetError();
		std::cout << "Image load failed - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
		return 0;
	}

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);
	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
	{
		iluFlipImage();
	}
	success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	if (!success)
	{
		error = ilGetError();
		std::cout << "Image conversion failed - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
		return 0;
	}

	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	data = ilGetData();

	if(w != 0)
		*w = width;
	if(h != 0)
		*h = height;

	if(transparent != 0)
	{
		*transparent = false;
		for(i=0; i<width*height; i++)
		{
			if(data[i*4 + alpha_channel] < 255)
			{
				*transparent = true;
				break;
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 				// Type of texture
				 0,								// Pyramid level (for mip-mapping) - 0 is the top level
				 ilGetInteger(IL_IMAGE_BPP),	// Image colour depth
				 width,							// Image width
				 height,						// Image height
				 0,								// Border width in pixels (can either be 1 or 0)
				 ilGetInteger(IL_IMAGE_FORMAT),	// Image format (i.e. RGB, RGBA, BGR etc.)
				 GL_UNSIGNED_BYTE,				// Image data type
				 data);							// The actual image data itself

	ilDeleteImages(1, &imageID);

	return textureID;
}

GLuint GLTextureFromColor(const CVector &color)
{
	const int width = 64;
	const int height = 64;
	unsigned char data[width * height * 3];
	unsigned char *p;
	GLuint handle;
	int i;

	unsigned char r = (unsigned char)fmax(0.0, fmin(255.0, floor(color.x * 256.0)));
	unsigned char g = (unsigned char)fmax(0.0, fmin(255.0, floor(color.y * 256.0)));
	unsigned char b = (unsigned char)fmax(0.0, fmin(255.0, floor(color.z * 256.0)));

	p = data;
	for(i = 0; i < width * height; i++)
	{
		*p++ = r;
		*p++ = g;
		*p++ = b;
	}

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
	//printf("Color (%d, %d, %d) successfully loaded [handle = %d, width = %d, height = %d] :)\n", r, g, b, handle, width, height);

	return handle;
}


