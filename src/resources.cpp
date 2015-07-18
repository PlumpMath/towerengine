//-----------------------------------------------------------------------
// This file has been generated by stringres by Metallic Entertainment
// for further information go to http://www.metallic-entertainment.com
//-----------------------------------------------------------------------

#include "tresources.h"

const char *geometry_pass_shader_vert = 
"#version 330\n"
"\n"
"in vec3 vertex_attr;\n"
"in vec3 vertex2_attr; // vertex of next keyframe\n"
"in vec2 uv_attr;\n"
"in vec3 normal_attr;\n"
"in vec3 tang_attr;\n"
"in vec3 bitang_attr;\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"uniform float vertex_mix_uni;\n"
"uniform mat4 transformation_uni;\n"
"\n"
"out vec3 pos_var;\n"
"out vec3 normal_var;\n"
"out vec3 tang_var;\n"
"out vec3 bitang_var;\n"
"out vec2 uv_var;\n"
"\n"
"out vec3 cam_dir_var;\n"
"\n"
"out vec3 reflection_center_var;\n"
"out float reflection_radius_var;\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	vec3 vertex_pos = vertex_attr;\n"
"	if(vertex_mix_uni > 0.0)\n"
"		vertex_pos = vertex_pos * (1.0 - vertex_mix_uni) + vertex2_attr * vertex_mix_uni;\n"
"	vec4 pos = vec4(vertex_pos, 1.0) * transformation_uni;\n"
"	pos_var = pos.xyz;\n"
"	normal_var = normalize(normal_attr * mat3(transformation_uni)); // TODO: correct transformation of normals\n"
"	tang_var = normalize(tang_attr * mat3(transformation_uni));\n"
"	bitang_var = normalize(bitang_attr * mat3(transformation_uni));\n"
"	uv_var = uv_attr;\n"
"	cam_dir_var = cam_pos_uni - pos.xyz;\n"
"	\n"
"	reflection_center_var = (vec4(0.0, 0.0, 0.0, 1.0) * transformation_uni).xyz;\n"
"	reflection_radius_var = 1.0;\n"
"	\n"
"	gl_Position = modelview_projection_matrix_uni * pos;\n"
"}";

