#include "W3OTFScene.h"
/*=========================================================================

File:			class CW3OTFScene.cpp
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN, Hong Jung
First date:		2015-06-24
Last modify:	2015-12-22

=========================================================================*/
#include <math.h>
#include <fstream>
#include <string>

#include <QDebug>
#include <QElapsedTimer>
#include <qdir.h>
#include <qfiledialog.h>
#include <qvector4d.h>
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>

#include <Engine/Common/Common/color_dialog.h>
#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/define_otf.h"
#include "../../Common/Common/language_pack.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3TF.h"

#include "../../UIModule/UIPrimitive/W3EllipseItem_otf.h"
#include "../../UIModule/UIPrimitive/W3LineItem_otf.h"
#include "../../UIModule/UIPrimitive/W3PolygonItem_otf.h"
#include "../../UIModule/UIPrimitive/W3TextItem_otf.h"

using namespace W3;

namespace
{
	const int LEAST_POLYGON_COUNT = 1;
	auto Comparing = [](QGraphicsEllipseItem *item1, QGraphicsEllipseItem *item2)
	{
		return item1->x() < item2->x();
	};
}  // end of namespace

CW3OTFScene::CW3OTFScene(QWidget *parent) : QGraphicsScene(parent)
{
	tf_colors_.reset(new std::vector<TF_BGRA>);

	tf_.reset(new CW3TF(tf_colors_, m_nOffset));
	ResourceContainer::GetInstance()->SetTfResource(tf_);

	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;

	m_adjustOTF.bright = 0.0f;
	m_adjustOTF.contrast = 1.0f;
	m_adjustOTF.opacity = 1.0f;

	// Menu & Actions.
	m_menu = new QMenu();
	m_addPolygonAct = new QAction(lang::LanguagePack::msg_36(), this);
	m_removePolygonAct = new QAction(lang::LanguagePack::msg_37(), this);
	m_addPointAct = new QAction(lang::LanguagePack::msg_35(), this);
	m_removePointAct = new QAction(lang::LanguagePack::msg_34(), this);
	m_addColorObjectAct = new QAction(lang::LanguagePack::msg_40(), this);
	m_updateColorObjectAct = new QAction(lang::LanguagePack::msg_39(), this);
	m_removeColorObjectAct = new QAction(lang::LanguagePack::msg_38(), this);

	connect(m_addPolygonAct, SIGNAL(triggered()), this, SLOT(slotAddPolygon()));
	connect(m_removePolygonAct, SIGNAL(triggered()), this,
		SLOT(slotRemovePolygon()));
	connect(m_addPointAct, SIGNAL(triggered()), this, SLOT(slotAddPoint()));
	connect(m_removePointAct, SIGNAL(triggered()), this, SLOT(slotRemovePoint()));
	connect(m_addColorObjectAct, SIGNAL(triggered()), this, SLOT(slotAddColor()));
	connect(m_updateColorObjectAct, SIGNAL(triggered()), this,
		SLOT(slotUpdateColor()));
	connect(m_removeColorObjectAct, SIGNAL(triggered()), this,
		SLOT(slotRemoveColor()));
}

CW3OTFScene::~CW3OTFScene()
{
	while (m_listOTFObj.size())
	{
		m_listOTFObj.at(0)->removeFromScene(this);
		auto iter = m_listOTFObj.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_listOTFObj.erase(iter);
	}

	SAFE_DELETE_OBJECT(m_currLine);
	SAFE_DELETE_OBJECT(m_currText);
	SAFE_DELETE_OBJECT(m_mainColorRect);

	SAFE_DELETE_OBJECT(m_menu);
	SAFE_DELETE_OBJECT(m_addPolygonAct);
	SAFE_DELETE_OBJECT(m_removePolygonAct);
	SAFE_DELETE_OBJECT(m_addPointAct);
	SAFE_DELETE_OBJECT(m_removePointAct);
	SAFE_DELETE_OBJECT(m_addColorObjectAct);
	SAFE_DELETE_OBJECT(m_updateColorObjectAct);
	SAFE_DELETE_OBJECT(m_removeColorObjectAct);
}

void CW3OTFScene::reset()
{
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;

	m_isPressed = false;
	m_isOnPolygon = false;
	m_nOffset = 0;

	m_selectedPolygonIdx = 0;
	m_activatePolygonIdx = -1;

	m_TFmin = 0;
	m_TFmax = 0;

	m_adjustOTF.bright = 0.0f;
	m_adjustOTF.contrast = 1.0f;
	m_adjustOTF.opacity = 1.0f;

	new_soft_tissue_min_from_generate_face_dialog_ = -1.0f;
}

void CW3OTFScene::movePolygonAtMin(float minValue)
{
	if (!tf_) return;

	int currentMin = tf_->min_value();
	int step = static_cast<int>(minValue) - currentMin;

	for (auto &elem : m_listOTFObj)
	{
		elem->movePolygon(step, m_nOffset);
	}

	updateColor();
}

void CW3OTFScene::setPreset(const QString &str)
{
	for (auto &elem : m_listTFPaths)
	{
		if (elem.contains(str))
		{
			m_strCurPreset = str;
			this->loadPresets(elem);
			emit sigRenderCompleted();
			break;
		}
	}
}

