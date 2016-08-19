#include "towerengine.h"

#include "rapidxml.hpp"


using namespace std;
using namespace rapidxml;


tMesh::tMesh(const char *file, tMaterialManager *material_manager)
{
	idle_pose = 0;
	vao = 0;
	physics_triangle_mesh = 0;
	bounding_box = tBoundingBox();
	//mat_indices = 0;
	refresh_func = 0;
	refresh_vbos = true;
	refresh_ibos = true;
	outdated_vertices.clear();
	vertex_indices.clear();
	entities.clear();

	vao = new tVAO();
	current_pose = idle_pose = new tMeshPose(this);
	vertex_vbo = new tVBO<float>(3);
	normal_vbo = new tVBO<float>(3);
	tang_vbo = new tVBO<float>(3);
	bitang_vbo = new tVBO<float>(3);
	face_normal_vbo = new tVBO<float>(3);
	uvcoord_vbo = new tVBO<float>(2);

	vao->Bind();
	vertex_vbo->SetAttribute(tFaceShader::vertex_attribute, GL_FLOAT);
	normal_vbo->SetAttribute(tFaceShader::normal_attribute, GL_FLOAT);
	tang_vbo->SetAttribute(tFaceShader::tang_attribute, GL_FLOAT);
	bitang_vbo->SetAttribute(tFaceShader::bitang_attribute, GL_FLOAT);
	uvcoord_vbo->SetAttribute(tFaceShader::uvcoord_attribute, GL_FLOAT);
	tVAO::UnBind();

	outdated_vertices.clear();
	data_count = 0;
	refresh_func = 0;
	refresh_vbos = true;
	refresh_ibos = true;
	idle_material = new tDefaultMaterial();
	material_ibos[idle_material] = new tMaterialIBO(this);
	//VAO::UnBind();

	SetAnimationLoop(1);
	ResetAnimationFinished();

	if(file)
		LoadFromFile(file);
}

tMesh::~tMesh(void)
{
	//printf("  delete mesh\n");
	delete idle_pose;
	for(map<string, tMeshPose *>::iterator i=custom_pose.begin(); i!=custom_pose.end(); i++)
		delete i->second;

	//printf("  deleted poses\n");

	for(vector<tAnimation *>::iterator i=animations.begin(); i!=animations.end(); i++)
		delete *i;

	//printf("  deleted animations\n");

	for(vector<tTriangle *>::iterator i=triangles.begin(); i!=triangles.end(); i++)
		delete *i;

	//printf("  deleted triangles\n");
	
	for(set<tMaterial *>::iterator mi=own_materials.begin(); mi!=own_materials.end(); mi++)
		delete *mi;

	//printf("  deleted own materials\n");

	for(map<tMaterial *, tMaterialIBO *>::iterator i=material_ibos.begin(); i!=material_ibos.end(); i++)
		delete i->second;

	//printf("  deleted material_ibos\n");

	for(vector<tVertex *>::iterator i=vertices.begin(); i!=vertices.end(); i++)
		delete *i;

	//printf("  deleted vertices\n");

	for(vector<tEntity *>::iterator i=entities.begin(); i!=entities.end(); i++)
		delete *i;

	//printf("  deleted stuff\n");

	DeleteVBOData();

	//printf("  deleted vbo data\n");

	delete physics_triangle_mesh;
	delete idle_material;
}


void tMesh::DeleteVBOData(void)
{
	if(vao)
	{
		delete vao;
		vao = 0;
		delete vertex_vbo;
		delete normal_vbo;
		delete tang_vbo;
		delete bitang_vbo;
		delete face_normal_vbo;
		delete uvcoord_vbo;
	}
}

tVBO<float> *tMesh::CreateFloatVBO(int components)
{
	return new tVBO<float>(components);
}


tVertex *tMesh::CreateVertex(tVector v)
{
	tVertex *o;
	o = new tVertex(v);
	AddVertex(o);
	refresh_vbos = false;
	return o;
}

tTriangle *tMesh::CreateTriangle(tVertex *v1, tVertex *v2, tVertex *v3, tVector color, tMaterial *material)
{
	refresh_ibos = true;

    tTriangle *t = tTriangle::CreateTriangle(v1, v2, v3, color, material);
    AddTriangle(t);
    return t;
}

/*tTriangle *tMesh::CreateTriangleAuto(tVector v1, tVector v2, tVector v3, tVector color, string material, tVector t1, tVector t2, tVector t3)
{
	tVertex *vert[3];
	vector<tVertex *>::iterator verti;
	int i;
	float v[9] = {v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z};

	vert[0] = vert[1] = vert[2] = 0;
	for(i=0; i<3; i++)
	{
		for(verti=vertices.begin(); verti!=vertices.end(); verti++)
			if((*verti)->pos.x == v[i*3+0] && (*verti)->pos.y == v[i*3+1] && (*verti)->pos.z == v[i*3+2])
			{
				vert[i] = *verti;
				break;
			}
		if(!vert[i])
			vert[i] = CreateVertex(Vec(v[i*3+0], v[i*3+1], v[i*3+2]));
	}

	refresh_ibos = true;

	return CreateTriangle(vert[0], vert[1], vert[2],
               color,			     // color
               material,                    // material
               t1,			    // t1
               t2,			    // t2
               t3);			    // t3
}*/

tEntity *tMesh::CreateEntity(string name, string group)
{
	tEntity *e = new tEntity();
	e->name = name;
	e->group = group;
	entities.push_back(e);
	return e;
}

tMaterial *tMesh::GetMaterialByName(string name)
{
	map<string, tMaterial *>::iterator i = materials.find(name);

	if(i == materials.end())
		return 0;
	else
		return i->second;
}

tVertex *tMesh::GetVertexByID(int id)
{
	map<int, tVertex *>::iterator i = vertex_indices.find(id);
	if(i != vertex_indices.end())
		return i->second;

	return 0;

	/*
	vector<CVertex *>::iterator v;
	int i = 0;

	for(v=vertices.begin(); v!=vertices.end(); v++)
		if((*v)->id == id)
		{
			if(i>=w)
				return *v;
			else i++;
		}

	return 0;*/
}