const char *geometry_pass_shader_frag = 
"#version 330\n"
"\n"
"uniform vec3 diffuse_color_uni;\n"
"uniform vec4 diffuse_color2_uni;\n"
"uniform vec3 specular_color_uni;\n"
"uniform float specular_size_uni;\n"
"uniform float bump_depth_uni;\n"
"uniform vec3 self_illumination_color_uni;\n"
"\n"
"uniform bool diffuse_tex_enabled_uni;\n"
"uniform bool specular_tex_enabled_uni;\n"
"uniform bool normal_tex_enabled_uni;\n"
"uniform bool bump_tex_enabled_uni;\n"
"uniform bool self_illumination_tex_enabled_uni;\n"
"\n"
"uniform sampler2D diffuse_tex_uni;\n"
"uniform sampler2D normal_tex_uni;\n"
"uniform sampler2D specular_tex_uni;\n"
"uniform sampler2D bump_tex_uni;\n"
"uniform sampler2D self_illumination_tex_uni;\n"
"\n"
"uniform bool cube_map_reflection_enabled_uni;\n"
"uniform vec3 cube_map_reflection_color_uni;\n"
"uniform samplerCube cube_map_reflection_tex_uni;\n"
"\n"
"\n"
"uniform vec3 clip_vec_uni;\n"
"uniform float clip_dist_uni;\n"
"\n"
"in vec3 pos_var;\n"
"in vec3 normal_var;\n"
"in vec3 tang_var;\n"
"in vec3 bitang_var;\n"
"in vec2 uv_var;\n"
"\n"
"in vec3 cam_dir_var;\n"
"\n"
"in vec3 reflection_center_var;\n"
"in float reflection_radius_var;\n"
"\n"
"layout (location = 0) out vec4 position_out;\n"
"layout (location = 1) out vec4 diffuse_out;\n"
"layout (location = 2) out vec4 normal_out;\n"
"layout (location = 3) out vec4 face_normal_out;\n"
"layout (location = 4) out vec4 specular_out; \n"
"layout (location = 5) out vec4 self_illumination_out;\n"
"\n"
"vec2 ParallaxUV(void);\n"
"vec2 ParallaxOcclusionUV(mat3 tang_mat);\n"
"\n"
"void main(void)\n"
"{\n"
"	if(!gl_FrontFacing) // backface culling\n"
"		discard;\n"
"		\n"
"	if(clip_vec_uni != vec3(0.0, 0.0, 0.0)) // face clipping for water\n"
"	{\n"
"		vec3 clip = pos_var - clip_vec_uni * clip_dist_uni;\n"
"		if(dot(clip, clip_vec_uni) >= 0.0)\n"
"			discard;\n"
"	}\n"
"	\n"
"	mat3 tang_mat = mat3(normalize(tang_var), normalize(bitang_var), normalize(normal_var));\n"
"	\n"
"	vec2 uv;\n"
"	\n"
"	if(bump_tex_enabled_uni)\n"
"		uv = ParallaxOcclusionUV(tang_mat);\n"
"	else\n"
"		uv = uv_var;\n"
"				\n"
"	\n"
"	// diffuse\n"
"	\n"
"	vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"	\n"
"	if(diffuse_tex_enabled_uni)\n"
"		diffuse_color = texture(diffuse_tex_uni, uv).rgba;\n"
"		\n"
"	if(diffuse_color.a < 0.5)\n"
"		discard;\n"
"		\n"
"	diffuse_color *= vec4(diffuse_color_uni.rgb, 1.0) * diffuse_color2_uni;\n"
"	\n"
"	\n"
"	//normal\n"
"	\n"
"	vec3 normal;\n"
"	if(normal_tex_enabled_uni)\n"
"	{\n"
"		vec3 normal_tex_color = texture(normal_tex_uni, uv).rgb;\n"
"		normal = tang_mat * ((normal_tex_color - vec3(0.5, 0.5, 0.5)) * 2.0);\n"
"	}\n"
"	else\n"
"		normal = normal_var;\n"
"		\n"
"	normal_out = vec4(normal * 0.5 + vec3(0.5, 0.5, 0.5), 1.0);\n"
"	\n"
"	face_normal_out = vec4(normal_var * 0.5 + vec3(0.5, 0.5, 0.5), 1.0);\n"
"	\n"
"	\n"
"	// specular\n"
"			\n"
"	vec3 specular_color = specular_color_uni;\n"
"	if(specular_tex_enabled_uni)\n"
"		specular_color *= texture(specular_tex_uni, uv).rgb;\n"
"	\n"
"		\n"
"	// self illumination\n"
"	\n"
"	vec3 self_illumination = self_illumination_color_uni;\n"
"	\n"
"	if(self_illumination_tex_enabled_uni)\n"
"		self_illumination *= texture(self_illumination_tex_uni, uv).rgb;\n"
"		\n"
"	\n"
"	// cube map reflection\n"
"	\n"
"	if(cube_map_reflection_enabled_uni)\n"
"	{\n"
"		vec3 cam_reflected = reflect(-cam_dir_var, normal);\n"
"		cam_reflected += (pos_var - reflection_center_var) / reflection_radius_var;\n"
"		self_illumination += texture(cube_map_reflection_tex_uni, cam_reflected).rgb * cube_map_reflection_color_uni;\n"
"	}\n"
"		\n"
"	\n"
"	// out\n"
"	\n"
"	position_out = vec4(pos_var, 1.0);\n"
"	diffuse_out = diffuse_color;\n"
"	specular_out = vec4(specular_color, specular_size_uni);\n"
"	self_illumination_out = vec4(self_illumination, 1.0);\n"
"}\n"
"\n"
"\n"
"vec2 ParallaxUV(void)\n"
"{\n"
"	float height = texture(bump_tex_uni, uv_var).r * bump_depth_uni;\n"
"	vec3 cam_offset = height * normalize(cam_dir_var);\n"
"	vec2 uv_offset;\n"
"	uv_offset.x = dot(cam_offset, tang_var);\n"
"	uv_offset.y = dot(cam_offset, bitang_var);\n"
"	return uv_var + uv_offset;\n"
"}\n"
"\n"
"#define PARALLAX_OCCLUSION_MIN_SAMPLES 4\n"
"#define PARALLAX_OCCLUSION_MAX_SAMPLES 15\n"
"\n"
"vec2 ParallaxOcclusionUV(mat3 tang_mat)\n"
"{\n"
"	vec3 cam_dir_tang = cam_dir_var * tang_mat;\n"
"	\n"
"	float parallax_limit = -length(cam_dir_tang.xy) / cam_dir_tang.z;\n"
"	parallax_limit *= bump_depth_uni;\n"
"	\n"
"	vec2 offset_dir = normalize(cam_dir_tang.xy);\n"
"		\n"
"	vec2 max_offset = offset_dir * parallax_limit;\n"
"	\n"
"	int samples_count = int(mix(PARALLAX_OCCLUSION_MAX_SAMPLES, PARALLAX_OCCLUSION_MIN_SAMPLES, dot(-normalize(cam_dir_var), normal_var)));\n"
"	float step_size = 1.0 / float(samples_count);\n"
"	\n"
"	vec2 dx = dFdx(uv_var);\n"
"	vec2 dy = dFdy(uv_var);\n"
"	\n"
"	float current_ray_height = 1.0;\n"
"	vec2 current_offset = vec2(0.0, 0.0);\n"
"	vec2 last_offset = vec2(0.0, 0.0);\n"
"	\n"
"	float last_sampled_height = 1.0;\n"
"	float current_sampled_height = 1.0;\n"
"	\n"
"	int current_sample = 0;\n"
"	\n"
"	while(current_sample < samples_count)\n"
"	{\n"
"		current_sampled_height = textureGrad(bump_tex_uni, uv_var + current_offset, dx, dy).r;\n"
"		\n"
"		if(current_sampled_height > current_ray_height)\n"
"		{\n"
"			float delta1 = current_sampled_height - current_ray_height;\n"
"			float delta2 = (current_ray_height + step_size) - last_sampled_height;\n"
"			float ratio = delta1 / (delta1 + delta2);\n"
"			\n"
"			current_offset = ratio * last_offset + (1.0 - ratio) * current_offset;\n"
"			\n"
"			break;\n"
"		}		\n"
"		else\n"
"		{\n"
"			current_sample++;\n"
"			\n"
"			current_ray_height -= step_size;\n"
"			last_offset = current_offset;\n"
"			current_offset += step_size * max_offset;\n"
"			\n"
"			last_sampled_height = current_sampled_height;\n"
"		}\n"
"	}\n"
"	\n"
"	return uv_var + current_offset;\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"";

const char *screen_shader_vert = 
"#version 330\n"
"\n"
"in vec2 vertex_attr;\n"
"\n"
"out vec2 uv_coord_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	uv_coord_var = (vertex_attr + 1.0) * 0.5;\n"
"	gl_Position = vec4(vertex_attr, 0.0, 1.0);\n"
"}\n"
"\n"
"";