void CW3OTFScene::setXRayTF(float max, float min)
{
	//m_strCurPreset = common::otf_preset::XRAY;

	// DELETE EXISTING RESOURCES.
	for (int i = 0; i < m_listOTFObj.size(); i++)
	{
		CW3OTFPolygon *temp = m_listOTFObj.at(i);

		// thyoo 160908: delete color objects
		auto colorObjects = temp->getColorObject();
		for (int k = 0; k < colorObjects->size(); k++)
			this->removeItem(colorObjects->at(k));

		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();

	// READ.
	m_bShade = false;
	emit sigShadeOnSwitch(m_bShade);

	// set UI color components.
	QPen temppen(Qt::white);
	temppen.setCosmetic(true);
	QBrush brush(Qt::green);
	QColor color(Qt::green);
	color.setAlphaF(0.0);
	brush.setColor(color);
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	font.setWeight(QFont::Bold);

	int polyNum = 1;
	for (int i = 0; i < polyNum; i++)  // READ POLYGON
	{
		CW3OTFPolygon *tmpObj = new CW3OTFPolygon();

		// read # of points.
		int numPts = 3;
		int numColors = 2;
		QPolygonF poly;
		std::vector<QPointF> points;
		points.push_back(QPointF(min, 158));
		points.push_back(QPointF(max, 25));
		points.push_back(QPointF(max, 158));

		for (int j = 0; j < numPts; j++)
		{
			QPointF pt(points[j].x() - m_nOffset, points[j].y());
			poly.append(pt);
			tmpObj->getPointList().append(new CW3EllipseItem_otf(pt));
			QString str(
				"[" +
				QString().setNum(static_cast<float>(pt.x()) + m_nOffset, 'f', 0) +
				"]");
			tmpObj->getTextList().append(
				new CW3TextItem_otf(QPointF(pt.x(), pt.y() - 25),
					static_cast<float>(pt.x() + m_nOffset)));
		}
		tmpObj->getPolygon()->setPolygon(poly);
		tmpObj->getPolygon()->setPen(QPen(Qt::transparent));
		tmpObj->getPolygon()->setBrush(brush);
		QPen pen = tmpObj->getPolygon()->pen();
		pen.setColor(Qt::red);
		tmpObj->getPolygon()->setPen(pen);

		// add lines.
		QList<CW3EllipseItem_otf *> &pointList = tmpObj->getPointList();
		QList<CW3LineItem_otf *> &lineList = tmpObj->getLineList();
		for (int i = 1; i < pointList.size() - 2; i++)
			lineList.append(new CW3LineItem_otf(pointList.at(i)->pos(),
				pointList.at(i + 1)->pos()));

		// ADD.
		tmpObj->addToScene(this);

		// setColorObject
		std::vector<QVector4D> colors;
		colors.push_back(QVector4D(min, 0, 0, 0));
		colors.push_back(QVector4D(max, 255, 255, 255));

		for (int i = 0; i < numColors; i++)  // READ COLOR.
		{
			QColor C(colors[i].y(), colors[i].z(), colors[i].w());

			QGraphicsEllipseItem *temp;
			QPen pen(Qt::white);
			if (C.red() > 125 && C.green() > 125 && C.blue() > 125)
			{
				pen.setColor(Qt::black);
			}
			temp = addEllipse(-5, -5, 10, 10, pen, QBrush(C));
			temp->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
			temp->setPos(colors[i].x() - m_nOffset, kTFHeight - kTFMargin / 2 + 5);

			m_pgColorObject = tmpObj->getColorObject();
			if (m_pgColorObject->size() != 0)
			{
				int idx = -1;
				for (int i = 0; i < m_pgColorObject->size(); i++)
				{
					if (m_pgColorObject->at(i)->pos().x() >= temp->pos().x())
					{
						m_pgColorObject->insert(i, temp);
						idx = i;
						break;
					}
				}
				if (idx == -1) m_pgColorObject->append(temp);
			}
			else
			{
				m_pgColorObject->append(temp);
			}
		}

		// update to m_listOTFObj <CW3OTFPolygon>
		m_listOTFObj.append(tmpObj);
	}
	// selectPolygon(m_listOTFObj.size() - 1);
	selectPolygon(0);

	setDefaultGeometry();
	updateColor();
}

bool CW3OTFScene::setAdjustOTF(const AdjustOTF &val)
{
	if (tf_->size() == 0)
	{
		common::Logger::instance()->Print(
			common::LogType::ERR, "CW3OTFScene::setAdjustOTF: TF is nullptr.");
		return false;
	}

	m_adjustOTF = val;
	updateColor();

	return true;
}

void CW3OTFScene::Export(const QString &path)
{
	std::ofstream outFile;
	outFile.open(path.toStdString(), std::ios::out);
	if (!outFile.is_open()) return;

	// WRITE.
	int shadeNum = (m_bShade) ? 1 : 0;
	outFile << m_listOTFObj.size() << " " << shadeNum << "\n";
	for (int i = 0; i < m_listOTFObj.size(); i++)
	{
		CW3OTFPolygon *tmp = m_listOTFObj.at(i);
		outFile << tmp->getPointList().size() << " "
			<< tmp->getColorObject()->size() << "\n";

		for (int j = 0; j < tmp->getPointList().size(); j++)
		{
			outFile << tmp->getPointList().at(j)->pos().x() + m_nOffset << " "
				<< tmp->getPointList().at(j)->pos().y() << "\n";
		}

		for (int j = 0; j < tmp->getColorObject()->size(); j++)
		{
			QColor color = tmp->getColorObject()->at(j)->brush().color();
			outFile << tmp->getColorObject()->at(j)->pos().x() + m_nOffset << " "
				<< color.red() << " " << color.green() << " " << color.blue()
				<< "\n";
		}
	}

	outFile.close();
	// clearSelection();

	m_listTFPaths.clear();
	m_listTFImg.clear();
	m_listTFImgPaths.clear();
	QDir directory("./tfpresets");

	if (!directory.exists())
		common::Logger::instance()->Print(common::LogType::ERR,
			"CW3OTFScene::slotExport: No directory.");

	QFileInfoList listInfo = directory.entryInfoList(QDir::Files, QDir::Name);
	for (const auto &i : listInfo)
	{
		if (i.suffix().compare("jpg") == 0)
		{
			QLabel *label = new QLabel();
			label->setPixmap(
				QPixmap::fromImage(QImage(i.filePath()).scaled(180, 180)));
			m_listTFImg.push_back(label);
			m_listTFImgPaths.push_back(i.filePath());
		}
		else if (i.suffix().compare("tf") == 0)
			m_listTFPaths.push_back(i.filePath());
	}
}

/* draw Histogram & initialize */
void CW3OTFScene::initOTF(int *histogram, int histoSize, float slope,
	int offset)
{
	qDebug() << "start CW3OTFScene::initOTF";

	QElapsedTimer timer;
	timer.start();

	m_TFmin = 0;
	m_TFmax = 0;

	int tf_size = histoSize/* * slope*/;

	slope_ = slope;
	m_nOffset = offset;
	this->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
	this->setSceneRect(0, 0, tf_size, kTFHeight);

	tf_colors_->clear();
	tf_colors_->resize(tf_size);

	tf_.reset(new CW3TF(tf_colors_, m_nOffset));
	ResourceContainer::GetInstance()->SetTfResource(tf_);

	QPen mainPen(Qt::transparent);
	mainPen.setWidth(0);
	SAFE_DELETE_OBJECT(m_mainColorRect);
	m_mainColorRect = addRect(0, kTFHeight - kTFMargin + 10, tf_size,
		kTFMargin / 2 + 2, mainPen, QBrush(Qt::white));
	m_mainColorRect->setZValue(0.0f);

	// polygon item
	QPolygonF histo;
	histo.append(QPointF(0.0f, kTFHeight - kTFMargin));
	float eps_float = std::numeric_limits<float>::epsilon();

#if 1
	std::vector<int> smooth_histo;
#if 1
	Common::SmoothingHisto(histogram, histoSize, smooth_histo, 150, 5, false);
#else
	smooth_histo.insert(smooth_histo.begin(), &histogram[0], &histogram[histoSize - 1]);
#endif
	// find max value
	int histo_max = 0;
	for (const auto &histo : smooth_histo)
	{
		histo_max = std::max(histo_max, histo);
	}

	if (histo_max > 0)
	{
		for (int i = 0; i < smooth_histo.size(); ++i)
		{
			float temp = static_cast<float>(smooth_histo[i]) / histo_max;

			if (temp <= eps_float)
			{
				continue;
			}

			int histogram_y_pos = kTFHeight - kTFMargin - (kTFHeight - 2 * kTFMargin) * temp;
			//qDebug() << i << ":" << smooth_histo[i] << "/" << temp << "/" << histogram_y_pos;
			histo.append(QPointF(i/* * slope*/, histogram_y_pos));
		}
	}
	else
	{
		printf("WARNING: max value of histogram is zero or negative!\n");
	}
#else
	int histo_max = 0;
	for (int i = 1; i < tf_size; ++i)
	{
#if 1
		histo_max = std::max(histo_max, histogram[i]);
#else
		int val = histogram[i];
		if (histo_max < val)
		{
			histo_max = val;
			qDebug() << i << ":" << val;
		}
#endif
	}
	histogram[0] = histo_max;

	if (histo_max > 0)
	{
		for (int i = 0; i < tf_size; ++i)
		{
			float temp = static_cast<float>(histogram[i]) / histo_max;

			if (temp <= eps_float)
			{
				continue;
			}

			int histogram_y_pos = kTFHeight - kTFMargin - (kTFHeight - 2 * kTFMargin) * temp;
			//qDebug() << i << ":" << histogram[i] << "/" << histo_max << "=" << temp << "," << histogram_y_pos;
			histo.append(QPointF(i, histogram_y_pos));
		}
	}
	else
	{
		printf("WARNING: max value of histogram is zero or negative!\n");
	}
#endif

	histo.append(QPointF(tf_size - 1, kTFHeight - kTFMargin));

	// item -> pixmap
	QPixmap pixmap(tf_size, kTFHeight - kTFMargin);
	pixmap.fill(Qt::black);
	QPainter painter(&pixmap);
	painter.setPen(QPen(Qt::gray));
	painter.setBrush(QBrush(Qt::gray));
	painter.drawPolygon(histo);
	painter.end();

	int maxPixmapAxisSize = 32767;
	QPixmap first = pixmap;
	if (tf_size > maxPixmapAxisSize)
	{
		first =
			pixmap.copy(QRect(0, 0, maxPixmapAxisSize - 1, kTFHeight - kTFMargin));
		QPixmap second =
			pixmap.copy(QRect(maxPixmapAxisSize, 0, tf_size - maxPixmapAxisSize - 1,
				kTFHeight - kTFMargin));

		QGraphicsPixmapItem *secondPixmapItem = this->addPixmap(second);
		secondPixmapItem->setPos(maxPixmapAxisSize, secondPixmapItem->y());
	}

	this->addPixmap(first);

	// DELETE EXISTING RESOURCES.
	for (int i = 0; i < m_listOTFObj.size(); i++)
	{
		CW3OTFPolygon *temp = m_listOTFObj.at(i);
		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();

	SAFE_DELETE_OBJECT(m_currLine);
	m_currLine = addLine(0, 0, 0, sceneRect().height(), QPen(Qt::white));
	m_currLine->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	font.setWeight(QFont::Bold);

	SAFE_DELETE_OBJECT(m_currText);
	m_currText = addText(QString().setNum(float(offset), 'f', 0), font);
	m_currText->setDefaultTextColor(Qt::white);
	m_currText->setPos(0, (sceneRect().height() / 2));
	m_currText->setFlag(QGraphicsRectItem::ItemIgnoresTransformations, true);
	m_currText->setZValue(5.0f);

	qDebug() << "end CW3OTFScene::initOTF :" << timer.elapsed() << "ms";
}

void CW3OTFScene::setThreshold(int thdAir, int thdTissue, int thdBone)
{
	for (int idx = 0; idx < m_thdLine.size(); ++idx)
	{
		removeItem(m_thdLine.at(idx));
		SAFE_DELETE_OBJECT(m_thdLine.at(idx));
	}
	m_thdLine.clear();

	QGraphicsLineItem* line_air = addLine(0, 0, 0, sceneRect().height(), QPen(Qt::white));
	line_air->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
	//line->setZValue(50);
	m_thdLine.push_back(line_air);

	QGraphicsLineItem* line_tissue = addLine(0, 0, 0, sceneRect().height(), QPen(Qt::red));
	line_tissue->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
	//line->setZValue(50);
	m_thdLine.push_back(line_tissue);

	QGraphicsLineItem* line_bone = addLine(0, 0, 0, sceneRect().height(), QPen(Qt::blue));
	line_bone->setFlag(QGraphicsLineItem::ItemIgnoresTransformations, true);
	//line->setZValue(50);
	m_thdLine.push_back(line_bone);

	m_thdLine[AIR_TISSUE]->setPos(thdAir, 0);
	m_thdLine[TISSUE_BONE]->setPos(thdTissue, 0);
	m_thdLine[BONE_TEETH]->setPos(thdBone, 0);

#if 0
	m_thdLine[AIR_TISSUE]->setVisible(false);
	m_thdLine[TISSUE_BONE]->setVisible(false);
	m_thdLine[BONE_TEETH]->setVisible(false);
#endif

	initLoadPresets();
}

void CW3OTFScene::deactiveItems()
{
	if (m_currLine) m_currLine->setVisible(false);

	if (m_currText) m_currText->setVisible(false);

	setDefaultGeometry();
}

/*
		Read all presets.
		 - load all .bmp files,
		 - save all .tf file paths
*/
void CW3OTFScene::initLoadPresets(void)
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	QString default_otf_preset = settings.value("OTF/default", common::otf_preset::BONE).toString();
	QStringList split = default_otf_preset.split('_');
	int remove_index = default_otf_preset.indexOf('_');

	if (split.size() >= 2)
	{
		default_otf_preset = default_otf_preset.right(default_otf_preset.size() - 1 - remove_index);
	}
	else
	{
		default_otf_preset = common::otf_preset::BONE;
	}

	m_listTFPaths.clear();
	m_listTFImg.clear();
	m_listTFImgPaths.clear();

	QDir directory("./tfpresets");
	if (!directory.exists())
		common::Logger::instance()->Print(
			common::LogType::ERR, "CW3OTFScene::initLoadPresets: No directory.");

	QFileInfoList file_info_list = directory.entryInfoList(QDir::Files, QDir::Name);
	for (const auto &i : file_info_list)
	{
		if (i.suffix().compare("jpg") == 0)
		{
			QLabel* label = new QLabel();
			label->setPixmap(QPixmap::fromImage(QImage(i.filePath()).scaled(180, 180)));
			m_listTFImg.push_back(label);
			m_listTFImgPaths.push_back(i.filePath());
		}
		else if (i.suffix().compare("tf") == 0)
		{
			m_listTFPaths.push_back(i.filePath());
		}
	}

	QString default_otf_path;
	for (const auto &i : m_listTFPaths)
	{
		if (i.contains(default_otf_preset))
		{
			m_strCurPreset = default_otf_preset;
			default_otf_path = i;
			break;
		}
	}

	if (!default_otf_path.isEmpty())
	{
		loadPresets(default_otf_path);
	}
}

void CW3OTFScene::loadPresets(const QString &path)
{
	std::ifstream inFile;
	inFile.open(path.toStdString(), std::ios::in);
	if (!inFile.is_open())
	{
		CW3MessageBox msg_box("Will3D", lang::LanguagePack::msg_23(), CW3MessageBox::Critical);
		msg_box.exec();
		return;
	}

	// DELETE EXISTING RESOURCES.
	for (int i = 0; i < m_listOTFObj.size(); i++)
	{
		CW3OTFPolygon *temp = m_listOTFObj.at(i);

		// thyoo 160908: delete color objects
		auto colorObjects = temp->getColorObject();
		for (int k = 0; k < colorObjects->size(); k++)
			this->removeItem(colorObjects->at(k));

		temp->removeFromScene(this);
		delete (m_listOTFObj.at(i));
		m_listOTFObj.replace(i, nullptr);
	}
	m_listOTFObj.clear();
	m_colorList.clear();

	// READ.
	int polyNum = 0;
	int shade = 0;
	inFile >> polyNum;
	inFile >> shade;

	m_bShade = (shade == 1) ? true : false;
	emit sigShadeOnSwitch(m_bShade);

	// set UI color components.
	QPen temppen(Qt::white);
	temppen.setCosmetic(true);
	QBrush brush(Qt::green);
	QColor color(Qt::green);
	color.setAlphaF(0.0);
	brush.setColor(color);
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	font.setWeight(QFont::Bold);

	// READ POLYGON
	for (int i = 0; i < polyNum; i++)
	{
		CW3OTFPolygon *tmpObj = new CW3OTFPolygon();

		// read # of points.
		int numPts = 0;
		int numColors = 0;
		inFile >> numPts;
		inFile >> numColors;

		QPolygonF poly;
		for (int j = 0; j < numPts; j++)
		{
			float x = 0.0f;
			float y = 0.0f;
			inFile >> x >> y;
			QPointF pt(x - m_nOffset, y);
			poly.append(pt);
			tmpObj->getPointList().append(new CW3EllipseItem_otf(pt));
			QString str(
				"[" +
				QString().setNum(static_cast<float>(pt.x()) + m_nOffset, 'f', 0) +
				"]");
			tmpObj->getTextList().append(
				new CW3TextItem_otf(QPointF(pt.x(), pt.y() - 25),
					static_cast<float>(pt.x() + m_nOffset)));
		}
		tmpObj->getPolygon()->setPolygon(poly);
		tmpObj->getPolygon()->setPen(QPen(Qt::transparent));
		tmpObj->getPolygon()->setBrush(brush);
		QPen pen = tmpObj->getPolygon()->pen();
		pen.setColor(Qt::red);
		tmpObj->getPolygon()->setPen(pen);

		// add lines.
		QList<CW3EllipseItem_otf *> &pointList = tmpObj->getPointList();
		QList<CW3LineItem_otf *> &lineList = tmpObj->getLineList();
		for (int j = 1; j < pointList.size() - 2; j++)
			lineList.append(new CW3LineItem_otf(pointList.at(j)->pos(),
				pointList.at(j + 1)->pos()));

		// ADD.
		tmpObj->addToScene(this);

		// READ COLOR.
		for (int j = 0; j < numColors; j++)
		{
			float posX = 0.0f;
			int R = 0, G = 0, B = 0;
			inFile >> posX >> R >> G >> B;
			QColor C(R, G, B);

			QGraphicsEllipseItem *temp;
			QPen pen(Qt::white);
			if (C.red() > 125 && C.green() > 125 && C.blue() > 125)
			{
				pen.setColor(Qt::black);
			}
			temp = addEllipse(-5, -5, 10, 10, pen, QBrush(C));
			temp->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
			temp->setPos(posX - m_nOffset, kTFHeight - kTFMargin / 2 + 5);

			m_pgColorObject = tmpObj->getColorObject();
			if (m_pgColorObject->size() != 0)
			{
				int idx = -1;
				for (int k = 0; k < m_pgColorObject->size(); k++)
				{
					if (m_pgColorObject->at(k)->pos().x() >= temp->pos().x())
					{
						m_pgColorObject->insert(k, temp);
						idx = k;
						break;
					}
				}
				if (idx == -1) m_pgColorObject->append(temp);
			}
			else
			{
				m_pgColorObject->append(temp);
			}
		}

		m_listOTFObj.append(tmpObj);
	}

	inFile.close();

	if (m_listOTFObj.size() < 1)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "The tf file is corrupted : " + QString::number(m_listOTFObj.size()).toStdString());
#if 1
		CW3MessageBox msg_box("Will3D", "The tf file is corrupted(object size < 1).", CW3MessageBox::Critical);
		msg_box.exec();
		return;
#endif
	}

	for (int i = 0; i < m_listOTFObj.size(); ++i)
	{
		if (m_listOTFObj[i]->getPointList().size() < 3)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "The tf file is corrupted : " + QString::number(m_listOTFObj[i]->getPointList().size()).toStdString());