void tMesh::SetVertexId(tVertex *v, int id)
{
	v->id = id;
	vertex_indices[id] = v;
}

void tMesh::AddVertex(tVertex *v)
{
	vector<tVertex *>::iterator i;

	for(i=vertices.begin(); i!=vertices.end(); i++)
		if(*i == v)
			return;
	vertices.push_back(v);

	refresh_vbos = true;
}

void tMesh::AddTriangle(tTriangle *t)
{
	vector<tTriangle *>::iterator i;

	for(i=triangles.begin(); i!=triangles.end(); i++)
		if(*i == t)
			return;
	triangles.push_back(t);
	refresh_vbos = true;
	refresh_ibos = true;
}

void tMesh::AddMaterial(string name, tMaterial *m)
{
	materials[name] = m;

	if(material_ibos.find(m) == material_ibos.end())
		material_ibos[m] = new tMaterialIBO(this);
}

void tMesh::AddCustomPose(string name, tMeshPose *p)
{
	if(custom_pose.find(name) != custom_pose.end())
		return;

	custom_pose.insert(pair<string, tMeshPose *>(name, p));
	//refresh_vbo = true;
}

void tMesh::AddAnimation(tAnimation *a)
{
	vector<tAnimation *>::iterator i;

	for(i=animations.begin(); i!=animations.end(); i++)
		if(*i == a)
			return;
	animations.push_back(a);
	//refresh_vbo = true;
}

void tMesh::RemoveVertex(tVertex *v)
{
	vector<tVertex *>::iterator i;

	for(i=vertices.begin(); i!=vertices.end(); i++)
		if(*i == v)
		{
			vertices.erase(i);
			break;
		}
	refresh_vbos = true;
	refresh_ibos = true;
}

void tMesh::RemoveTriangle(tTriangle *t)
{
	vector<tTriangle *>::iterator i;

	for(i=triangles.begin(); i!=triangles.end(); i++)
		if(*i == t)
		{
			triangles.erase(i);
			break;
		}
	refresh_ibos = true;
	//refresh_vbo = true;
}

void tMesh::RemoveMaterial(string name)
{
	map<string, tMaterial *>::iterator i = materials.find(name);

	if(i != materials.end())
	{
		map<tMaterial *, tMaterialIBO *>::iterator ii = material_ibos.find(i->second);
		if(ii != material_ibos.end())
		{
			delete ii->second;
			material_ibos.erase(ii);
		}

		materials.erase(i);
	}
}

void tMesh::RemoveCustomPose(string name)
{
	map<string, tMeshPose *>::iterator i;

	i = custom_pose.find(name);

	if(i != custom_pose.end())
		custom_pose.erase(i);
}

void tMesh::RemoveAnimation(tAnimation *a)
{
	vector<tAnimation *>::iterator i;

	for(i=animations.begin(); i!=animations.end(); i++)
		if(*i == a)
		{
			animations.erase(i);
			break;
		}
	//refresh_vbo = true;
}

void tMesh::RemoveEntity(tEntity *e)
{
	vector<tEntity *>::iterator i;

	for(i=entities.begin(); i!=entities.end(); i++)
		if(*i == e)
		{
			entities.erase(i);
			break;
		}
}

void tMesh::SetIDs(void)
{
	int i;
	vector<tVertex *>::iterator v;
	vector<tAnimation *>::iterator a;
	tKeyFrame *f;
	map<string, tMeshPose *>::iterator p;

	for(v=vertices.begin(), i=0; v!=vertices.end(); v++, i++)
		SetVertexId((*v), i);

	for(a=animations.begin(); a!=animations.end(); a++)
		for(f=(*a)->key_first; f; f=f->chain_next, i++)
			f->id = i;

	for(p=custom_pose.begin(), i=0; p!=custom_pose.end(); p++)
		p->second->id = i;
}

/*void tMesh::SetTriangleMaterials(void)
{
    vector<tTriangle *>::iterator t;

    for(t=triangles.begin(); t!=triangles.end(); t++)
        (*t)->mat = GetMaterialByName((*t)->m_name);

	//refresh_vbo = true;
}*/

void tMesh::AssignVertexArrayPositions(void)
{
	vector<tVertex *>::iterator i;
	int c;

	for(c=0, i=vertices.begin(); i!=vertices.end(); c++, i++)
		(*i)->index = c;

	refresh_vbos = true;
}

void tMesh::GenerateBoundingBox(void)
{
	vector<tVertex *>::iterator i;
	bounding_box = tBoundingBox();

	for(i=vertices.begin(); i!=vertices.end(); i++)
		bounding_box.AddPoint((*i)->pos);
}

btTriangleMesh *tMesh::GeneratePhysicsMesh(void)
{
	vector<tTriangle *>::iterator i;

	//if(physics_triangle_mesh)
	delete physics_triangle_mesh;

	if(triangles.empty())
		physics_triangle_mesh = 0;
	else
	{
		physics_triangle_mesh = new btTriangleMesh();

		for(i=triangles.begin(); i!=triangles.end(); i++)
			physics_triangle_mesh->addTriangle(BtVec((*i)->v[0]->pos), BtVec((*i)->v[1]->pos), BtVec((*i)->v[2]->pos));
	}

	return physics_triangle_mesh;
}