const char *directional_lighting_shader_frag = 
"#version 330\n"
"\n"
"#define MAX_DIRECTIONAL_SHADOW_SPLITS 8\n"
"#define DIRECTIONAL_LIGHT_SHADOW_LAYER_BLEND_DIST 0.5\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"uniform vec3 directional_light_dir_uni;\n"
"uniform vec3 directional_light_color_uni;\n"
"uniform bool directional_light_shadow_enabled_uni;\n"
"uniform vec2 directional_light_shadow_clip_uni;\n"
"uniform mat4 directional_light_shadow_tex_matrix_uni[MAX_DIRECTIONAL_SHADOW_SPLITS];\n"
"uniform int directional_light_shadow_splits_count_uni;\n"
"uniform float directional_light_shadow_splits_z_uni[MAX_DIRECTIONAL_SHADOW_SPLITS+1];\n"
"uniform sampler2DArray directional_light_shadow_map_uni;\n"
"\n"
"uniform sampler2D position_tex_uni;\n"
"uniform sampler2D diffuse_tex_uni;\n"
"uniform sampler2D normal_tex_uni;\n"
"uniform sampler2D specular_tex_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"\n"
"float linstep(float min, float max, float v)\n"
"{\n"
"	return clamp((v - min) / (max - min), 0.0, 1.0);\n"
"}\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	ivec2 texel_uv = ivec2(uv_coord_var * textureSize(diffuse_tex_uni, 0).xy);\n"
"	\n"
"	vec4 diffuse = texelFetch(diffuse_tex_uni, texel_uv, 0).rgba;\n"
"\n"
"	if(diffuse.a == 0.0)\n"
"		discard;\n"
"		\n"
"	vec3 position = texelFetch(position_tex_uni, texel_uv, 0).rgb; 	\n"
"	vec3 normal = normalize(texelFetch(normal_tex_uni, texel_uv, 0).rgb * 2.0 - vec3(1.0, 1.0, 1.0));\n"
"	vec4 specular = texelFetch(specular_tex_uni, texel_uv, 0).rgba;\n"
"	\n"
"	\n"
"	vec3 cam_dir = normalize(cam_pos_uni - position.xyz);\n"
"	\n"
"	float shadow = 1.0;	\n"
"\n"
"	if(directional_light_shadow_enabled_uni)\n"
"	{ \n"
"		vec2 coord = vec2(0.0, 0.0);\n"
"		int layer = -1;\n"
"		\n"
"		for(int s=0; s<directional_light_shadow_splits_count_uni; s++)\n"
"		{\n"
"			coord = ((directional_light_shadow_tex_matrix_uni[s] * vec4(position.xyz, 1.0)).xy * 0.5) + vec2(0.5, 0.5);\n"
"			\n"
"			if(!(coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0))\n"
"			{	\n"
"				layer = s;				\n"
"				break;\n"
"			}\n"
"		}\n"
"		\n"
"		if(layer != -1)\n"
"		{\n"
"			vec2 moments = texture(directional_light_shadow_map_uni, vec3(coord, float(layer))).rg;\n"
"										\n"
"			float light_dot = dot(position.xyz - cam_pos_uni, -directional_light_dir_uni) / (directional_light_shadow_clip_uni.y - directional_light_shadow_clip_uni.x);\n"
"\n"
"			if(light_dot <= moments.x)\n"
"				shadow = 1.0;\n"
"			else\n"
"			{\n"
"				float p = smoothstep(light_dot-0.0005, light_dot, moments.x);\n"
"			    float variance = max(moments.y - moments.x*moments.x, -0.001);\n"
"			    float d = light_dot - moments.x;\n"
"			    float p_max = linstep(0.5, 1.0, variance / (variance + d*d));\n"
"			    \n"
"			   	shadow = clamp(max(p, p_max), 0.0, 1.0);\n"
"			}	\n"
"		}\n"
"		else\n"
"			shadow = 1.0;\n"
"	}\n"
"	else\n"
"		shadow = 1.0;\n"
"		\n"
"		\n"
"	float light_intensity = max(dot(normal, directional_light_dir_uni), 0.0);\n"
"	vec3 color = shadow * light_intensity * directional_light_color_uni * diffuse.rgb; // diffuse light\n"
"\n"
"	//specular\n"
"	vec3 specular_color = specular.rgb * directional_light_color_uni;\n"
"	float specular_intensity = max(dot(normalize(reflect(-directional_light_dir_uni, normal)), cam_dir), 0.0);\n"
"	color += max(vec3(0.0, 0.0, 0.0), specular_color * pow(specular_intensity, specular.a)) * shadow;\n"
"	\n"
"	\n"
"	color_out = vec4(color, 1.0);\n"
"}\n"
"";

