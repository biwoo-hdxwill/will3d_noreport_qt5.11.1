#pragma once
#include <memory>
#include <mutex>

#include <QObject>
#include "W3Enum.h"
#include "common_global.h"

class CWill3D;
class TabContentsWidget;
class QLayout;
class EventSenderCommon;
class GLImplantWidget;
class GLNerveWidget;
class ImplantCollisionRenderer;

class COMMON_EXPORT EventHandleCommon : public QObject {
	Q_OBJECT

public:
	EventHandleCommon();
	~EventHandleCommon();

	EventHandleCommon(const EventHandleCommon&) = delete;
	EventHandleCommon& operator=(const EventHandleCommon&) = delete;

public:
	void EmitSigSetTabSlotLayout(QLayout* layout) const;
	void EmitSigSetMainVolume() const;
	void EmitSigSetSecondVolume() const;
	void EmitSigSetPanoVolume() const;
	void EmitSigClearPanoVolume() const;

	void EmitSigSetTFpreset(const QString& preset) const;
	void EmitSigMoveTFpolygon(const double& value) const;

	void EmitSigUpdateNerve() const;
	void EmitSigUpdateImplant() const;

	void ConnectSigSetTabSlotLayout(TabContentsWidget* receiver, const char* slot) const;

	void ConnectSigSetMainVolume(const CWill3D* receiver, const char* slot) const;
	void ConnectSigSetSecondVolume(const CWill3D* receiver, const char* slot) const;
	void ConnectSigSetPanoVolume(const CWill3D* receiver, const char* slot) const;
	void ConnectSigClearPanoVolume(const CWill3D* receiver, const char* slot) const;

	void ConnectSigSetTFpreset(const CWill3D* receiver, const char* slot) const;
	void ConnectSigMoveTFpolygon(const CWill3D* receiver, const char* slot) const;

	void ConnectSigUpdateNerve(const GLNerveWidget* receiver, const char* slot) const;
	void ConnectSigUpdateImplant(const GLImplantWidget* receiver, const char* slot) const;
	void ConnectSigUpdateNerve(const ImplantCollisionRenderer* receiver, const char* slot) const;
	void ConnectSigUpdateImplant(const ImplantCollisionRenderer* receiver, const char* slot) const;
private:
	std::unique_ptr<EventSenderCommon> sender_;
};