void tMesh::RefreshAllVBOs(void)
{
	vector<tTriangle *>::iterator i;
	//vector<tMaterial *>::iterator m;
	vector<tVertex *>::iterator v;
	tVertex *vt;
	int d;

	AssignVertexArrayPositions();

	data_count = GetVertexCount();
	vertex_vbo->SetSize(data_count);
	normal_vbo->SetSize(data_count);
	tang_vbo->SetSize(data_count);
	bitang_vbo->SetSize(data_count);
	face_normal_vbo->SetSize(data_count);
	uvcoord_vbo->SetSize(data_count);

	float *vertex_data = vertex_vbo->GetData();
	float *normal_data = normal_vbo->GetData();
	float *tang_data = tang_vbo->GetData();
	float *bitang_data = bitang_vbo->GetData();
	float *face_normal_data = face_normal_vbo->GetData();
	float *uvcoord_data = uvcoord_vbo->GetData();

	for(v=vertices.begin(); v!=vertices.end(); v++)
	{
		vt = *v;
		d = vt->index * 3;
		memcpy(vertex_data + d, vt->pos.v, 3 * sizeof(float));
		memcpy(normal_data + d, vt->normal.v, 3 * sizeof(float));
		memcpy(tang_data + d, vt->tang.v, 3 * sizeof(float));
		memcpy(bitang_data + d, vt->bitang.v, 3 * sizeof(float));
		memcpy(face_normal_data + d, vt->normal.v, 3 * sizeof(float));
		memcpy(uvcoord_data + vt->index*2, vt->uv.v, 2 * sizeof(float));

	}

	vertex_vbo->AssignData();
	normal_vbo->AssignData();
	tang_vbo->AssignData();
	bitang_vbo->AssignData();
	face_normal_vbo->AssignData();
	uvcoord_vbo->AssignData();

	refresh_vbos = false;
}

void tMesh::RefreshIBOs(void)
{
	vector<tTriangle *>::iterator i;
	map<tMaterial *, tMaterialIBO *>::iterator m;
	tTriangle *t;
	tMaterial *mt;
	tMaterialIBO *mibo;
	int c;
	int j;

	SortMaterials();

	for(m=material_ibos.begin(); m!=material_ibos.end(); m++)
		m->second->triangle_count = m->second->triangle_i = 0;

	for(i=triangles.begin(); i!=triangles.end(); i++)
	{
		mt = (*i)->mat;
		if(!mt)
			mt = GetIdleMaterial();
		material_ibos[mt]->triangle_count++;
	}

	for(m=material_ibos.begin(); m!=material_ibos.end(); m++)
	{
		mibo = m->second;
		mibo->ibo->SetSize(mibo->triangle_count * 3);
		mibo->ibo_data = mibo->ibo->GetData();
	}

	for(i=triangles.begin(), c=0; i!=triangles.end(); i++, c++)
	{
		t = *i;
		mt = (*i)->mat;
		if(!mt)
			mt = GetIdleMaterial();
		mibo = material_ibos[mt];

		for(j=0; j<3; j++)
			mibo->ibo_data[mibo->triangle_i*3 + j] = t->v[j]->index;
		mibo->triangle_i++;
	}

	for(m=material_ibos.begin(); m!=material_ibos.end(); m++)
		m->second->ibo->AssignData();

	refresh_ibos = false;
}


void tMesh::DepthPrePass(tRenderer *renderer, map<tMaterial *, tMaterial *> *replace_materials)
{
	if(refresh_vbos)
		RefreshAllVBOs();

	vao->Bind();

	if(refresh_ibos)
		RefreshIBOs();


	for(map<tMaterial *, tMaterialIBO *>::iterator i=material_ibos.begin(); i!=material_ibos.end(); i++)
	{
		tMaterial *mat = i->first;
		if(replace_materials)
		{
			map<tMaterial *, tMaterial *>::iterator ri = replace_materials->find(mat);
			if(ri != replace_materials->end())
				mat = ri->second;
		}

		if(mat->InitDepthPrePass(renderer))
			i->second->ibo->Draw(GL_TRIANGLES);
	}
}


void tMesh::GeometryPass(tRenderer *renderer, map<tMaterial *, tMaterial *> *replace_materials)
{
	if(refresh_vbos)
		RefreshAllVBOs();

	vao->Bind();

	if(refresh_ibos)
		RefreshIBOs();

	for(map<tMaterial *, tMaterialIBO *>::iterator i=material_ibos.begin(); i!=material_ibos.end(); i++)
	{
		tMaterial *mat = i->first;
		if(replace_materials)
		{
			map<tMaterial *, tMaterial *>::iterator ri = replace_materials->find(mat);
			if(ri != replace_materials->end())
				mat = ri->second;
		}

		if(mat->InitGeometryPass(renderer))
			i->second->ibo->Draw(GL_TRIANGLES);
	}
}

void tMesh::ForwardPass(tRenderer *renderer, float *transform, map<tMaterial *, tMaterial *> *replace_materials)
{
	bool vao_bound = false;

	for(map<tMaterial *, tMaterialIBO *>::iterator i=material_ibos.begin(); i!=material_ibos.end(); i++)
	{
		tMaterial *mat = i->first;
		if(replace_materials)
		{
			map<tMaterial *, tMaterial *>::iterator ri = replace_materials->find(mat);
			if(ri != replace_materials->end())
				mat = ri->second;
		}

		if(mat->InitForwardPass(renderer, transform))
		{
			if(!vao_bound)
			{
				vao_bound = true;
				vao->Bind();
				//current_pose->vbo->SetAttribute(tFaceShader::vertex_attribute, GL_FLOAT);
			}

			i->second->ibo->Draw(GL_TRIANGLES);
		}
	}
}


void tMesh::RefractionPass(tRenderer *renderer, float *transform, map<tMaterial *, tMaterial *> *replace_materials)
{
	bool vao_bound = false;

	for(map<tMaterial *, tMaterialIBO *>::iterator i=material_ibos.begin(); i!=material_ibos.end(); i++)
	{
		tMaterial *mat = i->first;
		if(replace_materials)
		{
			map<tMaterial *, tMaterial *>::iterator ri = replace_materials->find(mat);
			if(ri != replace_materials->end())
				mat = ri->second;
		}

		if(mat->InitRefractionPass(renderer, transform))
		{
			if(!vao_bound)
			{
				vao_bound = true;
				vao->Bind();
				//current_pose->vbo->SetAttribute(tFaceShader::vertex_attribute, GL_FLOAT);
			}

			i->second->ibo->Draw(GL_TRIANGLES);
		}
	}
}

bool tMesh::GetCubeMapReflectionEnabled(void)
{
	for(map<string, tMaterial *>::iterator i=materials.begin(); i!=materials.end(); i++)
		if(i->second->GetCubeMapReflectionEnabled())
			return true;

	return false;
}