const char *point_lighting_shader_frag = 
"#version 330\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"#define LIGHTS_COUNT $(param LIGHTS_COUNT)\n"
"\n"
"\n"
"uniform vec3 point_light_pos_uni[LIGHTS_COUNT];\n"
"uniform vec3 point_light_color_uni[LIGHTS_COUNT];\n"
"uniform float point_light_distance_uni[LIGHTS_COUNT];\n"
"uniform bool point_light_shadow_enabled_uni[LIGHTS_COUNT];\n"
"uniform samplerCube point_light_shadow_map_uni[LIGHTS_COUNT];\n"
"\n"
"\n"
"uniform sampler2D position_tex_uni;\n"
"uniform sampler2D diffuse_tex_uni;\n"
"uniform sampler2D normal_tex_uni;\n"
"uniform sampler2D specular_tex_uni;\n"
"\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"float linstep(float min, float max, float v)\n"
"{\n"
"	return clamp((v - min) / (max - min), 0.0, 1.0);\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"	ivec2 texel_uv = ivec2(uv_coord_var * textureSize(diffuse_tex_uni, 0).xy);\n"
"	\n"
"	vec4 diffuse = texelFetch(diffuse_tex_uni, texel_uv, 0).rgba;\n"
"\n"
"	if(diffuse.a == 0.0)\n"
"		discard;\n"
"		\n"
"	vec3 position = texelFetch(position_tex_uni, texel_uv, 0).rgb; 	\n"
"	vec3 normal = normalize(texelFetch(normal_tex_uni, texel_uv, 0).rgb * 2.0 - vec3(1.0, 1.0, 1.0));\n"
"	vec4 specular = texelFetch(specular_tex_uni, texel_uv, 0).rgba;\n"
"	\n"
"	vec3 cam_dir = normalize(cam_pos_uni - position.xyz);\n"
"	\n"
"	float shadow;\n"
"	vec3 light_dir;\n"
"	float light_dist_sq;\n"
"	float light_dist;\n"
"	float light_dist_attenuation;\n"
"	float light_intensity;\n"
"	vec3 specular_color;\n"
"	float specular_intensity;\n"
"	vec3 color = vec3(0.0, 0.0, 0.0);\n"
"	\n"
"	$(for i from 0 ex param LIGHTS_COUNT)\n"
"		shadow = 1.0;\n"
"	\n"
"		light_dir = point_light_pos_uni[$(var i)] - position.xyz; // pos to light\n"
"		light_dist_sq = light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z; // squared distance\n"
"		if(light_dist_sq <= point_light_distance_uni[$(var i)] * point_light_distance_uni[$(var i)])\n"
"		{\n"
"			light_dist = sqrt(light_dist_sq); // real distance\n"
"			light_dir /= light_dist; // normalized dir\n"
"			\n"
"			if(point_light_shadow_enabled_uni[$(var i)])\n"
"			{ \n"
"				vec2 moments = texture(point_light_shadow_map_uni[$(var i)], -light_dir).rg;\n"
"				\n"
"				float light_depth = length(point_light_pos_uni[$(var i)] - position.xyz) - 0.01;\n"
"										\n"
"				// Surface is fully lit. as the current fragment is before the light occluder\n"
"				if(light_depth <= moments.x)\n"
"					shadow = 1.0;\n"
"				else\n"
"				{\n"
"					float p = smoothstep(light_depth-0.00005, light_depth, moments.x);\n"
"				    float variance = max(moments.y - moments.x*moments.x, -0.001);\n"
"				    float d = light_depth - moments.x;\n"
"				    float p_max = linstep(0.3, 1.0, variance / (variance + d*d));\n"
"				    \n"
"				    shadow = p_max;//clamp(max(p, p_max), 0.0, 1.0);\n"
"				}\n"
"			}\n"
"			else\n"
"				shadow = 1.0;\n"
"		\n"
"		\n"
"			light_dist_attenuation = (1.0 - light_dist / point_light_distance_uni[$(var i)]);\n"
"			light_intensity = max(dot(normal, light_dir), 0.0) *  light_dist_attenuation;\n"
"			color += shadow * light_intensity * point_light_color_uni[$(var i)] * diffuse.rgb; // diffuse light\n"
"		\n"
"			//specular\n"
"			specular_color = specular.rgb * point_light_color_uni[$(var i)];\n"
"			specular_intensity = max(dot(normalize(reflect(-light_dir, normal)), cam_dir), 0.0);\n"
"			color += max(vec3(0.0, 0.0, 0.0), specular_color * pow(specular_intensity, specular.a)) * shadow * light_dist_attenuation;\n"
"		}\n"
"	$(end for)\n"
"	\n"
"	color_out = vec4(color, 1.0);\n"
"}";

const char *ambient_lighting_shader_frag = 
"#version 330\n"
"\n"
"uniform vec3 light_ambient_color_uni;\n"
"\n"
"uniform sampler2D diffuse_tex_uni;\n"
"uniform sampler2D self_illumination_tex_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	ivec2 texel_uv = ivec2(uv_coord_var * textureSize(diffuse_tex_uni, 0).xy);\n"
"\n"
"	vec4 diffuse = texelFetch(diffuse_tex_uni, texel_uv, 0).rgba;\n"
"\n"
"	if(diffuse.a == 0.0)\n"
"		discard;\n"
"		\n"
"	vec3 self_illumination = texelFetch(self_illumination_tex_uni, texel_uv, 0).rgb;\n"
"		\n"
"	vec3 color = light_ambient_color_uni * diffuse.rgb + self_illumination;\n"
"	\n"
"	color_out = vec4(color, 1.0);\n"
"}";

const char *ssao_ambient_lighting_shader_frag = 
"#version 330\n"
"\n"
"uniform vec3 light_ambient_color_uni;\n"
"\n"
"uniform sampler2D ssao_tex_uni;\n"
"uniform sampler2D diffuse_tex_uni;\n"
"uniform sampler2D self_illumination_tex_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec4 diffuse = texture(diffuse_tex_uni, uv_coord_var).rgba;\n"
"\n"
"	if(diffuse.a == 0.0)\n"
"		discard;\n"
"	\n"
"	float occlusion = texture(ssao_tex_uni, uv_coord_var).r;\n"
"	vec3 self_illumination = texture(self_illumination_tex_uni, uv_coord_var).rgb;\n"
"		\n"
"	vec3 color = light_ambient_color_uni * diffuse.rgb * occlusion + self_illumination;\n"
"	\n"
"	color_out = vec4(color, 1.0);\n"
"}";

const char *ssao_lighting_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2D ssao_tex_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	float occlusion = texture(ssao_tex_uni, uv_coord_var).r;\n"
"	\n"
"	color_out = vec4(vec3(1.0) * occlusion, 1.0);\n"
"}";

const char *cube_env_shader_vert = 
"#version 330\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"in vec3 vertex_attr;\n"
"\n"
"out vec3 pos_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	pos_var = vertex_attr;\n"
"	gl_Position = modelview_projection_matrix_uni * vec4(cam_pos_uni + vertex_attr, 1.0);\n"
"}";

const char *cube_env_shader_frag = 
"#version 330\n"
"\n"
"uniform samplerCube cube_map_uni;\n"
"\n"
"in vec3 pos_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec3 color = texture(cube_map_uni, normalize(pos_var)).xyz;\n"
"	color_out = vec4(color, 0.0);\n"
"}";

