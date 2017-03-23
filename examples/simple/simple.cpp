
#include <towerengine.h>
#include <GLFW/glfw3.h>

const int screen_width = 640;
const int screen_height = 480;

int main()
{
	GLFWwindow *window;

	if(!glfwInit())
		return 1;

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(screen_width, screen_height, "Simple TowerEngine Demo", 0, 0);
	if(!window)
	{
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	if(!tEngine::Init())
	{
		glfwTerminate();
		return 1;
	}

	tWorld *world = new tWorld();
	tScene *scene = new tScene(world);
	if(!scene->LoadFromFile("assets/cube.tes"))
	{
		fprintf(stderr, "Failed to load scene. Make sure to run this program in the examples/.\n");
		delete scene;
		delete world;
		glfwTerminate();
		return 1;
	}
	scene->AddToWorld();

	tDefaultDeferredRenderer *renderer = new tDefaultDeferredRenderer(screen_width, screen_height, world);

	tCamera *camera = renderer->GetCamera();
	camera->SetFOVVerticalAngle(60.0f, (float)screen_width / (float)screen_height);
	camera->SetPosition(Vec(3.0, 3.0, 3.0));
	camera->SetDirection(Vec(-1.0, -1.0, -1.0).Normalized());

	while(!glfwWindowShouldClose(window))
	{
		renderer->Render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}