bool tMesh::LoadFromFile(const char *file, tMaterialManager *material_manager)
{
	bool r;
	string path;
	//struct stat s;
	
	//if(stat(file, &s) != 0 && errno == ENOENT)
	//    return 0;
	
	path = PathOfFile(string(file));

	char *data = ReadFile(file);

	xml_document<> *doc = new xml_document<>();
	doc->parse<0>(data);
	r = LoadFromXML(doc, path, material_manager);

	delete doc;
	delete [] data;

	refresh_vbos = true;
	refresh_ibos = true;

	return r;
}

bool tMesh::LoadFromData(char *data, string path, tMaterialManager *material_manager)
{
	bool r;

	xml_document<> *doc = new xml_document<>();
	doc->parse<0>(data);

	r = LoadFromXML(doc, path, material_manager);

	refresh_vbos = true;
	refresh_ibos = true;

	return r;
}

int TEMVersionFromString(const char *s)
{
	if(strcmp(s, TEM_VERSION_0_4_STRING) == 0)
		return TEM_VERSION_0_4;
	if(strcmp(s, TEM_VERSION_0_3_STRING) == 0)
		return TEM_VERSION_0_3;
	if(strcmp(s, TEM_VERSION_0_2_STRING) == 0)
		return TEM_VERSION_0_2;
	if(strcmp(s, TEM_VERSION_0_1_STRING) == 0)
		return TEM_VERSION_0_1;
	return -1;
}

bool tMesh::LoadFromXML(xml_document<> *doc, string path, tMaterialManager *material_manager)
{
	xml_node<> *cur;
	xml_attribute<> *attr;

	if(!doc)
		return 0;

	cur = doc->first_node();
	if(!cur)
		return 0;

	if(strcmp(cur->name(), "tmesh") != 0)
		return 0;

	attr = cur->first_attribute("version");
	if(!attr)
	{
		printf("WARNING: File does not contain version info. Assuming it is %s.\n", TEM_CURRENT_VERSION_STRING);
		file_version = TEM_CURRENT_VERSION;
	}
	else
	{
		file_version = TEMVersionFromString(attr->value());
		if(file_version == -1)
		{
			printf("WARNING: Version of mesh file could not be recognized. Assuming it is %s.\n", TEM_CURRENT_VERSION_STRING);
			file_version = TEM_CURRENT_VERSION;
		}
		else if(file_version != TEM_CURRENT_VERSION)
		{
			printf("WARNING: Version of mesh file of is outdated. Only %s is supported\n", TEM_CURRENT_VERSION_STRING);
			return false;
		}
	}

	cur = cur->first_node();

	//printf("Path xml \"%s\"\n", path);

	int current_vertex_id = 0;

	while(cur)
	{
		char *name = cur->name();

		if(strcmp(name, "vertex") == 0)
			ParseVertexNode(cur);
		else if(strcmp(name, "material") == 0)
			ParseMaterialNode(cur, path);
		else if(strcmp(name, "triangle") == 0)
			ParseTriangleNode(cur, material_manager);
		else if(strcmp(name, "mesh_data") == 0)
			ParseMeshDataNode(cur, material_manager, current_vertex_id);
		else if(strcmp(name, "pose") == 0 || strcmp(name, "position") == 0)
			ParsePoseNode(cur);
		else if(strcmp(name, "animation") == 0)
			ParseAnimationNode(cur);
		else if(strcmp(name, "entity") == 0)
			ParseEntityNode(cur);

		cur = cur->next_sibling();
	}


	idle_pose->CopyFromVertices();

	GenerateBoundingBox();
	GeneratePhysicsMesh();

	refresh_vbos = true;
	refresh_ibos = true;

	return 1;
}

tVertex *tMesh::ParseVertexNode(xml_node<> *cur)
{
	int id;
	tVector p;
	tVector normal;
	tVector2 uv;
	tVertex *r;

	// TODO: check if attributes really are found

	id = atoi(cur->first_attribute("id")->value());
	p.x = (float)atof(cur->first_attribute("x")->value());
	p.y = (float)atof(cur->first_attribute("y")->value());
	p.z = (float)atof(cur->first_attribute("z")->value());

	uv.x = (float)atof(cur->first_attribute("u")->value());
	uv.y = (float)atof(cur->first_attribute("v")->value());
	normal.x = (float)atof(cur->first_attribute("nx")->value());
	normal.y = (float)atof(cur->first_attribute("ny")->value());
	normal.z = (float)atof(cur->first_attribute("nz")->value());

	r = new tVertex(p);
	AddVertex(r);
	SetVertexId(r, id);
	//r->uv_set = file_version >= TEM_VERSION_0_2;
	r->uv = uv;
	r->normal = normal;
	return r;
}



tMaterial *tMesh::ParseXMLMaterialNode(xml_node<> *cur, string &name, string path)
{
	xml_attribute<> *attr;

	if((attr = cur->first_attribute("type")))
	{
		if(strcmp(attr->value(), "simple_forward") == 0)
			return ParseXMLSimpleForwardMaterialNode(cur, name, path);
		else if(strcmp(attr->value(), "refraction") == 0)
			return ParseXMLRefractionMaterialNode(cur, name, path);
		else
			return ParseXMLDefaultMaterialNode(cur, name, path);
	}
	else
		return ParseXMLDefaultMaterialNode(cur, name, path);

}



void tMesh::LoadTextureFromXMLNodeData(xml_node<> *node, tDefaultMaterial *material, tDefaultMaterial::TextureType texture_type)
{
	unsigned char *data;
	size_t size;

	Base64Decode(node->value(), &data, &size);

	xml_attribute<> *attr = node->first_attribute("image-extension");
	char *extension;
	if(attr)
		extension = attr->value();
	else
		extension = 0;

	material->LoadTexture(texture_type, extension, data, size);
	delete [] data;
}

