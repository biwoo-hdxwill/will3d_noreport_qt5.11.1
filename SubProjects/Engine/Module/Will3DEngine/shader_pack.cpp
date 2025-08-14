#include "shader_pack.h"

#include <assert.h>

#include <QDebug>
#include <QDir>

void ShaderPack::GetShaderFilePath(const SHADER& shader_id, QString* vertex_path, QString* fragment_path)
{
	QDir currnet_dir("./");
	QString solution_path = currnet_dir.absolutePath() + "/../../";

	switch (shader_id)
	{
	case SHADER_FRONT_FACE_CUBE:
		*vertex_path = QString(":/volume_raycasting/frontface.vert");
		*fragment_path = QString(":/volume_raycasting/frontface.frag");
		break;
	case SHADER_FRONT_FACE_FINAL:
		*vertex_path = QString(":/volume_raycasting/frontface_final.vert");
		*fragment_path = QString(":/volume_raycasting/frontface_final.frag");
		break;
	case SHADER_BACK_FACE_CUBE:
		*vertex_path = QString(":/volume_raycasting/backface.vert");
		*fragment_path = QString(":/volume_raycasting/backface.frag");
		break;
	case SHADER_RAYCASTING:
		*vertex_path = QString(":/volume_raycasting/raycasting.vert");
#ifndef WILL3D_LIGHT
		*fragment_path = QString(":/volume_raycasting/raycasting.frag");
#else
		*fragment_path = QString(":/volume_raycasting/raycasting_light.frag");
#endif
		break;
	case SHADER_RAY_FIRST_HIT:
		*vertex_path = QString(":/volume_raycasting/firsthit_ray.vert");
		*fragment_path = QString(":/volume_raycasting/firsthit_ray.frag");
		break;
	case SHADER_TEXTURING_SURFACE:
		*vertex_path = QString(":/volume_raycasting/surface_texture.vert");
		*fragment_path = QString(":/volume_raycasting/surface_texture.frag");
		break;
	case SHADER_RENDER_SLICE:
		*vertex_path = QString(":/volume_slice/slice.vert");
		*fragment_path = QString(":/volume_slice/slice.frag");
		break;
	case SHADER_RENDER_SLICE_NERVE:
		*vertex_path = QString(":/volume_slice/slice_nerve.vert");
		*fragment_path = QString(":/volume_slice/slice_nerve.frag");
		break;
	case SHADER_RENDER_SLICE_IMPLANT:
#ifndef _DEBUG
		*vertex_path = QString(":/volume_slice/slice_implant.vert");
		*fragment_path = QString(":/volume_slice/slice_implant.frag");
#else
		*vertex_path = solution_path + QString("Will3D/shader_files/volume_slice/slice_implant.vert");
		*fragment_path = solution_path + QString("Will3D/shader_files/volume_slice/slice_implant.frag");
#endif
		break;
	case SHADER_RENDER_SLICE_IMPLANT_WIRE:
		*vertex_path = QString(":/shader/Test/implant.vert");
		*fragment_path = QString(":/shader/Test/implant.frag");
		break;
	case SHADER_RENDER_IMAGE:
#ifndef _DEBUG
		*vertex_path = QString(":/image/image.vert");
		*fragment_path = QString(":/image/image.frag");
#else
		*vertex_path = solution_path + QString("Will3D/shader_files/image/image.vert");
		*fragment_path = solution_path + QString("Will3D/shader_files/image/image.frag");
#endif
		break;
	case SHADER_RENDER_IMAGE_WINDOWING:
#ifndef _DEBUG
		*vertex_path = QString(":/image/image_windowing.vert");
		*fragment_path = QString(":/image/image_windowing.frag");
#else
		*vertex_path = solution_path + QString("Will3D/shader_files/image/image_windowing.vert");
		*fragment_path = solution_path + QString("Will3D/shader_files/image/image_windowing.frag");
#endif
		break;
	case SHADER_RENDER_SURFACE:
		*vertex_path = QString(":/surface/surface.vert");
		*fragment_path = QString(":/surface/surface.frag");
		break;
	case SHADER_BONEDENSITY:
		*vertex_path = QString(":/surface/bone_density.vert");
		*fragment_path = QString(":/surface/bone_density.frag");
		break;
	case SHADER_PICK_OBJECT:
#ifndef _DEBUG
		*vertex_path = QString(":/surface/pick_with_coord.vert");
		*fragment_path = QString(":/surface/pick_with_coord.frag");
#else
		*vertex_path = solution_path + QString("Will3D/shader_files/surface/pick_with_coord.vert");
		*fragment_path = solution_path + QString("Will3D/shader_files/surface/pick_with_coord.frag");
#endif
		break;
	default:
		assert(false);
		break;
	}
}
