#include "W3VChelper.h"

namespace {
	const float SCENE_TO_GL_NORMALIZE  = 2.f;
	const float VOL_TO_GL_NORMALIZE = 2.f;
}

float CW3GLhelper::scaledSceneToGL(float arg) {
	return arg * SCENE_TO_GL_NORMALIZE;
}

float CW3GLhelper::scaledGLToScene(float arg)
{
	return arg / SCENE_TO_GL_NORMALIZE;
}

float CW3GLhelper::scaledGLtoVol(float arg)
{
	return arg / VOL_TO_GL_NORMALIZE;
}

float CW3GLhelper::scaledVolToGL(float arg)
{
	return arg * VOL_TO_GL_NORMALIZE;
}

QPointF CW3GLhelper::scaledSceneToGL(const QPointF& point)
{
	return QPointF(scaledSceneToGL(point.x()), scaledSceneToGL(point.y()));
}

QPointF CW3GLhelper::scaledGLToScene(const QPointF& point)
{
	return QPointF(scaledGLToScene(point.x()), scaledGLToScene(point.y()));
}

QPointF CW3GLhelper::scaledGLtoVol(const QPointF & point)
{
	return QPointF(scaledGLtoVol(point.x()), scaledGLtoVol(point.y()));
}

QPointF CW3GLhelper::scaledVolToGL(const QPointF & point)
{
	return QPointF(scaledVolToGL(point.x()), scaledVolToGL(point.y()));
}