tDefaultMaterial *tMesh::ParseXMLDefaultMaterialNode(xml_node<> *cur, string &name, string path)
{
	xml_node<> *child;
	xml_attribute<> *attr;

	if(!(attr = cur->first_attribute("name")))
		return 0;

	name = string(attr->value());

	tDefaultMaterial *mat = new tDefaultMaterial();



	child = cur->first_node();
	while(child)
	{
		char *name_temp = child->name();

		if(strcmp(name_temp, "base_color") == 0)
		{
			if((attr = child->first_attribute("file")))
				mat->LoadTexture(tDefaultMaterial::BASE_COLOR, path + string(attr->value()));
			else if(child->value_size() > 0)
				LoadTextureFromXMLNodeData(child, mat, tDefaultMaterial::BASE_COLOR);

			tVector color = Vec(0.0, 0.0, 0.0);
			if((attr = child->first_attribute("r")))
				color.r = (float)atof(attr->value());
			if((attr = child->first_attribute("g")))
				color.g = (float)atof(attr->value());
			if((attr = child->first_attribute("b")))
				color.b = (float)atof(attr->value());
			mat->SetBaseColor(color);
		}
		else if(strcmp(name_temp, "metal_rough_reflect") == 0)
		{
			if((attr = child->first_attribute("file")))
				mat->LoadTexture(tDefaultMaterial::METAL_ROUGH_REFLECT, path + string(attr->value()));
			else if(child->value_size() > 0)
				LoadTextureFromXMLNodeData(child, mat, tDefaultMaterial::METAL_ROUGH_REFLECT);

			if((attr = child->first_attribute("metallic")))
				mat->SetMetallic((float)atof(attr->value()));
			if((attr = child->first_attribute("roughness")))
				mat->SetRoughness((float)atof(attr->value()));
			if((attr = child->first_attribute("reflectance")))
				mat->SetReflectance((float)atof(attr->value()));
		}
		else if(strcmp(name_temp, "normal") == 0)
		{
			if((attr = child->first_attribute("file")))
				mat->LoadTexture(tDefaultMaterial::NORMAL, path + string(attr->value()));
			else if(child->value_size() > 0)
				LoadTextureFromXMLNodeData(child, mat, tDefaultMaterial::NORMAL);
		}
		else if(strcmp(name_temp, "emission") == 0)
		{
			if((attr = child->first_attribute("file")))
				mat->LoadTexture(tDefaultMaterial::EMISSION, path + string(attr->value()));
			else if(child->value_size() > 0)
				LoadTextureFromXMLNodeData(child, mat, tDefaultMaterial::EMISSION);

			tVector color = Vec(0.0, 0.0, 0.0);
			if((attr = child->first_attribute("r")))
				color.r = (float)atof(attr->value());
			if((attr = child->first_attribute("g")))
				color.g = (float)atof(attr->value());
			if((attr = child->first_attribute("b")))
				color.b = (float)atof(attr->value());
			mat->SetEmission(color);
		}
		else if(strcmp(name_temp, "bump") == 0)
		{
			if((attr = child->first_attribute("file")))
				mat->LoadTexture(tDefaultMaterial::BUMP, path + string(attr->value()));
			else if(child->value_size() > 0)
				LoadTextureFromXMLNodeData(child, mat, tDefaultMaterial::BUMP);

			if((attr = child->first_attribute("depth")))
				mat->SetBumpDepth((float)atof(attr->value()));
		}
		else if(strcmp(name_temp, "shadow") == 0)
		{
			if((attr = child->first_attribute("cast")))
				mat->SetShadowCast(atoi(attr->value()) != 0);
		}

		child = child->next_sibling();
	}

	mat->UpdateUniformBuffer();

	return mat;
}


#define TEXTURE_DISABLED 0
#define TEXTURE_FILE 1
#define TEXTURE_DATA 2

tSimpleForwardMaterial *tMesh::ParseXMLSimpleForwardMaterialNode(xml_node<> *cur, string &name, string path)
{
	xml_node<> *child;

	int tex_mode = TEXTURE_DISABLED;
	string tex_file;
	tVector color = Vec(1.0, 1.0, 1.0);
	float alpha = 1.0;

	tSimpleForwardMaterial::BlendMode blend_mode = tSimpleForwardMaterial::ALPHA;

	unsigned char *tex_data = 0;
	size_t tex_size = 0;
	char *tex_ext = 0;
	char *base64_temp = 0;


	xml_attribute<> *attr;

	if(!(attr = cur->first_attribute("name")))
		return 0;

	name = string(attr->value());

	if((attr = cur->first_attribute("blend_mode")))
	{
		if(strcmp(attr->value(), "alpha") == 0)
			blend_mode = tSimpleForwardMaterial::ALPHA;
		else if(strcmp(attr->value(), "add") == 0)
			blend_mode = tSimpleForwardMaterial::ADD;
		else if(strcmp(attr->value(), "multiply") == 0)
			blend_mode = tSimpleForwardMaterial::MULTIPLY;
	}

	child = cur->first_node();
	while(child)
	{
		char *name_temp = child->name();

		if(strcmp(name_temp, "tex") == 0)
		{
			if((attr = child->first_attribute("file")))
			{
				tex_file = string(attr->value());
				tex_mode = TEXTURE_FILE;
			}
			else if(child->value_size() > 0)
			{
				base64_temp = child->value();

				Base64Decode(base64_temp, &tex_data, &tex_size);

				attr = child->first_attribute("image-extension");
				if(attr)
					tex_ext = attr->value();
				else
					tex_ext = 0;
				tex_mode = TEXTURE_DATA;
			}
		}
		else if(strcmp(name_temp, "color") == 0)
		{
			if((attr = child->first_attribute("r")))
				color.r = (float)atof(attr->value());
			if((attr = child->first_attribute("g")))
				color.g = (float)atof(attr->value());
			if((attr = child->first_attribute("b")))
				color.b = (float)atof(attr->value());
			if((attr = child->first_attribute("a")))
				alpha = (float)atof(attr->value());
		}

		child = child->next_sibling();
	}


	tSimpleForwardMaterial *r = new tSimpleForwardMaterial();

	r->SetColor(color, alpha);
	r->SetBlendMode(blend_mode);

	if(tex_mode == TEXTURE_FILE)
		r->LoadTexture(path + tex_file);
	else if(tex_mode == TEXTURE_DATA)
		r->LoadTexture(tex_ext, tex_data, tex_size);

	return r;
}