const char *point_shadow_shader_vert = 
"#version 330\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"in vec3 vertex_attr;\n"
"in vec3 vertex2_attr; // vertex of next keyframe\n"
"\n"
"uniform float vertex_mix_uni;\n"
"uniform mat4 transformation_uni;\n"
"\n"
"out vec3 pos_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec3 vertex_pos = vertex_attr;\n"
"	if(vertex_mix_uni > 0.0)\n"
"		vertex_pos = vertex_pos * (1.0 - vertex_mix_uni) + vertex2_attr * vertex_mix_uni;\n"
"		\n"
"	vec4 pos = vec4(vertex_pos, 1.0) * transformation_uni;\n"
"	pos_var = pos.xyz;\n"
"	\n"
"	gl_Position = modelview_projection_matrix_uni * pos;\n"
"}";

const char *point_shadow_shader_frag = 
"#version 330\n"
"\n"
"in vec3 pos_var;\n"
"\n"
"uniform vec3 light_pos_uni;\n"
"uniform float light_dist_uni;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec3 dir = pos_var - light_pos_uni;\n"
"	float dist = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;\n"
"	\n"
"	if(dist > light_dist_uni * light_dist_uni)\n"
"		discard;\n"
"	\n"
"	dist = sqrt(dist);\n"
"	\n"
"	color_out = vec4(dist, dist*dist, 0.0, 1.0);\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"";

const char *point_shadow_blur_shader_vert = 
"#version 330\n"
"\n"
"in vec2 vertex_attr;\n"
"\n"
"uniform vec3 blur_dir_uni;\n"
"\n"
"out vec3 coords_var[6];\n"
"out vec3 blur_dir_var[6];\n"
"\n"
"void main(void)\n"
"{\n"
"	coords_var[0] = vec3(1.0, -vertex_attr.y, -vertex_attr.x);\n"
"	coords_var[1] = vec3(-1.0, -vertex_attr.y, vertex_attr.x);\n"
"	\n"
"	coords_var[2] = vec3(vertex_attr.x, 1.0, vertex_attr.y);\n"
"	coords_var[3] = vec3(vertex_attr.x, -1.0, -vertex_attr.y);\n"
"	\n"
"	coords_var[4] = vec3(vertex_attr.x, -vertex_attr.y, 1.0);\n"
"	coords_var[5] = vec3(-vertex_attr.x, -vertex_attr.y, -1.0);\n"
"	\n"
"	blur_dir_var[0] = vec3(0.0, blur_dir_uni.y, blur_dir_uni.x);\n"
"	blur_dir_var[1] = vec3(0.0, blur_dir_uni.y, -blur_dir_uni.x);\n"
"	\n"
"	blur_dir_var[2] = vec3(blur_dir_uni.x, 0.0, blur_dir_uni.y);\n"
"	blur_dir_var[3] = vec3(blur_dir_uni.x, 0.0, -blur_dir_uni.y);\n"
"	\n"
"	blur_dir_var[4] = vec3(-blur_dir_uni.x, blur_dir_uni.y, 0.0);\n"
"	blur_dir_var[5] = vec3(blur_dir_uni.x, blur_dir_uni.y, 0.0);\n"
"		\n"
"	gl_Position = vec4(vertex_attr, -1.0, 1.0);\n"
"}\n"
"";

const char *point_shadow_blur_shader_frag = 
"#version 330\n"
"\n"
"uniform samplerCube tex_uni;\n"
"\n"
"in vec3 coords_var[6];\n"
"in vec3 blur_dir_var[6];\n"
"\n"
"out vec4 tex_out[6];\n"
"\n"
"void main()\n"
"{\n"
"	vec2 color;\n"
"\n"
"	for(int s=0; s<6; s++)\n"
"	{\n"
"		color = vec2(0.0, 0.0);\n"
"		\n"
"		color += texture(tex_uni, coords_var[s] - blur_dir_var[s] * 2.0).rg * 0.2;\n"
"		color += texture(tex_uni, coords_var[s] - blur_dir_var[s]).rg * 0.2;\n"
"		color += texture(tex_uni, coords_var[s]).rg * 0.2;\n"
"		color += texture(tex_uni, coords_var[s] + blur_dir_var[s]).rg * 0.2;\n"
"		color += texture(tex_uni, coords_var[s] + blur_dir_var[s] * 2.0).rg * 0.2;\n"
"		\n"
"		tex_out[s] = vec4(color, 0.0, 1.0);\n"
"	}\n"
"}";

const char *directional_shadow_shader_vert = 
"#version 330\n"
"\n"
"in vec3 vertex_attr;\n"
"in vec3 vertex2_attr; // vertex of next keyframe\n"
"in vec2 uv_attr;\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"uniform float vertex_mix_uni;\n"
"uniform mat4 transformation_uni;\n"
"\n"
"uniform vec3 light_dir_uni;\n"
"uniform vec2 clip_uni;\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"out float moment1_var;\n"
"out vec2 uv_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec3 vertex_pos = vertex_attr;\n"
"	if(vertex_mix_uni > 0.0)\n"
"		vertex_pos = vertex_pos * (1.0 - vertex_mix_uni) + vertex2_attr * vertex_mix_uni;\n"
"	vec4 pos = vec4(vertex_pos, 1.0) * transformation_uni;\n"
"	\n"
"	moment1_var = dot(pos.xyz - cam_pos_uni, light_dir_uni) / (clip_uni.y - clip_uni.x);\n"
"	uv_var = uv_attr;\n"
"	\n"
"	gl_Position = modelview_projection_matrix_uni * pos;\n"
"}";

const char *directional_shadow_shader_frag = 
"#version 330\n"
"\n"
"uniform bool diffuse_tex_enabled_uni;\n"
"uniform sampler2D diffuse_tex_uni;\n"
"\n"
"in float moment1_var;\n"
"in vec2 uv_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	if(diffuse_tex_enabled_uni)\n"
"	{\n"
"		if(texture(diffuse_tex_uni, uv_var).a < 0.5)\n"
"			discard;\n"
"	}\n"
"		\n"
"	color_out = vec4(moment1_var, moment1_var * moment1_var, 0.0, 1.0);\n"
"}\n"
"";

