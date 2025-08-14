#pragma once
/*=========================================================================

File:			class DicomSend
Language:		C++11
Library:		Qt 5.9.9
Author:			JUNG DAE GUN
First date:		2021-03-05
Last modify:	2021-03-05

=========================================================================*/

#include <QString>
#include <QList>

#include "w3dicomio_global.h"

class W3DICOMIO_EXPORT DicomSend
{
public:
	static bool NetworkConnectionCheck(const QString& server_ae_title, const QString& server_ip, const int port);
	static int Do(const QString& server_ip, const int port, const QString& server_ae_title, const QList<QString>& files);

protected:
	DicomSend();
	virtual ~DicomSend();
};