tRefractionMaterial *tMesh::ParseXMLRefractionMaterialNode(xml_node<> *cur, string &name, string path)
{
	xml_node<> *child;

	int color_tex_mode = TEXTURE_DISABLED;
	string color_tex_file;
	int normal_tex_mode = TEXTURE_DISABLED;
	string normal_tex_file;
	tVector color = Vec(1.0, 1.0, 1.0);

	tVector edge_color = Vec(0.0, 0.0, 0.0);
	float edge_alpha = 0.0;

	unsigned char *color_tex_data = 0;
	size_t color_tex_size = 0;
	char *color_tex_ext = 0;

	unsigned char *normal_tex_data = 0;
	size_t normal_tex_size = 0;
	char *normal_tex_ext = 0;

	char *base64_temp = 0;


	xml_attribute<> *attr;

	if(!(attr = cur->first_attribute("name")))
		return 0;

	name = string(attr->value());

	child = cur->first_node();
	while(child)
	{
		char *name_temp = child->name();

		if(strcmp(name_temp, "normal_tex") == 0)
		{
			if((attr = child->first_attribute("file")))
			{
				normal_tex_file = string(attr->value());
				normal_tex_mode = TEXTURE_FILE;
			}
			else if(child->value_size() > 0)
			{
				base64_temp = child->value();

				Base64Decode(base64_temp, &normal_tex_data, &normal_tex_size);

				attr = child->first_attribute("image-extension");
				if(attr)
					normal_tex_ext = attr->value();
				else
					normal_tex_ext = 0;
				normal_tex_mode = TEXTURE_DATA;
			}
		}
		else if(strcmp(name_temp, "color") == 0)
		{
			if((attr = child->first_attribute("r")))
				color.r = (float)atof(attr->value());
			if((attr = child->first_attribute("g")))
				color.g = (float)atof(attr->value());
			if((attr = child->first_attribute("b")))
				color.b = (float)atof(attr->value());


			if((attr = child->first_attribute("file")))
			{
				color_tex_file = string(attr->value());
				color_tex_mode = TEXTURE_FILE;
			}
			else if(child->value_size() > 0)
			{
				base64_temp = child->value();

				Base64Decode(base64_temp, &color_tex_data, &color_tex_size);

				attr = child->first_attribute("image-extension");
				if(attr)
					color_tex_ext = attr->value();
				else
					color_tex_ext = 0;
				color_tex_mode = TEXTURE_DATA;
			}
		}
		else if(strcmp(name_temp, "edge_color") == 0)
		{
			if((attr = child->first_attribute("r")))
				edge_color.r = (float)atof(attr->value());
			if((attr = child->first_attribute("g")))
				edge_color.g = (float)atof(attr->value());
			if((attr = child->first_attribute("b")))
				edge_color.b = (float)atof(attr->value());
			if((attr = child->first_attribute("a")))
				edge_alpha = (float)atof(attr->value());
		}

		child = child->next_sibling();
	}


	tRefractionMaterial *r = new tRefractionMaterial();

	r->SetColor(color);
	r->SetEdgeColor(edge_color, edge_alpha);

	if(color_tex_mode == TEXTURE_FILE)
		r->LoadColorTexture(path + color_tex_file);
	else if(color_tex_mode == TEXTURE_DATA)
		r->LoadColorTexture(color_tex_ext, color_tex_data, color_tex_size);

	if(normal_tex_mode == TEXTURE_FILE)
		r->LoadNormalTexture(path + normal_tex_file);
	else if(normal_tex_mode == TEXTURE_DATA)
		r->LoadNormalTexture(normal_tex_ext, normal_tex_data, normal_tex_size);

	return r;
}


tMaterial *tMesh::ParseMaterialNode(xml_node<> *cur, string path)
{
	string name;
	tMaterial *mat = ParseXMLMaterialNode(cur, name, path);

	if(mat)
	{
		AddMaterial(name, mat);
		own_materials.insert(mat);
	}

	return mat;
}

void tMesh::ParseMeshDataNode(xml_node<> *cur, tMaterialManager *material_manager, int &current_vertex_id)
{
	char *data = cur->value();
	size_t data_size = cur->value_size();

	//enum DataType { VERTEX, TRIANGLE };

	const int max_values_count = 9;

	//DataType current_data_type;

	char *values[max_values_count];

	int values_count = 0;
	size_t value_start = 0;
	for(size_t i=0; i<data_size; i++)
	{
		char &c = data[i];
		if(c == ' ')
		{
			values[values_count] = data + value_start;
			c = 0;
			values_count++;
			value_start = i+1;
		}

		if(values_count >= max_values_count)
		{
			while(true)
			{
				if(i >= data_size)
					break;
				if(data[i] == '\n' || data[i] == '\r')
					break;
			}
		}

		if(c == '\n' || c == '\r' || i == data_size-1)
		{
			if(values_count < max_values_count)
			{
				values[values_count] = data + value_start;
				if(c == '\n' || c == '\r')
					c = 0;
				values_count++;
			}

			/*printf("%d values: ", values_count);
			for(int j=0; j<values_count; j++)
			{
				printf("\"%s\" ", values[j]);
			}
			printf("\n");*/


			// process values
			if(values_count > 0)
			{
				if(strcmp(values[0], "v") == 0) // vertex
				{
					if(values_count >= 9)
					{
						tVector p;
						tVector normal;
						tVector2 uv;
						tVertex *r;

						int id = current_vertex_id;
						current_vertex_id++;
						p.x = (float)atof(values[1]);
						p.y = (float)atof(values[2]);
						p.z = (float)atof(values[3]);

						uv.x = (float)atof(values[4]);
						uv.y = (float)atof(values[5]);
						normal.x = (float)atof(values[6]);
						normal.y = (float)atof(values[7]);
						normal.z = (float)atof(values[8]);

						r = new tVertex(p);
						AddVertex(r);
						SetVertexId(r, id);
						//r->uv_set = file_version >= TEM_VERSION_0_2;
						r->uv = uv;
						r->normal = normal;
					}
				}
				else if(strcmp(values[0], "t") == 0)
				{
					if(values_count >= 5)
					{
						tMaterial *mat = 0;
						string mat_name = string(values[4]);
						map<string, tMaterial *>::iterator mi = materials.find(mat_name);
						if(mi == materials.end())
						{
							if(material_manager)
							{
								mat = material_manager->GetMaterial(mat_name);
								if(mat)
									AddMaterial(mat_name, mat);
							}
						}
						else
						{
							mat = mi->second;
						}

						if(!mat)
							mat = GetIdleMaterial();

						CreateTriangle(GetVertexByID(atoi(values[1])), GetVertexByID(atoi(values[2])), GetVertexByID(atoi(values[3])), Vec(1.0, 1.0, 1.0), mat);
					}
				}
			}



			// reset zero terminators
			for(int j=0; j<values_count; j++)
			{
				size_t len = strlen(values[j]);
				if(values[j] + len >= data + data_size)
					break;

				values[j][len] = j == values_count-1 ? '\n' : ' ';
			}

			values_count = 0;
			value_start = i+1;
		}
	}
}