#if 1
			CW3MessageBox msg_box("Will3D", "The tf file is corrupted(point size < 3).", CW3MessageBox::Critical);
			msg_box.exec();
			return;
#endif
		}
	}

	// use bone, tissue threshold
	if (path.contains(common::otf_preset::BONE, Qt::CaseInsensitive))
	{
#if 0
		if (m_listOTFObj.size() != 2 ||
			m_listOTFObj[0]->getColorObject()->size() != 5 ||
			m_listOTFObj[1]->getColorObject()->size() != 5)
		{
			CW3MessageBox msg_box("Will3D", "The tf file is corrupted.",
				CW3MessageBox::Critical);
			msg_box.exec();
			return;
		}
#endif

#if 0
		const auto &colorList = m_listOTFObj[0]->getColorObject();
		float threshold = m_thdLine[TISSUE_BONE]->pos().x() + additional_offset_;
		float colorPos = colorList->at(1)->pos().x();

		for (int i = 0; i < m_listOTFObj.size(); ++i)
		{
			m_listOTFObj[i]->movePolygon(threshold - colorPos, m_nOffset);
		}
#else
		float threshold = m_thdLine[TISSUE_BONE]->pos().x() + additional_offset_;

		for (int i = 0; i < m_listOTFObj.size(); ++i)
		{
			QList<CW3EllipseItem_otf*> point_list = m_listOTFObj[i]->getPointList();
			float first_point_x = point_list.at(0)->x();
			m_listOTFObj[i]->movePolygon(threshold - first_point_x, m_nOffset);
		}
#endif
	}
	else if (path.contains(common::otf_preset::TEETH, Qt::CaseInsensitive))
	{
#if 1
		if (m_listOTFObj.size() != 2 ||
			m_listOTFObj[1]->getPointList().size() != 4 ||
			m_listOTFObj[0]->getPointList().size() != 4)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "The tf file is corrupted(size) : " + QString::number(m_listOTFObj.size()).toStdString());
			common::Logger::instance()->Print(common::LogType::ERR, "The tf file is corrupted(0) : " + QString::number(m_listOTFObj[0]->getPointList().size()).toStdString());
			common::Logger::instance()->Print(common::LogType::ERR, "The tf file is corrupted(1) : " + QString::number(m_listOTFObj[1]->getPointList().size()).toStdString());
