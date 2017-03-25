
#include "towerengine.h"

using namespace std;

void tForwardRenderer::InitForwardRenderer(tWorld *world)
{
	InitRenderer(world);

	standard_shader = new tStandardForwardShader();

	point_lights_buffer = new tLightingShaderPointLightsBuffer();
}

tForwardRenderer::~tForwardRenderer()
{
	delete standard_shader;
	delete point_lights_buffer;
}

void tForwardRenderer::PrepareRender(tCamera *camera, tRenderSpace *render_space)
{
	tRenderer::PrepareRender(camera, render_space);
}

void tForwardRenderer::StandardForwardPass()
{
	SetCurrentFaceShader(standard_shader);
	BindCurrentFaceShader();

	// point lights
	vector<tPointLight *> point_lights;

	set<tPointLight *>::iterator point_light_sit;
	for(point_light_sit=current_rendering_render_space->point_lights.begin(); point_light_sit!=current_rendering_render_space->point_lights.end(); point_light_sit++)
	{
		tPointLight *light = *point_light_sit;
		if(!light->GetEnabled())
			continue;
		point_lights.push_back(light);
	}

	point_lights_buffer->UpdateBuffer(point_lights);
	point_lights_buffer->Bind();


	standard_shader->SetCameraPosition(current_rendering_camera->GetPosition());
	standard_shader->SetAmbientLight(world->GetAmbientColor());

	//SetReflections(lighting_shader, GetCurrentRenderingCamera()->GetPosition()); // TODO: For VR, a common position for both eyes has to be used


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	glEnable(GL_DEPTH_TEST);

	current_rendering_render_space->StandardForwardPass(this);
}