const char *directional_shadow_blur_shader_vert = 
"#version 330\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"in vec2 vertex_attr;\n"
"in vec2 uv_coord_attr;\n"
"\n"
"out vec2 uv_coord_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	uv_coord_var = uv_coord_attr;\n"
"	gl_Position = modelview_projection_matrix_uni * vec4(vertex_attr, 0.0, 1.0);\n"
"}\n"
"";

const char *directional_shadow_blur_shader_frag = 
"#version 330\n"
"\n"
"#define MAX_LAYERS 8\n"
"\n"
"#define GAUSSIAN_BLUR\n"
"\n"
"uniform sampler2DArray tex_uni;\n"
"\n"
"uniform vec2 blur_dir_uni;\n"
"uniform float blur_factors_uni[MAX_LAYERS];\n"
"uniform int tex_layers_count_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 tex_out[MAX_LAYERS];\n"
"\n"
"void main()\n"
"{\n"
"	vec2 color;\n"
"	float layer;\n"
"	vec2 blur_dir;\n"
"	\n"
"	for(int s=0; s<tex_layers_count_uni; s++)\n"
"	{\n"
"		layer = float(s);\n"
"		color = vec2(0.0, 0.0);\n"
"		blur_dir = blur_dir_uni * blur_factors_uni[s];\n"
"		\n"
"		#ifdef GAUSSIAN_BLUR	\n"
"		\n"
"		color += texture(tex_uni, vec3(uv_coord_var - blur_dir * 2.0, layer)).rg * 0.06;\n"
"		color += texture(tex_uni, vec3(uv_coord_var - blur_dir, layer)).rg * 0.24;\n"
"		color += texture(tex_uni, vec3(uv_coord_var, layer)).rg * 0.4;\n"
"		color += texture(tex_uni, vec3(uv_coord_var + blur_dir, layer)).rg * 0.24;\n"
"		color += texture(tex_uni, vec3(uv_coord_var + blur_dir * 2.0, layer)).rg * 0.06;\n"
"		\n"
"		#else\n"
"		\n"
"		color += texture(tex_uni, vec3(uv_coord_var - blur_dir * 2.0, layer)).rg * 0.2;\n"
"		color += texture(tex_uni, vec3(uv_coord_var - blur_dir, layer)).rg * 0.2;\n"
"		color += texture(tex_uni, vec3(uv_coord_var, layer)).rg * 0.2;\n"
"		color += texture(tex_uni, vec3(uv_coord_var + blur_dir, layer)).rg * 0.2;\n"
"		color += texture(tex_uni, vec3(uv_coord_var + blur_dir * 2.0, layer)).rg * 0.2;\n"
"		\n"
"		#endif\n"
"				\n"
"		tex_out[s] = vec4(color, 0.0, 1.0);\n"
"	}\n"
"}";

const char *post_process_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2D color_tex_uni;\n"
"uniform vec2 tex_pixel_uni;\n"
"\n"
"uniform bool fxaa_enabled_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"#define FXAA_EDGE_THRESHOLD_MIN	(1.0 / 16.0)\n"
"#define FXAA_EDGE_THRESHOLD		(1.0 / 8.0)\n"
"#define FXAA_SPAN_MAX			8.0\n"
"#define FXAA_REDUCE_MUL			(1.0 / 8.0)\n"
"#define FXAA_REDUCE_MIN			(1.0 / 128.0)\n"
"\n"
"vec3 FXAA(vec3 rgb_m)\n"
"{\n"
"	vec3 rgb_nw = texture(color_tex_uni, uv_coord_var + vec2(-1.0, 1.0) * tex_pixel_uni).rgb;\n"
"	vec3 rgb_ne = texture(color_tex_uni, uv_coord_var + vec2(1.0, 1.0) * tex_pixel_uni).rgb;\n"
"	vec3 rgb_sw = texture(color_tex_uni, uv_coord_var + vec2(-1.0, -1.0) * tex_pixel_uni).rgb;\n"
"	vec3 rgb_se = texture(color_tex_uni, uv_coord_var + vec2(1.0, -1.0) * tex_pixel_uni).rgb;\n"
"	\n"
"	vec3 luma = vec3(0.299, 0.587, 0.114);\n"
"	\n"
"	float luma_nw = dot(rgb_nw, luma);\n"
"	float luma_ne = dot(rgb_ne, luma);\n"
"	float luma_sw = dot(rgb_sw, luma);\n"
"	float luma_se = dot(rgb_se, luma);\n"
"	float luma_m = dot(rgb_m, luma);\n"
"	\n"
"	float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));\n"
"	float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));\n"
"	\n"
"	if(luma_max - luma_min < max(FXAA_EDGE_THRESHOLD_MIN, luma_max * FXAA_EDGE_THRESHOLD))\n"
"		return rgb_m;\n"
"		\n"
"\n"
"	vec2 dir;\n"
"	dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));\n"
"	dir.y = ((luma_nw + luma_sw) - (luma_ne + luma_se));\n"
"	\n"
"	float dir_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);\n"
"	float rcp_dir_min = 1.0/(min(abs(dir.x), abs(dir.y)) + dir_reduce);\n"
"	\n"
"	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),\n"
"			max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcp_dir_min)) * tex_pixel_uni;\n"
"			\n"
"			\n"
"	vec3 rgb_a = (1.0/2.0) * (\n"
"			texture(color_tex_uni, uv_coord_var + dir * (1.0/3.0 - 0.5)).xyz +\n"
"			texture(color_tex_uni, uv_coord_var + dir * (2.0/3.0 - 0.5)).xyz);\n"
"	vec3 rgb_b = rgb_a * (1.0/2.0) + (1.0/4.0) * (\n"
"			texture(color_tex_uni, uv_coord_var + dir * (0.0/3.0 - 0.5)).xyz +\n"
"			texture(color_tex_uni, uv_coord_var + dir * (3.0/3.0 - 0.5)).xyz);\n"
"	float luma_b = dot(rgb_b, luma);\n"
"	\n"
"	\n"
"	if((luma_b < luma_min) || (luma_b > luma_max))\n"
"		return rgb_a;\n"
"	else\n"
"		return rgb_b;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"	vec4 src_color = texture(color_tex_uni, uv_coord_var);\n"
"	vec3 color = vec3(0.0, 0.0, 0.0);\n"
"	\n"
"	if(fxaa_enabled_uni)\n"
"		color = FXAA(src_color.rgb);\n"
"	else\n"
"		color = src_color.rgb;	\n"
"		\n"
"	\n"
"	color_out = vec4(color, src_color.a);\n"
"}";