#if 1
			CW3MessageBox msg_box("Will3D", "The tf file is corrupted(TEETH).", CW3MessageBox::Critical);
			msg_box.exec();
			return;
#endif
		}
#endif

		const auto &bonePolyEll = m_listOTFObj[1]->getPointList();
		const auto &teethPolyEll = m_listOTFObj[0]->getPointList();

		float bone_threshold = m_thdLine[TISSUE_BONE]->pos().x() + additional_offset_;
		float theeth_threshold = m_thdLine[BONE_TEETH]->pos().x() + additional_offset_;

#if 0
		m_listOTFObj[0]->movePolygon(
			theeth_threshold -
			((teethPolyEll[1]->pos().x() - teethPolyEll[0]->pos().x()) * 0.5f +
				teethPolyEll[0]->pos().x()),
			m_nOffset);
#else
		m_listOTFObj[0]->movePolygon(theeth_threshold - teethPolyEll[0]->pos().x(), m_nOffset);
#endif
		m_listOTFObj[1]->movePolygon(bone_threshold - bonePolyEll[0]->pos().x(), m_nOffset);
	}
	else if (path.contains(common::otf_preset::SOFTTISSUE1, Qt::CaseInsensitive))
	{
#if 0
		if (m_listOTFObj.size() != 1 ||
			m_listOTFObj[0]->getPointList().size() != 4)
		{
			CW3MessageBox msg_box("Will3D", "The tf file is corrupted.",
				CW3MessageBox::Critical);
			msg_box.exec();
			return;
		}
#endif

		const auto &pointList = m_listOTFObj[0]->getPointList();
		float threshold = m_thdLine[AIR_TISSUE]->pos().x() + additional_offset_;
		if (new_soft_tissue_min_from_generate_face_dialog_ > 0.0f)
		{
			threshold = new_soft_tissue_min_from_generate_face_dialog_;
		}
#if 0
		m_listOTFObj[0]->movePolygon(threshold - pointList[3]->pos().x(), m_nOffset);
#else
		m_listOTFObj[0]->movePolygon(threshold - pointList[0]->pos().x(), m_nOffset);
#endif
	}
	else if (path.contains(common::otf_preset::SOFTTISSUE2, Qt::CaseInsensitive))
	{
#if 0
		if (m_listOTFObj.size() != 1 ||
			m_listOTFObj[0]->getPointList().size() != 4)
		{
			CW3MessageBox msg_box("Will3D", "The tf file is corrupted.",
				CW3MessageBox::Critical);
			msg_box.exec();
			return;
		}
#endif

		const auto &pointList = m_listOTFObj[0]->getPointList();
		float threshold = m_thdLine[AIR_TISSUE]->pos().x() + additional_offset_;
		if (new_soft_tissue_min_from_generate_face_dialog_ > 0.0f)
		{
			threshold = new_soft_tissue_min_from_generate_face_dialog_;
		}

		m_listOTFObj[0]->movePolygon(threshold - pointList[0]->pos().x(), m_nOffset);
	}

	selectPolygon(0);
	setDefaultGeometry();
	updateColor();
}

