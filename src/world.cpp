
#include "towerengine.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

using namespace std;

tWorld::tWorld(void)
{
	ambient_color = tVec(0.1, 0.1, 0.1);
	sky_box = 0;

	physics.broadphase = new btDbvtBroadphase();
	physics.collision_configuration = new btDefaultCollisionConfiguration();
	physics.collision_dispatcher = new btCollisionDispatcher(physics.collision_configuration);
	physics.solver = new btSequentialImpulseConstraintSolver();
	physics.dynamics_world = new btDiscreteDynamicsWorld(physics.collision_dispatcher, physics.broadphase, physics.solver, physics.collision_configuration);
	physics.dynamics_world->setGravity(btVector3(0, -10, 0));
	//physics.dynamics_world->getDispatchInfo().m_allowedCcdPenetration=0.0001f;

	physics.ghost_pair_callback = new btGhostPairCallback();
	physics.broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(physics.ghost_pair_callback);
}

tWorld::~tWorld(void)
{
	delete physics.dynamics_world;
	delete physics.solver;
	delete physics.collision_dispatcher;
	delete physics.collision_configuration;
	delete physics.broadphase;
	delete physics.ghost_pair_callback;

	/*for(vector<tObject *>::iterator i=objects.begin(); i!=objects.end(); i++)
		delete *i;

	for(vector<tPointLight *>::iterator i=point_lights.begin(); i!=point_lights.end(); i++)
		delete *i;

	for(vector<tDirectionalLight *>::iterator i=dir_lights.begin(); i!=dir_lights.end(); i++)
		delete *i;

	for(vector<tParticleSystem *>::iterator i=particle_systems.begin(); i!=particle_systems.end(); i++)
		delete *i;

	for(vector<tReflectionProbe *>::iterator i=reflection_probes.begin(); i!=reflection_probes.end(); i++)
		delete *i;*/
}

void tWorld::AddObject(tObject *o)
{
	vector<tObject *>::iterator i;

	for(i=objects.begin(); i!=objects.end(); i++)
		if(*i == o)
			return;

	objects.push_back(o);

	if(tReflectionProbe *reflection_probe = dynamic_cast<tReflectionProbe *>(o))
		reflection_probes.insert(reflection_probe);

	//if(tParticleSystem *particle_system = dynamic_cast<tParticleSystem *>(o))
	//	particle_systems.insert(particle_system);

	o->AddedObjectToWorld(this);
}

void tWorld::RemoveObject(tObject *o)
{
	vector<tObject *>::iterator i;

	for(i=objects.begin(); i!=objects.end(); i++)
		if(*i == o)
		{
			objects.erase(i);
			o->RemovedObjectFromWorld(this);
			return;
		}

	if(tReflectionProbe *reflection_probe = dynamic_cast<tReflectionProbe *>(o))
		reflection_probes.erase(reflection_probe);

	//if(tParticleSystem *particle_system = dynamic_cast<tParticleSystem *>(o))
	//	particle_systems.erase(particle_system);
}

void tWorld::FillObjectSpace(tObjectSpace *space, tCulling **cullings, int cullings_count, bool clear, bool init_cullings)
{
	if(clear)
		space->Clear();

	if(init_cullings)
		for(int c=0; c<cullings_count; c++)
			cullings[c]->InitCulling();

	for(tObject *object : objects)
	{
		bool cull = true;

		tBoundingBox bounding_box = object->GetBoundingBox();
		for(int c=0; c<cullings_count; c++)
		{
			if(!cullings[c]->TestBoundingBoxCulling(bounding_box))
			{
				cull = false;
				break;
			}
		}

		if(cull)
			continue;

		space->AddObject(object);
	}
}


/*void tWorld::FillRenderSpace(tRenderObjectSpace *space, tCulling **cullings, int cullings_count, bool init_cullings)
{
	tBoundingBox b;
	//tVector minv, maxv;
	tVector light_pos;
	float light_dist;
	vector<tObject *>::iterator i;

	if(init_cullings)
		for(int c=0; c<cullings_count; c++)
			cullings[c]->InitCulling();

	space->Clear();

	FillRenderObjectSpace(space, cullings, cullings_count, false, false);

	vector<tPointLight *>::iterator pi;

	for(pi=point_lights.begin(); pi!=point_lights.end(); pi++)
	{
		if(!(*pi)->GetEnabled())
			continue;

		if(!(*pi)->GetShadowInvalid())
		{
			light_pos = (*pi)->GetPosition();
			light_dist = (*pi)->GetDistance();

			bool cull = true;

			for(int c=0; c<cullings_count; c++)
			{
				if(!cullings[c]->TestSphereCulling(light_pos, light_dist))
				{
					cull = false;
					break;
				}
			}

			if(cull)
				continue;
		}

		space->point_lights.insert(*pi);
	}

	vector<tDirectionalLight *>::iterator di;

	for(di=dir_lights.begin(); di!=dir_lights.end(); di++)
	{
		space->dir_lights.insert(*di);

		if(!(*di)->GetShadowEnabled())
			continue;

		for(i=objects.begin(); i!=objects.end(); i++)
			(*di)->GetShadow()->GetRenderSpace()->objects.insert((*i)); // TODO: move to tDeferredRenderer
	}
}*/


void tWorld::AssignUnsetReflectionProbes(void)
{
	for(vector<tObject *>::iterator i=objects.begin(); i!=objects.end(); i++)
	{
		tMeshObject *mesh_object = dynamic_cast<tMeshObject *>(*i);
		if(!mesh_object)
			continue;

		if(mesh_object->GetCubeMapReflection() // Reflection already set
				|| !mesh_object->GetCubeMapReflectionEnabled()) // or not needed
			continue;

		tReflectionProbe *min = 0;
		float min_v = FLT_MAX;

		for(auto ri=reflection_probes.begin(); ri!=reflection_probes.end(); ri++)
		{
			tReflectionProbe *reflection = *ri;
			float v = (reflection->GetPosition() - mesh_object->GetTransform().GetPosition()).SquaredLen();
			if(v < min_v)
			{
				min_v = v;
				min = reflection;
			}
		}

		if(min)
			mesh_object->SetCubeMapReflection(min);
	}
}


void tWorld::Step(float time, int max_sub_steps, float fixed_time_step)
{
	if(time < 0.001)
		time = 0.001;

	physics.dynamics_world->stepSimulation(time, max_sub_steps, fixed_time_step);
	//CProfileManager::dumpAll();

	//for(vector<tParticleSystem *>::iterator psi = particle_systems.begin(); psi != particle_systems.end(); psi++)
	//	(*psi)->Step(time);
}




