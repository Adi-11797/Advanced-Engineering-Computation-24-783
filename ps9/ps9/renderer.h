#ifndef RENDERE_IS_INCLUDED
#define RENDERE_IS_INCLUDED
/* { */

#include <ysgl.h>
#include <stdio.h>
#include <vector>

std::vector <char> ReadTextFile(const char fn[]);

class RendererBase
{
public:
    GLuint programIdent;
    GLuint vertexShaderIdent,fragmentShaderIdent;

	bool CompileFile(const char vtxShaderFn[],const char fragShaderFn[]);
    bool Compile(const std::vector <char> &vtxShaderSource,const std::vector <char> &fragShaderSource);
protected:
    bool CompileShader(int shaderIdent);
    bool LinkShader(void);
    virtual void CacheAttributeAndUniformIdent(void)=0;
};

class Gouraud3dRenderer : public RendererBase
{
public:
    GLuint attribVertexPos;
    GLuint attribNormalPos;
    GLuint attribColorPos;

    GLuint uniformProjectionPos;
    GLuint uniformModelViewPos;
    GLuint uniformLightDirPos;
    GLuint uniformAmbientPos;
    GLuint uniformSpecularIntensityPos;
    GLuint uniformSpecularExponentPos;

    virtual void CacheAttributeAndUniformIdent(void);
};

class BezierRenderer : public RendererBase
{
public:
    GLuint attribParamPos;

    GLuint uniformProjectionPos;
    GLuint uniformModelViewPos;
    GLuint uniformCtpPos;

    virtual void CacheAttributeAndUniformIdent(void);
};

/* } */
#endif
