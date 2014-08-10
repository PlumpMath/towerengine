
#ifndef _DIRECTIONAL_SHADOW_SHADER_H
#define _DIRECTIONAL_SHADOW_SHADER_H

class CDirectionalShadowShader : public CFaceShader
{
	private:
		GLint light_dir_uniform;
		GLint clip_uniform;
		GLint cam_pos_uniform;

		GLint transformation_uniform;

		GLint vertex_mix_uniform;

	public:
		void Init(void);
		void SetLightDir(CVector v);
		void SetClip(float near, float far);
		void SetTransformation(const float m[16]);
		void SetVertexMix(float m = 0.0);
		void SetCamPos(CVector v);
};

#endif