tTriangle *tMesh::ParseTriangleNode(xml_node<> *cur, tMaterialManager *material_manager)
{
	string mat_name;
	int v[3];
	int i;
	tVertex *vt[3];
	tTriangle *r;
	xml_node<> *child;
	xml_attribute<> *attr;


	if((attr = cur->first_attribute("mat")))
		mat_name = string(attr->value());

	child = cur->first_node("vertex");
	for(i=0; i<3 && child; child=child->next_sibling("vertex"))
	{
		attr = child->first_attribute("id");
		if(!attr)
			continue;

		v[i] = atoi(attr->value());
		vt[i] = GetVertexByID(v[i]);

		i++;
	}

	if(i<2) // less than 3 vertices
		return 0;

	tMaterial *mat = 0;
	map<string, tMaterial *>::iterator mi = materials.find(mat_name);
	if(mi == materials.end())
	{
		if(material_manager)
		{
			mat = material_manager->GetMaterial(mat_name);
			if(mat)
				AddMaterial(mat_name, mat);
		}
	}
	else
	{
		mat = mi->second;
	}

	if(!mat)
		mat = GetIdleMaterial();

	r = CreateTriangle(vt[0], vt[1], vt[2], Vec(1.0, 1.0, 1.0), mat);
	return r;
}

tMeshPose *tMesh::ParsePoseNode(xml_node<> *cur)
{
	char *name;
	xml_node<> *child;
	int count;
	int i;
	int *vert;
	tVector *vec;
	tMeshPose *r;

	name = cur->first_attribute("name")->value();

	child = cur->first_node("vertex");
	count = 0;
	while(child)
	{
		count++;
		child = child->next_sibling("vertex");
	}

	vert = new int[count];
	vec = new tVector[count];

	for(child = cur->first_node("vertex"), i=0; child && i<count; child = child->next_sibling("vertex"))
	{
		vert[i] = atoi(child->first_attribute("id")->value());
		vec[i].x = (float)atof(child->first_attribute("x")->value());
		vec[i].y = (float)atof(child->first_attribute("y")->value());
		vec[i].z = (float)atof(child->first_attribute("z")->value());

		i++;
	}

	r = new tMeshPose(this);
	r->CopyFromData(count, vert, vec);
	AddCustomPose(name, r);
	return r;
}

tAnimation *tMesh::ParseAnimationNode(xml_node<> *cur)
{
	char *name;
	float len;
	xml_node<> *child;
	xml_attribute<> *attr;
	tAnimation *r;

	if(!(attr = cur->first_attribute("name")))
		return 0;

	name = attr->value();

	if(!(attr = cur->first_attribute("len")))
		return 0;
	len = (float)atof(attr->value());

	r = new tAnimation(this, name, len, 0);

	for(child = cur->first_node("frame"); child; child = child->next_sibling("frame"))
	{
		ParseKeyFrameNode(child, r);
	}

	return r;
}

tKeyFrame *tMesh::ParseKeyFrameNode(xml_node<> *cur, tAnimation *anim)
{
	float time;
	xml_node<> *child;
	int count;
	int i;
	int *vert;
	tVector *vec;
	tKeyFrame *r;
	xml_attribute<> *attr;

	if(!(attr = cur->first_attribute("time")))
		return 0;
	time = (float)atof(attr->value());

	child = cur->first_node("vertex");
	count = 0;
	while(child)
	{
		count++;
		child = child->next_sibling("vertex");
	}

	vert = new int[count];
	vec = new tVector[count];

	for(child = cur->first_node("vertex"), i=0; child && i<count; child = child->first_node("vertex"))
	{
		vert[i] = atoi(child->first_attribute("id")->value());
		vec[i].x = (float)atof(child->first_attribute("x")->value());
		vec[i].y = (float)atof(child->first_attribute("y")->value());
		vec[i].z = (float)atof(child->first_attribute("z")->value());

		i++;
	}

	r = anim->NewKeyFrameFromData(time, count, vert, vec);
	return r;
}


tEntity *tMesh::ParseEntityNode(xml_node<> *root)
{
	tEntity *e = new tEntity();
	tEntityAttribute *a;
	xml_node<> *cur;
	xml_attribute<> *attr;
	string a_type, a_name;


	if(!(attr = root->first_attribute("name")))
		return 0;

	e->name = string(attr->value());
	if(e->name.length() == 0)
	{
		delete e;
		return 0;
	}

	if((attr = root->first_attribute("group")))
		e->group = string(attr->value());

	for(cur=root->first_node(); cur; cur=cur->next_sibling())
	{
		a_type = string(cur->name());

		a = new tEntityAttribute();

		if(a_type.compare("vec3") == 0)
		{
			a->type = tEntityAttribute::VECTOR;

			tVector v;

			if((attr = cur->first_attribute("x")))
				v.x = (float)atof(attr->value());
			if((attr = cur->first_attribute("y")))
				v.y = (float)atof(attr->value());
			if((attr = cur->first_attribute("z")))
				v.z = (float)atof(attr->value());

			a->vec_v = v;
		}
		else if(a_type.compare("vec2") == 0)
		{
			a->type = tEntityAttribute::VECTOR2;

			tVector2 v;

			if((attr = cur->first_attribute("x")))
				v.x = (float)atof(attr->value());
			if((attr = cur->first_attribute("y")))
				v.y = (float)atof(attr->value());

			a->vec2_v = v;
		}
		else if(a_type.compare("float") == 0)
		{
			a->type = tEntityAttribute::FLOAT;

			float v = 0.0;

			if((attr = cur->first_attribute("v")))
				v = (float)atof(attr->value());

			a->float_v = v;
		}
		else if(a_type.compare("int") == 0)
		{
			a->type = tEntityAttribute::INT;

			int v = 0;

			if((attr = cur->first_attribute("v")))
				v = atoi(attr->value());

			a->int_v = v;
		}
		else if(a_type.compare("string") == 0)
		{
			a->type = tEntityAttribute::STRING;

			string v;

			if((attr = cur->first_attribute("v")))
				v = string(attr->value());

			a->string_v = v;
		}
		else
		{
			delete a;
			continue;
		}

		if(!(attr = cur->first_attribute("name")))
			continue;

		a_name = string(attr->value());

		e->attributes.insert(pair<string, tEntityAttribute *>(a_name, a));
	}

	entities.push_back(e);

	return e;
}



