#pragma once

#include <QPointF>
class CW3GLhelper
{
public:
	//SceneToGL: Scene좌표계(-0.5 ~ 0.5)을 GL좌표계 (-1 ~ 1)로 변환.
	static float scaledSceneToGL(float arg);
	static QPointF scaledSceneToGL(const QPointF& point);

	static float scaledGLToScene(float arg);
	static QPointF scaledGLToScene(const QPointF& point);


	//GlToVol: GL좌표게(-1 ~ 1)을 볼륨 좌표계(-0.5 ~ 0.5)로 변환.
	static float scaledGLtoVol(float arg);
	static float scaledVolToGL(float arg);

	static QPointF scaledGLtoVol(const QPointF& point);
	static QPointF scaledVolToGL(const QPointF& point);

};
