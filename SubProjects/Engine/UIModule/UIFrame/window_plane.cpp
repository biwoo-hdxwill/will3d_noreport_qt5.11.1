#include "window_plane.h"

WindowPlane::WindowPlane(const QString& title, QWidget *parent)
	: Window(parent) {
	window_title_ = title;
	Initialize();
}

WindowPlane::~WindowPlane() {}

void WindowPlane::Initialize() {
	InitViewMenu();

}

void WindowPlane::AddMaximizeButton()
{
	Window::AddMaximizeButton();
}
