
#include "towerengine.h"
#include "tresources.h"

void tSSAOAmbientLightingShader::Init(tGBuffer *gbuffer)
{
	InitScreenShader(ssao_ambient_lighting_shader_frag, "SSAO Ambient Lighting Shader");

	ssao_tex_uniform = GetUniformLocation("ssao_tex_uni");
	diffuse_tex_uniform = GetUniformLocation("diffuse_tex_uni");
	self_illumination_tex_uniform = GetUniformLocation("self_illumination_tex_uni");

	light_ambient_color_uniform = GetUniformLocation("light_ambient_color_uni");

	Bind();
	glUniform1i(diffuse_tex_uniform, gbuffer->GetTextureUnit(tGBuffer::DIFFUSE_TEX));
	glUniform1i(self_illumination_tex_uniform, gbuffer->GetTextureUnit(tGBuffer::SELF_ILLUMINATION_TEX));

	ssao_tex_unit = gbuffer->GetLastTextureUnit() + 1;
	glUniform1i(ssao_tex_uniform, ssao_tex_unit);
}

void tSSAOAmbientLightingShader::SetSSAOTexture(GLuint tex)
{
	glActiveTexture(GL_TEXTURE0 + ssao_tex_unit);
	glBindTexture(GL_TEXTURE_2D, tex);
}

void tSSAOAmbientLightingShader::SetAmbientLight(tVector color)
{
	glUniform3f(light_ambient_color_uniform, color.x, color.y, color.z);
}