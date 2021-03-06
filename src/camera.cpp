
#include "towerengine.h"

tCamera::tCamera(void)
{
	pos = tVec(0.0, 0.0, 0.0);
	dir = tVec(0.0, 0.0, -1.0);
	up = tVec(0.0, 1.0, 0.0);
	near_clip = 0.1;
	far_clip = 300.0;

	fov.top = -5.0f;
	fov.bottom = 5.0f;
	fov.left = -5.0f;
	fov.right = 5.0f;
}

tVector *tCamera::GetRelativeFrustumCorners(float near_clip, float far_clip)
{
	tVector *c = new tVector[8];
	tVector hvec = Cross(dir, up);
	tVector vvec = Cross(hvec, dir);
	hvec.Normalize();
	vvec.Normalize();


	c[0] = c[1] = c[2] = c[3] = dir;
	
	c[0] += fov.bottom * vvec		+ hvec * fov.left;
	c[1] += fov.top * vvec			+ hvec * fov.left;
	c[2] += fov.top * vvec			+ hvec * fov.right;
	c[3] += fov.bottom * vvec		+ hvec * fov.right;

	c[4] = c[0] * far_clip;
	c[5] = c[1] * far_clip;
	c[6] = c[2] * far_clip;
	c[7] = c[3] * far_clip;

	c[0] *= near_clip;
	c[1] *= near_clip;
	c[2] *= near_clip;
	c[3] *= near_clip;

	return c;
}

tVector *tCamera::GetViewRays(void)
{
	tVector *c = new tVector[4];
	
	c[0] = tVec(fov.right, fov.top, 1.0);
	c[1] = tVec(fov.left, fov.top, 1.0);
	c[2] = tVec(fov.left, fov.bottom, 1.0);
	c[3] = tVec(fov.right, fov.bottom, 1.0);

	return c;
}

tVector tCamera::GetScreenRay(float x, float y)
{
	tVector r = tVec(x * fov.right, y * fov.bottom, 1.0); // TODO: Fix for asymmetrical FOV
	r *= far_clip;

	tVector hvec = Cross(dir, up);
	tVector vvec = Cross(hvec, dir);
	hvec.Normalize();
	vvec.Normalize();

	r = r.x * hvec + r.y * vvec + r.z * dir;

	return r;

}

tVector2 tCamera::GetProjectedPoint(tVector point)
{
	tVector hvec = Cross(dir, up);
	tVector vvec = Cross(hvec, dir);
	hvec.Normalize();
	vvec.Normalize();

	float dist = Dot(dir, point - pos);
	tVector proj_center = pos + dir * dist;

	float height = dist * fov.bottom; // TODO: Fix for asymmetrical fov
	float width = dist * fov.right;

	tVector2 proj;
	proj.x = (float)Dot(point - proj_center, hvec) / width;
	proj.y = (float)Dot(point - proj_center, vvec) / height;

	return proj;
}


void tCamera::CalculateFrustumPlanes(void)
{
	tVector *points = frustum_planes_points;
	tVector *normals = frustum_planes_normals;

	tVector hvec = Cross(dir, up);
	tVector vvec = Cross(hvec, dir);
	hvec.Normalize();
	vvec.Normalize();

	tVector aux;

	points[0] = pos + dir * near_clip;
	normals[0] = dir;

	points[1] = pos + dir * far_clip;
	normals[1] = -dir;

	aux = (points[0] + vvec * near_clip * fov.top) - pos;
	aux.Normalize();
	normals[2] = Cross(aux, hvec);
	points[2] = points[0] + vvec * near_clip * fov.top;

	aux = (points[0] + vvec * near_clip * fov.bottom) - pos;
	aux.Normalize();
	normals[3] = Cross(hvec, aux);
	points[3] = points[0] + vvec * near_clip * fov.bottom;

	aux = (points[0] + hvec * near_clip * fov.left) - pos;
	aux.Normalize();
	normals[4] = Cross(aux, vvec);
	points[4] = points[0] + hvec * near_clip * fov.left;
	
	aux = (points[0] + hvec * near_clip * fov.right) - pos;
	aux.Normalize();
	normals[5] = Cross(vvec, aux);
	points[5] = points[0] + hvec * near_clip * fov.right;
}

void tCamera::InitCulling(void)
{
	CalculateFrustumPlanes();
}

bool tCamera::TestPointCulling(tVector point)
{
	for(int i=0; i<6; i++)
	{
		if(Dot(point - frustum_planes_points[i], frustum_planes_normals[i]) < 0.0)
			return true;
	}
	return false;
}

bool tCamera::TestSphereCulling(tVector center, float radius)
{
	for(int i=0; i<6; i++)
	{
		if(Dot(center - frustum_planes_points[i], frustum_planes_normals[i]) < -radius)
			return true;
	}
	return false;
}

bool tCamera::TestAABBCulling(tAABB b)
{
	if(b.GetInfinite())
		return false;

	tVector p, n;
	tVector normal;

	for(int i=0; i<6; i++)
	{
		normal = frustum_planes_normals[i];

		p = b.GetMin();
		if(normal.x >= 0)
			p.x = b.GetMax().x;
		if(normal.y >=0)
			p.y = b.GetMax().y;
		if(normal.z >= 0)
			p.z = b.GetMax().z;

		n = b.GetMax();
		if(normal.x >= 0)
			n.x = b.GetMin().x;
		if(normal.y >=0)
			n.y = b.GetMin().y;
		if(normal.z >= 0)
			n.z = b.GetMin().z;

		if((Dot(n - frustum_planes_points[i], normal) < 0.0) && (Dot(p - frustum_planes_points[i], normal) < 0.0))
			return true;
	}

	return false;
}

void tCamera::CalculateModelViewProjectionMatrix(void)
{
	tVector to = pos + dir;
	modelview_matrix.SetLookAt(pos, to, up);

	projection_matrix.SetFrustum(	fov.left * near_clip, 
									fov.right * near_clip, 
									fov.bottom * near_clip,
									fov.top * near_clip, 
									near_clip,
									far_clip);

	modelview_projection_matrix = projection_matrix * modelview_matrix;
}

void tCamera::SetFOV(float left, float right, float bottom, float top)
{
	fov.left = left;
	fov.right = right;
	fov.bottom = bottom;
	fov.top = top;
}

void tCamera::SetFOVVerticalAngle(float angle, float aspect)
{
	fov.bottom = -(float)tan(degtorad(angle) * 0.5);
	fov.top = -fov.bottom;
	fov.right = -fov.bottom * aspect;
	fov.left = -fov.right;
}





