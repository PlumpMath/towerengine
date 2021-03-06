
#include "towerengine.h"

using namespace std;



void tSceneObject::SetAttribute(string name, string value)
{
	attributes.insert(pair<string, string>(name, value));
}

string tSceneObject::GetAttribute(string name)
{
	map<string, string>::iterator i = attributes.find(name);

	if(i != attributes.end())
		return i->second;

	return string();
}

string tSceneObject::GetAttribute(string name, string default_v)
{
	map<string, string>::iterator i = attributes.find(name);

	if(i != attributes.end())
		return i->second;

	return default_v;
}

int tSceneObject::GetAttributeInt(string name, int default_v)
{
	map<string, string>::iterator i = attributes.find(name);

	if(i != attributes.end())
		return atoi(i->second.c_str());

	return default_v;
}

float tSceneObject::GetAttributeFloat(string name, float default_v)
{
	map<string, string>::iterator i = attributes.find(name);

	if(i != attributes.end())
		return (float)atof(i->second.c_str());

	return default_v;
}

// ---------------------------------------------

tObjectSceneObject::tObjectSceneObject(tObject *object)
{
	this->object = object;
}

tObjectSceneObject::~tObjectSceneObject(void)
{
	delete object;
}

void tObjectSceneObject::AddToWorld(tWorld *world)
{
	world->AddObject(object);
}

void tObjectSceneObject::RemoveFromWorld(tWorld *world)
{
	world->RemoveObject(object);
}

// ---------------------------------------------

tDirectionalLightSceneObject::tDirectionalLightSceneObject(tDirectionalLight *light)
{
	this->light = light;
}

tDirectionalLightSceneObject::~tDirectionalLightSceneObject(void)
{
	delete light;
}

void tDirectionalLightSceneObject::AddToWorld(tWorld *world)
{
	world->AddObject(light);
}

void tDirectionalLightSceneObject::RemoveFromWorld(tWorld *world)
{
	world->RemoveObject(light);
}

// ---------------------------------------------

tPointLightSceneObject::tPointLightSceneObject(tPointLight *light)
{
	this->light = light;
}

tPointLightSceneObject::~tPointLightSceneObject(void)
{
	delete light;
}

void tPointLightSceneObject::AddToWorld(tWorld *world)
{
	world->AddObject(light);
}

void tPointLightSceneObject::RemoveFromWorld(tWorld *world)
{
	world->RemoveObject(light);
}

// ---------------------------------------------

tEmptySceneObject::tEmptySceneObject(tTransform transform)
{
	this->transform = transform;
}

tEmptySceneObject::~tEmptySceneObject(void)
{
}

// ---------------------------------------------

tReflectionProbeSceneObject::tReflectionProbeSceneObject(tReflectionProbe *reflection)
{
	this->reflection = reflection;
}

tReflectionProbeSceneObject::~tReflectionProbeSceneObject()
{
	delete reflection;
}

void tReflectionProbeSceneObject::AddToWorld(tWorld *world)
{
	world->AddObject(reflection);
}

void tReflectionProbeSceneObject::RemoveFromWorld(tWorld *world)
{
	world->AddObject(reflection);
}