const char *ssao_shader_frag = 
"#version 330\n"
"\n"
"#define MAX_KERNEL_SIZE 32\n"
"\n"
"uniform vec3 kernel_uni[MAX_KERNEL_SIZE];\n"
"uniform int kernel_size_uni;\n"
"\n"
"uniform sampler2D noise_tex_uni;\n"
"uniform vec2 noise_tex_scale_uni;\n"
"\n"
"uniform sampler2D depth_tex_uni;\n"
"uniform sampler2D position_tex_uni;\n"
"uniform sampler2D normal_tex_uni;\n"
"\n"
"uniform mat4 projection_matrix_uni;\n"
"uniform mat4 modelview_matrix_uni;\n"
"\n"
"uniform float radius_uni;\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"uniform vec3 cam_dir_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	mat4 transpose_modelview = transpose(modelview_matrix_uni);\n"
"	vec3 origin = texture(position_tex_uni, uv_coord_var).xyz;\n"
"		\n"
"	vec3 normal = texture(normal_tex_uni, uv_coord_var).rgb * 2.0 - vec3(1.0, 1.0, 1.0);\n"
"	normal = normalize(normal);\n"
"	\n"
"	vec3 rvec = texture(noise_tex_uni, uv_coord_var * noise_tex_scale_uni).rgb * 2.0 - 1.0;\n"
"	vec3 tang = normalize(rvec - normal * dot(rvec, normal));\n"
"	vec3 bitang = cross(normal, tang);\n"
"	mat3 tang_mat = mat3(tang, bitang, normal);\n"
"	\n"
"	float occlusion = 0.0;\n"
"	\n"
"	for(int i=0; i<kernel_size_uni; i++)\n"
"	{\n"
"		vec3 sample = tang_mat * kernel_uni[i];\n"
"		sample = sample * radius_uni + origin;\n"
"		\n"
"		float sample_z = (vec4(sample, 1.0) * transpose_modelview).z; //dot(sample - cam_pos_uni, cam_dir_uni);\n"
"		\n"
"		vec4 offset = vec4(sample, 1.0) * transpose_modelview;\n"
"		offset = projection_matrix_uni * offset;\n"
"		offset.xy /= offset.w;\n"
"		offset.xy = offset.xy * 0.5 + 0.5;\n"
"		\n"
"		if(offset.x > 1.0 || offset.y > 1.0 || offset.x < 0.0 || offset.y < 0.0)\n"
"			continue;\n"
"						\n"
"		vec3 sample_pos = texture(position_tex_uni, offset.xy).xyz;\n"
"		float sample_real_z = (vec4(sample_pos, 1.0) * transpose_modelview).z;//dot(sample_pos - cam_pos_uni, cam_dir_uni);\n"
"		\n"
"		float range_check = abs(sample_real_z - sample_z) < radius_uni ? 1.0 : 0.0;\n"
"		occlusion += sample_real_z < sample_z ? 0.0 : 1.0 * range_check;\n"
"	}\n"
"		\n"
"	occlusion = 1.0 - (occlusion / float(kernel_size_uni));\n"
"		\n"
"	color_out = vec4(occlusion, 0.0, 0.0, 1.0);\n"
"}";

const char *ssao_blur_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2D tex_uni;\n"
"\n"
"uniform bool blur_vertical_uni; // false => horizontal\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	vec2 texel_size = 1.0 / textureSize(tex_uni, 0);\n"
"	float result = 0.0;\n"
"	\n"
"	vec2 offset;\n"
"	\n"
"	if(!blur_vertical_uni) // horizontal\n"
"		offset = vec2(2.0, 0.0) * texel_size.x;\n"
"	else // vertical\n"
"		offset = vec2(0.0, 2.0) * texel_size.y;\n"
"		\n"
"	result += texture(tex_uni, uv_coord_var + offset * 2).r	* 0.06136;\n"
"	result += texture(tex_uni, uv_coord_var + offset).r 	* 0.24477;\n"
"	result += texture(tex_uni, uv_coord_var).r 				* 0.38774;\n"
"	result += texture(tex_uni, uv_coord_var - offset).r 	* 0.24477;\n"
"	result += texture(tex_uni, uv_coord_var - offset * 2).r	* 0.06136;\n"
"	\n"
"	color_out = vec4(result, 0.0, 0.0, 1.0);\n"
"}";

const char *color_shader_vert = 
"#version 330\n"
"\n"
"uniform mat4 modelview_projection_matrix_uni;\n"
"\n"
"uniform mat4 transformation_uni;\n"
"\n"
"in vec3 vertex_attr;\n"
"in vec4 color_attr;\n"
"\n"
"out vec4 color_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	color_var = color_attr;\n"
"	\n"
"	vec4 pos = vec4(vertex_attr, 1.0) * transformation_uni;\n"
"		\n"
"	gl_Position = modelview_projection_matrix_uni * pos;\n"
"}";

const char *color_shader_frag = 
"#version 330\n"
"\n"
"in vec4 color_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	if(!gl_FrontFacing)\n"
"		discard;\n"
"\n"
"	color_out = color_var;\n"
"}\n"
"";