void CW3OTFScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_listOTFObj.isEmpty()) return;

	QGraphicsScene::mouseMoveEvent(event);
	QPointF pt = event->scenePos();

	if (!m_isPressed)
	{  // mouse is just moving. (not pressed)
// mouse tracking histogram guide line
		m_currLine->setVisible(true);
		m_currLine->setPos(pt.x(), 0);

		QString str(QString().setNum(pt.x() + m_nOffset, 'f', 0));
		m_currText->setVisible(true);
		m_currText->setPlainText(str);
		m_currText->moveBy(pt.x() - m_currText->pos().x(), 0);

		// [First] check if mouse is On Points.
		for (int otfIdx = (int)m_listOTFObj.size() - 1; otfIdx >= 0; otfIdx--)
		{
			CW3OTFPolygon *temp = m_listOTFObj.at(otfIdx);
			for (int i = 0; i < temp->getPointList().size(); i++)
			{
				if (temp->getPointList().at(i)->contains(pt))
				{
					//// deactivate previous selected polygons.
					for (const auto &i : m_listOTFObj) i->deactivateAll();
					temp->getPointList().at(i)->activate();
					// set color polygon.
					QBrush brush = temp->getPolygon()->brush();
					QColor color = brush.color();
					color.setAlphaF(0.3);
					brush.setColor(color);
					temp->getPolygon()->setBrush(brush);
					m_currElem._which = COMPONENT::POINT;
					m_currElem._iIdx = i;
					m_currElem._iPolyIdx = otfIdx;
					m_isOnPolygon = true;
					return;
				}
			}
		}
		// [Second] check if mouse is On Lines.
		for (int otfIdx = (int)m_listOTFObj.size() - 1; otfIdx >= 0; otfIdx--)
		{
			CW3OTFPolygon *temp = m_listOTFObj.at(otfIdx);
			for (int i = 0; i < temp->getLineList().size(); i++)
			{
				// check if near the line.
				if (temp->getLineList().at(i)->contains(pt))
				{
					// deactivate previous selected polygons.
					for (const auto &i : m_listOTFObj) i->deactivateAll();
					temp->getLineList().at(i)->activate();
					// set color polygon.
					QBrush brush = temp->getPolygon()->brush();
					QColor color = brush.color();
					color.setAlphaF(0.3);
					brush.setColor(color);
					temp->getPolygon()->setBrush(brush);
					m_currElem._which = COMPONENT::LINE;
					m_currElem._iIdx = i;
					m_currElem._iPolyIdx = otfIdx;
					m_isOnPolygon = true;
					return;
				}
			}
		}
		// [Third] check if mouse is On Polygon.
		setDefaultGeometry();

		m_activatePolygonIdx = -1;
		for (int otfIdx = 0; otfIdx < m_listOTFObj.size(); otfIdx++)
		{
			CW3OTFPolygon *temp = m_listOTFObj.at(otfIdx);
			if (temp->getPolygon()->contains(pt))
			{
				m_currElem._which = COMPONENT::POLYGON;
				m_currElem._iIdx = -1;
				m_currElem._iPolyIdx = otfIdx;
				m_isOnPolygon = true;

				if (m_activatePolygonIdx < otfIdx) m_activatePolygonIdx = otfIdx;
			}
		}
		if (m_activatePolygonIdx >= 0)
		{
			// set polygon activate.
			m_listOTFObj.at(m_activatePolygonIdx)->getPolygon()->activate();
			return;
		}

		// [Fourth] check if mouse is On Color Object.
		if (m_pgColorObject == nullptr) return;

		for (int i = 0; i < m_pgColorObject->size(); i++)
		{
			QGraphicsView *view = (QGraphicsView *)this->parent();

			QPointF objScenePos = m_pgColorObject->at(i)->scenePos();
			QPoint ptView = view->mapFromScene(objScenePos);
			QRect rectView(ptView - QPoint(6, 6), ptView + QPoint(6, 6));
			QRectF rectSceneAfter(view->mapToScene(rectView.topLeft()),
				view->mapToScene(rectView.bottomRight()));

			if (pt.x() >= rectSceneAfter.left() && rectSceneAfter.right() >= pt.x() &&
				pt.y() >= rectSceneAfter.top() && rectSceneAfter.bottom() >= pt.y())
			{
				setDefaultGeometry();
				m_pgColorObject->at(i)->setRect(
					m_pgColorObject->at(i)->rect().x() * 2.0f,
					m_pgColorObject->at(i)->rect().y() * 2.0f,
					m_pgColorObject->at(i)->rect().width() * 2.0f,
					m_pgColorObject->at(i)->rect().height() * 2.0f);
				m_currElem._which = COMPONENT::COLOR;
				m_currElem._iIdx = i;
				m_currElem._iPolyIdx = -1;
				m_isOnPolygon = true;
				return;
			}
		}
		// It is background.
		m_currElem._which = COMPONENT::NONE;
		m_currElem._iIdx = -1;
		m_currElem._iPolyIdx = -1;
		m_isOnPolygon = false;
	}
	else
	{  // mouse is dragging. (pressed)
		CW3OTFPolygon *temp;
		if (m_currElem._which == COMPONENT::POLYGON)
		{
			temp = m_listOTFObj.at(m_selectedPolygonIdx);
			temp->movePolygon(pt.x() - m_currPoint.x(), m_nOffset);
			m_currPoint = pt;
		}
		else if (m_currElem._which == COMPONENT::POINT)
		{
			temp = m_listOTFObj.at(m_currElem._iPolyIdx);
			int idx = m_currElem._iIdx;
			if (idx == -1) return;

			// move constraints.
			if (idx != (int)temp->getPointList().size() - 1 &&
				idx != 0)
			{  // point is not end-points.
// Y-axis constraints.
				if (pt.y() <= kTFMargin)
				{  // if point is tend to go upper-limit.
					pt.setY(kTFMargin);
					m_currPoint.setY(temp->getPointList().at(idx)->y());
				}
				else if (pt.y() >=
					kTFHeight -
					kTFMargin)
				{  // if point is tend to go lower-limit.
					pt.setY(kTFHeight - kTFMargin);
					m_currPoint.setY(temp->getPointList().at(idx)->y());
				}
				// X-axis constraints.
				if (pt.x() >= temp->getPointList()
					.at(idx + 1)
					->x())
				{  // if point is tend to go right-limit.
					pt.setX(temp->getPointList().at(idx + 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() <= temp->getPointList()
					.at(idx - 1)
					->x())
				{  // if point is tend to go left-limit.
					pt.setX(temp->getPointList().at(idx - 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			else if (idx == 0)
			{  // point is start-end point.
// Y-axis constraints.
				pt.setY(kTFHeight - kTFMargin);
				m_currPoint.setY(kTFHeight - kTFMargin);
				// X-axis constraints.
				if (pt.x() >= temp->getPointList().at(idx + 1)->x())
				{
					pt.setX(temp->getPointList().at(idx + 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() <= 0)
				{
					pt.setX(0);
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			else if (idx == (int)temp->getPointList().size() -
				1)
			{  // point is end-end point.
// Y-axis constraints.
				pt.setY(kTFHeight - kTFMargin);
				m_currPoint.setY(kTFHeight - kTFMargin);
				// X-axis constraints.
				if (pt.x() <= temp->getPointList().at(idx - 1)->x())
				{
					pt.setX(temp->getPointList().at(idx - 1)->x());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
				else if (pt.x() > tf_->size())
				{
					pt.setX(tf_->size());
					m_currPoint.setX(temp->getPointList().at(idx)->x());
				}
			}
			// actual point's movement.
			temp->movePoint(idx, pt.x() - m_currPoint.x(), pt.y() - m_currPoint.y(),
				m_nOffset);
			m_currPoint = pt;

			// display Alpha Value.
			QString tmpStr(
				"  [" +
				QString().setNum((kTFHeight - (int)m_currPoint.y() - kTFMargin) /
				(float)(kTFHeight - 2 * kTFMargin) * 100.0f,
					'f', 0) +
				"%]");
			m_currText->setPlainText(tmpStr);
			m_currText->setPos(m_currPoint.x() - 65, m_currPoint.y() - 20);
			m_currText->setVisible(true);
		}
		else if (m_currElem._which == COMPONENT::LINE)
		{
			temp = m_listOTFObj.at(m_currElem._iPolyIdx);
			int idx = m_currElem._iIdx;
			if (idx == -1) return;
			int ptIdx1 = idx + 1;
			int ptIdx2 = idx + 2;
			int dx = pt.x() - m_currPoint.x();
			int dy = pt.y() - m_currPoint.y();
			// move constraints. (Movement algorithm)
			CW3EllipseItem_otf *point1 = temp->getPointList().at(ptIdx1);
			CW3EllipseItem_otf *point2 = temp->getPointList().at(ptIdx2);
			// X-axis constraints.
			if (point1->x() + dx < temp->getPointList()
				.at(ptIdx1 - 1)
				->x())
			{  // trying to go over left-limit.
				dx = temp->getPointList().at(ptIdx1 - 1)->x() - point1->x();
				pt.setX(m_currPoint.x() + dx);
			}
			else if (temp->getPointList().at(ptIdx2 + 1)->x() <
				point2->x() + dx)
			{  // trying to go over right-limit.
				dx = temp->getPointList().at(ptIdx2 + 1)->x() - point2->x();
				pt.setX(m_currPoint.x() + dx);
			}
			// Y-axis constraints.
			// for Point #1.
			if (point1->y() + dy <= kTFMargin)
			{  // trying to go over upper-limit.
				dy = kTFMargin - point1->y();
				pt.setY(m_currPoint.y() + dy);
			}
			else if (point1->y() + dy >=
				kTFHeight - kTFMargin)
			{  // trying to go over lower-limit.
				dy = kTFHeight - kTFMargin - point1->y();
				pt.setY(m_currPoint.y() + dy);
			}
			// for Point #2.
			if (point2->y() + dy <= kTFMargin)
			{  // trying to go over upper-limit.
				dy = kTFMargin - point2->y();
				pt.setY(m_currPoint.y() + dy);
			}
			else if (point2->y() + dy >=
				kTFHeight - kTFMargin)
			{  // trying to go over lower-limit.
				dy = kTFHeight - kTFMargin - point2->y();
				pt.setY(m_currPoint.y() + dy);
			}

			temp->moveLine(idx, pt.x() - m_currPoint.x(), pt.y() - m_currPoint.y(),
				m_nOffset);
			m_currPoint = pt;

			// display Alpha Value.
			int val = static_cast<int>((point1->y() + point2->y()) / 2.0f);
			QString tmpStr("  [" +
				QString().setNum((kTFHeight - val - kTFMargin) /
				(float)(kTFHeight - 2 * kTFMargin) *
					100.0f,
					'f', 0) +
				"%]");
			m_currText->setPlainText(tmpStr);
			m_currText->setPos(m_currPoint.x() - 30, m_currPoint.y() - 25);
			m_currText->setVisible(true);
		}
		else if (m_currElem._which == COMPONENT::COLOR)
		{
			// Color Object Update.
			if (m_currElem._iIdx == -1) return;
			QGraphicsEllipseItem *tmpRect = m_pgColorObject->at(m_currElem._iIdx);
			if (pt.x() < 0) pt.setX(0);
			if (pt.x() > tf_->size()) pt.setX(tf_->size());
			tmpRect->moveBy(pt.x() - m_currPoint.x(), 0);
			m_currPoint = pt;
		}
		else
			return;

		updateColor();
	}
}

// when mouse leave or non-overlapped
void CW3OTFScene::setDefaultGeometry(void)
{
	for (const auto &i : m_listOTFObj) i->deactivateAll();

	if (m_pgColorObject != nullptr)
	{
		for (const auto &i : *m_pgColorObject)
			i->setRect(-5, -5, 10, 10);  // skpark 20151104 Demo add
	}
}

void CW3OTFScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mousePressEvent(event);
	m_isPressed = true;
	QPointF pt = event->lastScenePos();
	if (m_isOnPolygon == false)
		emit sigSetScrollHandDrag();
	else if (event->button() == Qt::LeftButton)
	{
		if (m_listOTFObj.isEmpty()) return;

		for (int i = 0; i < m_listOTFObj.size(); i++)
		{
			CW3OTFPolygon *temp = m_listOTFObj.at(i);
			temp->getPolygon()->deactivate();
		}

		if (m_activatePolygonIdx >= 0)
		{
			selectPolygon(m_activatePolygonIdx);
			updateColor();
		}
	}
	m_currPoint = pt;
}

void CW3OTFScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);

	m_isPressed = false;

	m_currPoint = event->lastScenePos();
	QPoint pos(event->screenPos().x(), event->screenPos().y());
	CURR_COMPONENT currComponent = getCurrComponent(m_currPoint);
	if (event->button() == Qt::RightButton)
	{
		m_menu->clear();
		switch (currComponent)
		{
		case CURR_COMPONENT::BACKGROUND:
			m_menu->addAction(m_addPolygonAct);
			m_menu->popup(pos);
			break;
		case CURR_COMPONENT::POINT:
			m_menu->addAction(m_removePointAct);
			m_menu->popup(pos);
			break;
		case CURR_COMPONENT::POLYGON:
			m_menu->addAction(m_addPointAct);
			m_menu->addAction(m_removePointAct);
			if (m_listOTFObj.size() > 1)
			{
				m_menu->addSeparator();
				m_menu->addAction(m_removePolygonAct);
			}
			m_menu->popup(pos);
			break;
		case CURR_COMPONENT::COLOROBJ:
			m_menu->addAction(m_updateColorObjectAct);
			if (m_colorList.size() > 1) m_menu->addAction(m_removeColorObjectAct);
			m_menu->popup(pos);
			break;
		case CURR_COMPONENT::COLORTAB:
			m_menu->addAction(m_addColorObjectAct);
			m_menu->popup(pos);
			break;
		default:
			break;
		}
	}

	emit sigRenderCompleted();
}

CW3OTFScene::CURR_COMPONENT CW3OTFScene::getCurrComponent(
	const QPointF &point)
{
	// classify which component was selected.
	if (point.y() >= kTFHeight - kTFMargin)
	{  // check if color tab,
		if (m_currElem._which == COMPONENT::COLOR)
			return CURR_COMPONENT::COLOROBJ;
		else
			return CURR_COMPONENT::COLORTAB;
	}
	// check if polygon,
	else
	{
		CURR_COMPONENT curComp = CURR_COMPONENT::BACKGROUND;

		for (int objIdx = 0; objIdx < (int)m_listOTFObj.size(); objIdx++)
		{
			for (int i = 0; i < (int)m_listOTFObj.at(objIdx)->getPointList().size();
				i++)
			{
				// if point.
				if (m_listOTFObj.at(objIdx)->getPointList().at(i)->contains(point))
				{
					if (i > 0 &&
						i < (int)m_listOTFObj.at(objIdx)->getPointList().size() - 1)
					{
						curComp = CURR_COMPONENT::POINT;
						return curComp;
					}
					else
					{
						curComp = CURR_COMPONENT::POLYGON;
					}
				}
			}
			// if line.
			// if polygon.
			if (curComp == CURR_COMPONENT::POLYGON ||
				m_listOTFObj.at(objIdx)->getPolygon()->contains(point))
				return CURR_COMPONENT::POLYGON;
		}
		return CURR_COMPONENT::BACKGROUND;
	}
}

void CW3OTFScene::selectPolygon(int index)
{
	if (m_pgColorObject != nullptr)
		for (int i = 0; i < m_pgColorObject->size(); i++)
			this->removeItem(m_pgColorObject->at(i));

	m_pgColorObject = nullptr;

	for (int i = 0; i < m_listOTFObj.size(); i++)
	{
		CW3OTFPolygon *temPoly = m_listOTFObj.at(i);

		if (i == index)
		{
			temPoly->getPolygon()->select();
			m_pgColorObject = temPoly->getColorObject();
			m_selectedPolygonIdx = index;
		}
		else
		{
			temPoly->getPolygon()->deactivate();
		}
	}

	for (int i = 0; i < m_pgColorObject->size(); i++)
		this->addItem(m_pgColorObject->at(i));
}

void CW3OTFScene::slotAddPolygon(void)
{
	int index = m_listOTFObj.size();
	CW3OTFPolygon *tmpObj = new CW3OTFPolygon(index);
	tmpObj->setDefaultPolygon(m_currPoint, m_nOffset);
	tmpObj->setDefaultColor();
	tmpObj->addToScene(this);

	m_listOTFObj.append(tmpObj);

	selectPolygon(index);

	updateColor();

	emit sigRenderCompleted();
}

void CW3OTFScene::slotRemovePolygon(void)
{
	if (m_listOTFObj.size() <= LEAST_POLYGON_COUNT) return;

	for (int i = 0; i < m_pgColorObject->size(); i++)
		this->removeItem(m_pgColorObject->at(i));

	m_pgColorObject = nullptr;

	CW3OTFPolygon *temp = m_listOTFObj.at(m_activatePolygonIdx);
	temp->removeFromScene(this);
	delete (m_listOTFObj.at(m_activatePolygonIdx));
	m_listOTFObj.replace(m_activatePolygonIdx, nullptr);

	for (int i = 0; i < m_listOTFObj.size();)
	{
		if (m_listOTFObj.at(i) == nullptr)
			m_listOTFObj.removeAt(i);
		else
			++i;
	}

	if (m_listOTFObj.isEmpty()) return;

	m_pgColorObject = m_listOTFObj.last()->getColorObject();

	updateColor();

	emit sigRenderCompleted();
}

void CW3OTFScene::slotAddPoint(void)
{
	CW3OTFPolygon *temp = m_listOTFObj.at(m_currElem._iPolyIdx);
	temp->addPoint(m_currPoint, m_nOffset, this);

	m_listOTFObj.replace(m_currElem._iPolyIdx, temp);
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;
}

void CW3OTFScene::slotRemovePoint(void)
{
	// remove closest point.
	CW3OTFPolygon *temp;
	int polyIdx = -1;
	int pointIdx = -1;
	float dist = std::numeric_limits<float>::max();
	QPointF point(m_currPoint);

	for (int objIdx = 0; objIdx < m_listOTFObj.size(); objIdx++)
	{
		temp = m_listOTFObj.at(objIdx);
		for (int i = 0; i < temp->getPointList().size(); i++)
		{
			float currDist =
				pow(temp->getPointList().at(i)->pos().x() - point.x(), 2.0) +
				pow(temp->getPointList().at(i)->pos().y() - point.y(), 2.0);
			if (currDist < dist)
			{
				dist = currDist;
				polyIdx = objIdx;
				pointIdx = i;
			}
		}
	}

	if (polyIdx == -1 || pointIdx == -1) return;

	temp = m_listOTFObj.at(polyIdx);

	if (temp->getPointList().size() <= 3) return;

	temp->removePoint(pointIdx, this);

	m_listOTFObj.replace(polyIdx, temp);
	m_currElem._which = COMPONENT::NONE;
	m_currElem._iIdx = -1;
	m_currElem._iPolyIdx = -1;

	// remove OTF Object if insufficient points left.
	if (temp->getPointList().size() < 3)
	{
		temp->removeFromScene(this);
		m_listOTFObj.removeAt(polyIdx);
	}

	updateColor();

	emit sigRenderCompleted();
}

void CW3OTFScene::slotAddColor(void)
{
	ColorDialog color_dialog;
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();
	if (!color.isValid())
	{
		return;
	}

	addColorObject(m_currPoint.x(), color);
	updateColor();

	emit sigRenderCompleted();
}

void CW3OTFScene::slotRemoveColor(void)
{
	if (m_currElem._which != COMPONENT::COLOR) return;

	const int LEAST_COLOR_COUNT = 1;
	if (m_pgColorObject->count() <= LEAST_COLOR_COUNT) return;

	this->removeItem(m_pgColorObject->at(m_currElem._iIdx));
	m_pgColorObject->removeAt(m_currElem._iIdx);
	updateColor();

	emit sigRenderCompleted();
}

void CW3OTFScene::slotUpdateColor(void)
{
	QGraphicsEllipseItem* color_object = m_pgColorObject->at(m_currElem._iIdx);
	if (!color_object)
	{
		return;
	}

	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(color_object->brush().color());
	if (!color_dialog.exec())
	{
		return;
	}

	QColor color = color_dialog.SelectedColor();
	if (!color.isValid())
	{
		m_isPressed = false;
		return;
	}

	color_object->setBrush(color);
	QPen pen;
	pen.setColor(Qt::gray);
	if (color.red() > 125 && color.green() > 125 && color.blue() > 125)
	{
		pen.setColor(Qt::black);
	}
	color_object->setPen(pen);
	updateColor();
	emit sigRenderCompleted();
}

void CW3OTFScene::addColorObject(int posX, const QColor &color)
{
	QGraphicsEllipseItem *temp;
	QPen pen(Qt::gray);
	if (color.red() > 125 && color.green() > 125 && color.blue() > 125)
	{
		pen.setColor(Qt::black);
	}
	temp = addEllipse(-5, -5, 10, 10, pen, QBrush(color));
	temp->setFlag(QGraphicsEllipseItem::ItemIgnoresTransformations, true);
	temp->setPos(posX, kTFHeight - kTFMargin / 2 + 5);

	if (m_pgColorObject->size() != 0)
	{
		int idx = -1;
		for (int i = 0; i < m_pgColorObject->size(); i++)
		{
			if (m_pgColorObject->at(i)->pos().x() >= temp->pos().x())
			{
				m_pgColorObject->insert(i, temp);
				idx = i;
				break;
			}
		}
		if (idx == -1) m_pgColorObject->append(temp);
	}
	else
	{
		m_pgColorObject->append(temp);
	}
}

void CW3OTFScene::updateColor(void)
{
	if (m_listOTFObj.isEmpty())
	{
		int size = tf_colors_->size();
		tf_colors_->assign(size, TF_BGRA());
		emit sigChangeTFMove(true);
		return;
	}

	// use tmpList by sorting & displaying.
	QList<QGraphicsEllipseItem *> tmpList;
	for (int i = 0; i < m_pgColorObject->size(); i++)
		tmpList.append(m_pgColorObject->at(i));
	qSort(tmpList.begin(), tmpList.end(), Comparing);

	QList<QList<OTFColor>> allPolygonColorList;
	if (tmpList.size() != 0)
	{
		QPointF start(0.0, 0.0), last(tf_->size(), 0.0);

		QLinearGradient linearGrad(start, last);
		linearGrad.setColorAt(0, tmpList.at(0)->brush().color());
		for (int i = 0; i < tmpList.size(); i++)
		{
			if (tmpList.at(i)->pos().x() < 0)
				linearGrad.setColorAt(0, tmpList.at(i)->brush().color());
			else if (tmpList.at(i)->pos().x() >= tf_->size())
				linearGrad.setColorAt(1, tmpList.at(i)->brush().color());
			else
				linearGrad.setColorAt((tmpList.at(i)->pos().x()) /
					static_cast<float>(last.x() - start.x()),
					tmpList.at(i)->brush().color());
		}
		linearGrad.setColorAt(1, tmpList.last()->brush().color());

		QBrush brush(linearGrad);
		m_mainColorRect->setBrush(brush);

		for (int j = 0; j < m_listOTFObj.size(); j++)
		{
			QList<QGraphicsEllipseItem *> *ptTmpList =
				m_listOTFObj.at(j)->getColorObject();
			m_colorList.clear();
			for (int i = 0; i < ptTmpList->size(); i++)
			{
				OTFColor colorpoint;
				if (ptTmpList->at(i)->pos().x() < 0)
					colorpoint._intensity = 0;
				else if (ptTmpList->at(i)->pos().x() >= (int)tf_->size())
					colorpoint._intensity = (int)tf_->size() - 1;
				else
					colorpoint._intensity = ptTmpList->at(i)->pos().x();

				colorpoint._color = ptTmpList->at(i)->brush().color();

				m_colorList.append(colorpoint);
			}

			allPolygonColorList.append(m_colorList);
		}
	}
	else
	{
		m_mainColorRect->setBrush(QBrush());
		m_colorList.clear();
		return;
	}

	updateTF(allPolygonColorList);
}

void CW3OTFScene::updateTF(const QList<QList<OTFColor>> &colorList)
{
	tf_colors_->assign(tf_colors_->size(), TF_BGRA());

	int tf_size = tf_->size();
	// Initialize.
	const float dynamic_range = kTFHeight - 2 * kTFMargin;
	float *alphaSum = SAFE_ALLOC_1D(float, tf_size);
	memset(alphaSum, 0.0f, sizeof(float) * tf_size);

	float *tempAlpha = SAFE_ALLOC_1D(float, tf_size);
	for (int objIdx = 0; objIdx < m_listOTFObj.size(); objIdx++)
	{
		memset(tempAlpha, 0, sizeof(float) * tf_size);

		QList<CW3EllipseItem_otf *> ptList =
			m_listOTFObj.at(objIdx)->getPointList();
		for (int i = 0; i < (int)ptList.size() - 1; i++)
		{
			int startIdx = ptList.at(i)->x();
			int endIdx = ptList.at(i + 1)->x();
			float inc = (ptList.at(i)->y() - ptList.at(i + 1)->y()) /
				(endIdx - startIdx) *
				m_adjustOTF.contrast;  // thyoo 160923: contrast 추가
			float yPos = kTFHeight - ptList.at(i)->y() - kTFMargin;
			for (int j = startIdx; j < endIdx; j++)
			{
				if (j < 0 || j >= tf_size) continue;
				float val = yPos / dynamic_range;
				if (tf_colors_->at(j).a < val) tf_colors_->at(j).a = val;

				tempAlpha[j] = val;
				alphaSum[j] += val;

				yPos += inc;
			}
		}

		m_colorList = colorList.at(objIdx);
		for (int i = 0; i < m_colorList.size(); i++)
		{
			QColor tmpColor = m_colorList.at(i)._color;
			if (i == 0)
			{
				for (int j = 0; j < m_colorList.at(i)._intensity; j++)
				{
					tf_colors_->at(j).r += (tmpColor.redF() * tempAlpha[j]);
					tf_colors_->at(j).g += (tmpColor.greenF() * tempAlpha[j]);
					tf_colors_->at(j).b += (tmpColor.blueF() * tempAlpha[j]);
				}
			}
			else
			{
				int length =
					m_colorList.at(i)._intensity - m_colorList.at(i - 1)._intensity;
				float dR = (m_colorList.at(i)._color.redF() -
					m_colorList.at(i - 1)._color.redF()) /
					static_cast<float>(length);
				float dG = (m_colorList.at(i)._color.greenF() -
					m_colorList.at(i - 1)._color.greenF()) /
					static_cast<float>(length);
				float dB = (m_colorList.at(i)._color.blueF() -
					m_colorList.at(i - 1)._color.blueF()) /
					static_cast<float>(length);
				float valR = 0.0f;
				float valG = 0.0f;
				float valB = 0.0f;
				for (int j = m_colorList.at(i - 1)._intensity;
					j < m_colorList.at(i)._intensity; j++)
				{
					tf_colors_->at(j).r +=
						((m_colorList.at(i - 1)._color.redF() + valR) * tempAlpha[j]);
					tf_colors_->at(j).g +=
						((m_colorList.at(i - 1)._color.greenF() + valG) * tempAlpha[j]);
					tf_colors_->at(j).b +=
						((m_colorList.at(i - 1)._color.blueF() + valB) * tempAlpha[j]);
					valR += dR;
					valG += dG;
					valB += dB;
				}
			}
		}
		for (int i = m_colorList.last()._intensity; i < tf_size; i++)
		{
			tf_colors_->at(i).r += (m_colorList.last()._color.redF() * tempAlpha[i]);
			tf_colors_->at(i).g +=
				(m_colorList.last()._color.greenF() * tempAlpha[i]);
			tf_colors_->at(i).b += (m_colorList.last()._color.blueF() * tempAlpha[i]);
		}
	}

	SAFE_DELETE_ARRAY(tempAlpha);

	/////////////////////////////////////////////////////////////////////
	// thyoo: integrate를 하기 위해 TF의 RGBA 바꾸면 raycasting이 아닌 다른 곳에서
	// TF를 참조 할 때 실제 값을 알 수 없으므로 수정함.
	// TF.cpp 94줄 참조
	/////////////////////////////////////////////////////////////////////

	int shiftOffset = (int)m_adjustOTF.bright;
	if (shiftOffset >= 0)
	{
		for (int i = shiftOffset; i < tf_size; i++)
		{
			float a = alphaSum[i];

			if (a > 0.0f)
			{
				tf_colors_->at(i).r = tf_colors_->at(i).r / a;
				tf_colors_->at(i).g = tf_colors_->at(i).g / a;
				tf_colors_->at(i).b = tf_colors_->at(i).b / a;
			}

			const int shiftIdx = i - shiftOffset;
			tf_colors_->at(shiftIdx).r = tf_colors_->at(i).r;
			tf_colors_->at(shiftIdx).g = tf_colors_->at(i).g;
			tf_colors_->at(shiftIdx).b = tf_colors_->at(i).b;
			tf_colors_->at(shiftIdx).a = tf_colors_->at(i).a * (m_adjustOTF.opacity);

			tf_colors_->at(shiftIdx).r = (tf_colors_->at(shiftIdx).r > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).r;
			tf_colors_->at(shiftIdx).r = (tf_colors_->at(shiftIdx).r < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).r;

			tf_colors_->at(shiftIdx).g = (tf_colors_->at(shiftIdx).g > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).g;
			tf_colors_->at(shiftIdx).g = (tf_colors_->at(shiftIdx).g < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).g;

			tf_colors_->at(shiftIdx).b = (tf_colors_->at(shiftIdx).b > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).b;
			tf_colors_->at(shiftIdx).b = (tf_colors_->at(shiftIdx).b < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).b;

			tf_colors_->at(shiftIdx).a = (tf_colors_->at(shiftIdx).a > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).a;
			tf_colors_->at(shiftIdx).a = (tf_colors_->at(shiftIdx).a < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).a;
		}
	}
	else
	{
		for (int i = tf_size - 1 + shiftOffset; i > -1; i--)
		{
			float a = alphaSum[i];

			if (a > 0.0f)
			{
				tf_colors_->at(i).r = tf_colors_->at(i).r / a;
				tf_colors_->at(i).g = tf_colors_->at(i).g / a;
				tf_colors_->at(i).b = tf_colors_->at(i).b / a;
			}

			const int shiftIdx = i - shiftOffset;
			tf_colors_->at(shiftIdx).r = tf_colors_->at(i).r;
			tf_colors_->at(shiftIdx).g = tf_colors_->at(i).g;
			tf_colors_->at(shiftIdx).b = tf_colors_->at(i).b;
			tf_colors_->at(shiftIdx).a = tf_colors_->at(i).a * (m_adjustOTF.opacity);

			tf_colors_->at(shiftIdx).r = (tf_colors_->at(shiftIdx).r > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).r;
			tf_colors_->at(shiftIdx).r = (tf_colors_->at(shiftIdx).r < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).r;

			tf_colors_->at(shiftIdx).g = (tf_colors_->at(shiftIdx).g > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).g;
			tf_colors_->at(shiftIdx).g = (tf_colors_->at(shiftIdx).g < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).g;

			tf_colors_->at(shiftIdx).b = (tf_colors_->at(shiftIdx).b > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).b;
			tf_colors_->at(shiftIdx).b = (tf_colors_->at(shiftIdx).b < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).b;

			tf_colors_->at(shiftIdx).a = (tf_colors_->at(shiftIdx).a > 1.0f)
				? 1.0f
				: tf_colors_->at(shiftIdx).a;
			tf_colors_->at(shiftIdx).a = (tf_colors_->at(shiftIdx).a < 0.0f)
				? 0.0f
				: tf_colors_->at(shiftIdx).a;
		}
	}

	SAFE_DELETE_ARRAY(alphaSum);
	tf_->FindMinMax();

	// TF 영역 바뀐 것 sig 따로 보낼 것
	if (m_TFmin != tf_->min_value() || m_TFmax != tf_->max_value())
	{
		m_TFmin = tf_->min_value();
		m_TFmax = tf_->max_value();
		emit sigChangeTFMove(true);
	}
	else
	{
		emit sigChangeTFMove(false);
	}
}

void CW3OTFScene::SetSoftTissueMin(const float value)
{
	new_soft_tissue_min_from_generate_face_dialog_ = value;
}