map<string, tEntity *> tMesh::GetEntitiesInGroup(const char *group)
{
	map<string, tEntity *> r;
	vector<tEntity *>::iterator i;

	for(i=entities.begin(); i!=entities.end(); i++)
		if((*i)->group.compare(group) == 0)
			r.insert(pair<string, tEntity *>((*i)->name, *i));

	return r;
}


tMeshPose *tMesh::GetCustomPoseByName(string name)
{
	try
	{
		return custom_pose.at(name);
	}
	catch(...) {}

	return 0;
}

tMeshPose *tMesh::CreateCustomPose(string name)
{
	if(GetCustomPoseByName(name))
		return 0;

	tMeshPose *p = new tMeshPose(this);
	p->CopyFromVertices();
	custom_pose.insert(pair<string, tMeshPose *>(name, p));

	return p;
}

void tMesh::ChangePose(string name, string idle)
{
	tMeshPose *p;

	if(name == idle)
		current_pose = idle_pose;
	else if((p = GetCustomPoseByName(name)) != 0)
		current_pose = p;
	else
		return;

	animation_mode = false;

	//current_pose->ApplyPose();
}

void tMesh::ChangePose(tMeshPose *pos)
{
	if(pos == current_pose)
		return;

	current_pose = pos;

	animation_mode = false;

	//if(current_pose)
	//	current_pose->ApplyPose();
}

tMeshPose *tMesh::GetCurrentPose(void)
{
	return current_pose;
}

void tMesh::CopyPoseFromVertices(void)
{
	if(current_pose)
		current_pose->CopyFromVertices();
}


string tMesh::GetCurrentPoseName(string idle) // deprecated
{
	string r;

	if(!current_pose)
		return 0;

	if(current_pose == idle_pose)
		return idle;
	else
	{
		map<string, tMeshPose *>::iterator i;
		for(i=custom_pose.begin(); i!=custom_pose.end(); i++)
		{
			if(i->second == current_pose)
				return i->first;
		}
	}

	return string();
}



tAnimation *tMesh::CreateAnimation(const char *name, float len)
{
	if(GetAnimationByName(name))
		return 0;

	return new tAnimation(this, name, len);
}

void tMesh::ChangeAnimation(tAnimation *a)
{
	if(a->mesh != this)
		return;

	ResetAnimationFinished();
	current_animation = a;
	current_animation->SetTime(0.0);
	animation_mode = true;
	//RefreshAllVBOs();
}

void tMesh::ChangeAnimation(const char *name)
{
	tAnimation *a;

	a = GetAnimationByName(name);

	if(!a)
		return;

	ChangeAnimation(a);
}

tAnimation *tMesh::GetAnimationByName(const char *name)
{
	vector<tAnimation *>::iterator a;

	for(a=animations.begin(); a!=animations.end(); a++)
		if(strcmp(name, (*a)->GetName()) == 0)
			return *a;
	return 0;

}

tAnimation *tMesh::GetCurrentAnimation(void)
{
	return current_animation;
}

char *tMesh::GetCurrentAnimationName(void)
{
	char *r;
	char *n;

	if(!current_animation)
		return 0;

	n = current_animation->GetName();

	r = new char[strlen(n) + 1];
	strcpy(r, n);
	return r;
}

void tMesh::PlayAnimation(float t)
{
	if(!current_animation)
		return;
	anim_finished = current_animation->Play(t, loop_anim);
	current_animation->ApplyCurrentFrame();
}

void tMesh::ApplyAnimation(void)
{
	if(current_animation)
		current_animation->ApplyCurrentFrame();
}

bool CompareTriangleDist(tTriangle *a, tTriangle *b)
{
	return a->cam_dist > b->cam_dist;
}

/*bool CompareTriangleMaterial(tTriangle *a, tTriangle *b)
{
	//if(a->mat == b->mat)
	//	return CompareDist(a, b);
	//else
	if(b->mat->GetTransparent() == a->mat->GetTransparent())
		return a->mat < b->mat;
	else
		return b->mat->GetTransparent() && !a->mat->GetTransparent();
}*/

/*bool CompareMaterialTransparency(tMaterial *a, tMaterial *b)
{
	if(b->GetTransparent() == a->GetTransparent())
		return a < b;
	else
		return b->GetTransparent() && !a->GetTransparent();
}*/

/*void tMesh::SortTriangles(tVector cam)
{
	//vector<CTriangle *>::iterator t;
	//vector<CVertex *>::iterator v;

	//for(v=vertices.begin(); v!=vertices.end(); v++)
	//	(*v)->cam_dist = (cam - ApplyMatrix4(transformation, **v)).SquaredLen();
	
	//for(t=triangles.begin(); t!=triangles.end(); t++)
	//	(*t)->cam_dist = ((*t)->v[0]->cam_dist + (*t)->v[1]->cam_dist + (*t)->v[2]->cam_dist) * (1.0/3.0);

	sort(triangles.begin(), triangles.end(), CompareTriangleMaterial);
	refresh_vbos = true;
}*/

void tMesh::SortMaterials(void)
{
	return;
	//sort(materials.begin(), materials.end(), CompareMaterialTransparency);
}