const char *fog_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2D position_tex_uni;\n"
"uniform sampler2D color_tex_uni;\n"
"\n"
"uniform vec3 cam_pos_uni;\n"
"\n"
"uniform float start_dist_uni;\n"
"uniform float end_dist_uni;\n"
"uniform float exp_uni;\n"
"uniform vec3 color_uni;\n"
"\n"
"in vec2 uv_coord_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"\n"
"void main(void)\n"
"{\n"
"	vec4 src_color = texture(color_tex_uni, uv_coord_var);\n"
"	vec3 color = src_color.rgb;\n"
"	\n"
"	vec3 pos = texture(position_tex_uni, uv_coord_var).xyz;\n"
"	\n"
"	vec3 dir = pos - cam_pos_uni;\n"
"	float dist = length(dir);//dir.x*dir.x + dir.y*dir.y + dir.z*dir.z;\n"
"	float att = clamp((dist - start_dist_uni) / end_dist_uni, 0.0, 1.0);\n"
"	att = pow(att, exp_uni) * src_color.a;\n"
"	color = color_uni * att + (1.0 - att) * color; \n"
"	\n"
"	color_out = vec4(color, src_color.a);\n"
"}";

const char *_2d_sprite_shader_vert = 
"#version 330\n"
"\n"
"uniform int screen_size_uni[2];\n"
"\n"
"uniform int sprite_coords_uni[4];\n"
"\n"
"in vec2 vertex_attr;\n"
"\n"
"out vec2 uv_var;\n"
"\n"
"void main(void)\n"
"{\n"
"	vec2 pos = vec2(sprite_coords_uni[0], sprite_coords_uni[1]);\n"
"	pos += vertex_attr * vec2(sprite_coords_uni[2], sprite_coords_uni[3]);\n"
"	pos /= vec2(screen_size_uni[0], screen_size_uni[1]);\n"
"	pos = pos * 2.0 - 1.0;\n"
"	pos.y *= -1.0;\n"
"	\n"
"	uv_var = vec2(vertex_attr.x, 1.0 - vertex_attr.y);\n"
"	\n"
"	gl_Position = vec4(pos, 0.0, 1.0);\n"
"}";

const char *_2d_sprite_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2D tex_uni;\n"
"\n"
"in vec2 uv_var;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{\n"
"	color_out = texture(tex_uni, uv_var);\n"
"}";

const char *particle_shader_vert = 
"#version 330\n"
"\n"
"uniform mat4 modelview_matrix_uni;\n"
"\n"
"in vec3 vertex_attr;\n"
"in float size_attr;\n"
"in float rotation_attr;\n"
"in vec4 color_attr;\n"
"\n"
"in float texture_index_attr;\n"
"\n"
"out float size_var;\n"
"out float rotation_var;\n"
"flat out vec4 color_var_g;\n"
"flat out float texture_index_var_g;\n"
"\n"
"void main(void)\n"
"{\n"
"	size_var = size_attr;\n"
"	rotation_var = rotation_attr;\n"
"	color_var_g = color_attr;\n"
"	texture_index_var_g = texture_index_attr;\n"
"	 \n"
"	gl_Position = modelview_matrix_uni * vec4(vertex_attr, 1.0);\n"
"}";

const char *particle_shader_frag = 
"#version 330\n"
"\n"
"uniform sampler2DArray texture_uni;\n"
"\n"
"in vec2 uv_var;\n"
"flat in float texture_index_var_f;\n"
"flat in vec4 color_var_f;\n"
"\n"
"out vec4 color_out;\n"
"\n"
"void main(void)\n"
"{		\n"
"	color_out = color_var_f * texture(texture_uni, vec3(uv_var, float(texture_index_var_f)));\n"
"}";

const char *particle_shader_geom = 
"#version 330\n"
"\n"
"uniform mat4 projection_matrix_uni;\n"
"\n"
"layout(points) in;\n"
"layout(triangle_strip, max_vertices=4) out;\n"
"\n"
"in float size_var[];\n"
"in float rotation_var[];\n"
"flat in float texture_index_var_g[];\n"
"flat in vec4 color_var_g[];\n"
"\n"
"out vec2 uv_var;\n"
"flat out float texture_index_var_f;\n"
"flat out vec4 color_var_f;\n"
"\n"
"void main(void)\n"
"{\n"
"	float size = size_var[0];\n"
"\n"
"	vec2 up = vec2(sin(rotation_var[0]), cos(rotation_var[0]));\n"
"	vec2 right = vec2(up.y, -up.x);\n"
"\n"
"	gl_Position = projection_matrix_uni * (gl_in[0].gl_Position + vec4((-right + up) * size, 0.0, 0.0));\n"
"	uv_var = vec2(0.0, 1.0);\n"
"	texture_index_var_f = texture_index_var_g[0];\n"
"	color_var_f = color_var_g[0];\n"
"	EmitVertex();\n"
"	\n"
"	gl_Position = projection_matrix_uni * (gl_in[0].gl_Position + vec4((-right - up) * size, 0.0, 0.0));\n"
"	uv_var = vec2(0.0, 0.0);\n"
"	texture_index_var_f = texture_index_var_g[0];\n"
"	color_var_f = color_var_g[0];\n"
"	EmitVertex();\n"
"	\n"
"	gl_Position = projection_matrix_uni * (gl_in[0].gl_Position + vec4((right + up) * size, 0.0, 0.0));\n"
"	uv_var = vec2(1.0, 1.0);\n"
"	texture_index_var_f = texture_index_var_g[0];\n"
"	color_var_f = color_var_g[0];\n"
"	EmitVertex();\n"
"	\n"
"	gl_Position = projection_matrix_uni * (gl_in[0].gl_Position + vec4((right - up) * size, 0.0, 0.0));\n"
"	uv_var = vec2(1.0, 0.0);\n"
"	texture_index_var_f = texture_index_var_g[0];\n"
"	color_var_f = color_var_g[0];\n"
"	EmitVertex();\n"
"	\n"
"	EndPrimitive();\n"
"}";

