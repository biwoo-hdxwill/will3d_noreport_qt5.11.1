#include "W3DicomLoaderThumbnailView.h"

#include <QMouseEvent>
#include <QApplication>
#include <QGraphicsPixmapItem>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/W3Cursor.h"

namespace {
bool ConvertINT16toARGB32(short* buffer, int width, int height, unsigned char* &ubuffer, unsigned char* pLUT, bool isImgInversed) {
	if (!buffer || width <= 0 || height <= 0)
		return false;

	short *buffer16 = buffer;
	ubuffer = new unsigned char[width*height * 4];
	unsigned char *pubuffer = ubuffer;
	for (int i = 0; i < width*height; i++) {
		unsigned char value;
		if (isImgInversed == false)
			value = pLUT[*buffer16++];
		else
			value = 255 - pLUT[*buffer16++];
		*pubuffer++ = value;
		*pubuffer++ = value;
		*pubuffer++ = value;
		*pubuffer++ = 0xff; // alpha channel
	}

	return true;
}
} // end of namespace 

CW3DicomLoaderThumbnailView::CW3DicomLoaderThumbnailView(int dIndex) {
	m_nControlPos = dIndex;

	QGraphicsScene *pScene = new QGraphicsScene(this);
	this->setScene(pScene);
	this->setMouseTracking(true);
	this->setBackgroundBrush(Qt::black);

	m_pGuideRect = new CW3GuideRect();

	this->scene()->addItem(m_pGuideRect);
	m_pGuideRect->setVisible(true);
	m_pGuideRect->setZValue(1.0f);
}

CW3DicomLoaderThumbnailView::~CW3DicomLoaderThumbnailView() {
	SAFE_DELETE_OBJECT(m_pRenderPixmap);
}

void CW3DicomLoaderThumbnailView::setImage(short *pImage, int width, int height, int winWidth, int winLevel, const QRectF& giudeRect) {
	short *image_buf = new short[width * height];
	memcpy(image_buf, pImage, width * height * sizeof(short));

	bool bDimensionChanged = false;
	if (m_nWidth != width || m_nHeight != height)
		bDimensionChanged = true;

	m_nWidth = width;
	m_nHeight = height;
	m_nWindowingWidth = winWidth;
	m_nWindowingLevel = winLevel;

	this->scene()->setSceneRect(0, 0, m_nWidth, m_nHeight);
	this->fitInView(0, 0, m_nWidth, m_nHeight, Qt::AspectRatioMode::KeepAspectRatio);
	this->centerOn(QPointF(m_nWidth * 0.5f, m_nHeight * 0.5f));

	//int dInnerGap = m_nWidth > m_nHeight ? m_nWidth * 0.025f : m_nHeight * 0.025f;
	//if (bDimensionChanged)
	//{
	//	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER] = QRect(0, 0, m_nWidth, m_nHeight);
	//	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::INNER] = QRect(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().x() - dInnerGap,
	//		m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().y() - dInnerGap,
	//		dInnerGap * 2, dInnerGap * 2);
	//}

	if (m_pLUT == nullptr)
		m_pLUT = new unsigned char[0x01 << 16];
	updateLUT();

  unsigned char *display_buf = nullptr;
  ConvertINT16toARGB32(image_buf, m_nWidth, m_nHeight, display_buf, m_pgResultLUT, false);
	m_pImage = new QImage(display_buf, m_nWidth, m_nHeight, QImage::Format_ARGB32);

	QPixmap pix(m_nWidth, m_nHeight);
	QPainter painter(&pix);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.fillRect(pix.rect(), Qt::black);
	painter.drawImage(pix.rect(), *m_pImage, m_pImage->rect());
	painter.end();

	if (m_pRenderPixmap)
		this->scene()->removeItem(m_pRenderPixmap);
	SAFE_DELETE_OBJECT(m_pRenderPixmap);
	m_pRenderPixmap = new QGraphicsPixmapItem(QPixmap());
	m_pRenderPixmap->setTransformationMode(Qt::SmoothTransformation);
	this->scene()->addItem(m_pRenderPixmap);

	m_pRenderPixmap->setPixmap(pix);
	//this->fitInView(m_pRenderPixmap, Qt::AspectRatioMode::KeepAspectRatio);
	m_bDisplayed = true;
	m_pRenderPixmap->setVisible(true);
	m_pRenderPixmap->setZValue(0.0f);

	rect_size_min_ = width * 0.1f;
	if (bDimensionChanged) {
		if (giudeRect.width() > 0.0f && giudeRect.height() > 0.0f) {
			left_max_ = giudeRect.width() - rect_size_min_;
			right_min_ = giudeRect.x() + rect_size_min_;
			top_max_ = giudeRect.height() - rect_size_min_;
			bottom_min_ = giudeRect.y() + rect_size_min_;

			m_pGuideRect->setRect(giudeRect, true);
		} else {
			left_max_ = width - rect_size_min_;
			right_min_ = rect_size_min_;
			top_max_ = height - rect_size_min_;
			bottom_min_ = rect_size_min_;

			m_pGuideRect->setRect(m_pRenderPixmap->boundingRect(), true);
		}
	}

	m_viewMargin = mapToScene(QPoint(20, 20));

	//QPointF margin = mapToScene(QPoint(10, 10));
	//this->fitInView(this->sceneRect().x() - margin.x(), this->sceneRect().y() - margin.y(), this->sceneRect().width() + (margin.x() * 2), this->sceneRect().height() + (margin.y() * 2));

	this->fitInView(m_pRenderPixmap->boundingRect().x() - m_viewMargin.x(),
		m_pRenderPixmap->boundingRect().y() - m_viewMargin.y(),
		m_pRenderPixmap->boundingRect().width() + m_viewMargin.x(),
		m_pRenderPixmap->boundingRect().height() + m_viewMargin.y(),
		Qt::AspectRatioMode::KeepAspectRatio);

	m_pGuideRect->setVisible(true);

	update();

	SAFE_DELETE_OBJECT(m_pImage);
	SAFE_DELETE_OBJECT(display_buf);
	SAFE_DELETE_OBJECT(image_buf);
	SAFE_DELETE_OBJECT(m_pLUT);
}

void CW3DicomLoaderThumbnailView::updateLUT() {
	int start, end;

	double lower = m_nWindowingLevel - m_nWindowingWidth * 0.5;
	double upper = m_nWindowingLevel + m_nWindowingWidth * 0.5;
	double HU;

	bool bDataSigned = true; // 수정 필요

	if (bDataSigned) {
		start = -std::numeric_limits<short>::max(); // - 32768
		end = std::numeric_limits<short>::max();
		m_pgResultLUT = &m_pLUT[std::numeric_limits<unsigned short>::max() / 2]; //  32768
		//m_pgResultLUT = &m_pLUT[32768];
	} else {
		start = 0;
		end = std::numeric_limits<unsigned short>::max();
		//end = 65536;
		m_pgResultLUT = m_pLUT;
	}

	HU = start;
	//for signed
	for (int i = start; i < end; i++) {
		m_pgResultLUT[i] = W3::ramp(0, 255, lower, upper, HU);
		HU++;
	}
}

void CW3DicomLoaderThumbnailView::leaveEvent(QEvent * event) {
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void CW3DicomLoaderThumbnailView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);

	if (!m_pRenderPixmap)
		return;

	if (m_nControlPos == -1)
		return;

	m_ptScene = mapToScene(event->pos());

	if (event->buttons() & Qt::LeftButton) {
		float minRectSize = rect_size_min_;
		QRectF rect = m_pGuideRect->boundingRect();
		QPointF rectCenter = m_ptScene;

		switch (m_eSelectedElement) {
		case CENTER:
			//m_pGuideRect->setPos(m_pGuideRect->scenePos() + (m_ptScene - m_ptOld));
			//rect.setX(rect.x() + (m_ptScene.x() - m_ptOld.x()));
			//rect.setY(rect.y() + (m_ptScene.y() - m_ptOld.y()));
			if (rectCenter.x() < this->sceneRect().left() + (rect.width() * 0.5f))
				rectCenter.setX(this->sceneRect().left() + (rect.width() * 0.5f));

			if (rectCenter.y() < this->sceneRect().top() + (rect.height() * 0.5f))
				rectCenter.setY(this->sceneRect().top() + (rect.height() * 0.5f));

			if (rectCenter.x() > this->sceneRect().right() - (rect.width() * 0.5f))
				rectCenter.setX(this->sceneRect().right() - (rect.width() * 0.5f));

			if (rectCenter.y() > this->sceneRect().bottom() - (rect.height() * 0.5f))
				rectCenter.setY(this->sceneRect().bottom() - (rect.height() * 0.5f));

			rect.moveCenter(rectCenter);
			break;
		case LEFT:
			rect.setX(m_ptScene.x());
			break;
		case RIGHT:
			rect.setWidth(rect.width() + (m_ptScene.x() - m_ptOld.x()));
			break;
		case TOP:
			rect.setY(m_ptScene.y());
			break;
		case BOTTOM:
			rect.setHeight(rect.height() + (m_ptScene.y() - m_ptOld.y()));
			break;
		case TOP_LEFT:
			rect.setY(m_ptScene.y());
			rect.setX(m_ptScene.x());
			break;
		case TOP_RIGHT:
			rect.setY(m_ptScene.y());
			rect.setWidth(rect.width() + (m_ptScene.x() - m_ptOld.x()));
			break;
		case BOTTOM_LEFT:
			rect.setHeight(rect.height() + (m_ptScene.y() - m_ptOld.y()));
			rect.setX(m_ptScene.x());
			break;
		case BOTTOM_RIGHT:
			rect.setHeight(rect.height() + (m_ptScene.y() - m_ptOld.y()));
			rect.setWidth(rect.width() + (m_ptScene.x() - m_ptOld.x()));
			break;
		default:
			break;
		}

		switch (m_eSelectedElement) {
		case LEFT:
			limiteRectLeft(rect, minRectSize);
			break;
		case RIGHT:
			limiteRectRight(rect, minRectSize);
			break;
		case TOP:
			limiteRectTop(rect, minRectSize);
			break;
		case BOTTOM:
			limiteRectBottom(rect, minRectSize);
			break;
		case TOP_LEFT:
			limiteRectTop(rect, minRectSize);
			limiteRectLeft(rect, minRectSize);
			break;
		case TOP_RIGHT:
			limiteRectTop(rect, minRectSize);
			limiteRectRight(rect, minRectSize);
			break;
		case BOTTOM_LEFT:
			limiteRectBottom(rect, minRectSize);
			limiteRectLeft(rect, minRectSize);
			break;
		case BOTTOM_RIGHT:
			limiteRectBottom(rect, minRectSize);
			limiteRectRight(rect, minRectSize);
			break;
		default:
			break;
		}

		m_pGuideRect->setRect(rect, false);
		scene()->update();

		right_min_ = rect.left() + rect_size_min_;
		left_max_ = rect.right() - rect_size_min_;
		bottom_min_ = rect.top() + rect_size_min_;
		top_max_ = rect.bottom() - rect_size_min_;

		emit sigSyncThumbnailRect(rect, m_nControlPos);
	} else {
		EGUIDERECT_ELEMENT element = m_pGuideRect->getElement(m_ptScene);

		switch (element) {
		case CENTER:
			QApplication::setOverrideCursor(CW3Cursor::SizeAllCursor());
			break;
		case LEFT:
		case RIGHT:
			QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
			break;
		case TOP:
		case BOTTOM:
			QApplication::setOverrideCursor(CW3Cursor::SizeVerCursor());
			break;
		case TOP_LEFT:
		case BOTTOM_RIGHT:
			QApplication::setOverrideCursor(CW3Cursor::SizeFDiagCursor());
			break;
		case TOP_RIGHT:
		case BOTTOM_LEFT:
			QApplication::setOverrideCursor(CW3Cursor::SizeBDiagCursor());
			break;
		default:
			QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
			break;
		}
	}

	m_ptOld = m_ptScene;

	//if (m_nSelectedCtrl == -1)
	//{
	//	int dPosIdx = getControlPos(mapToScene(event->pos()));

	//QPointF trans = m_ptClickedPos - mapToScene(event->pos());
	//int dInnerGap = m_nWidth > m_nHeight ? m_nWidth * 0.025f : m_nHeight * 0.025f;
	//if (event->buttons() == Qt::LeftButton && m_nSelectedCtrl != -1)
	//{
	//	if (m_nSelectedCtrl == 4)
	//	{
	//		QRect OuterRt = m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].adjusted(-trans.rx(), -trans.ry(), -trans.rx(), -trans.ry());
	//		if (QRect(0, 0, m_nWidth, m_nHeight).contains(OuterRt))
	//		{
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER] = OuterRt;
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::INNER].adjust(-trans.rx(), -trans.ry(), -trans.rx(), -trans.ry());
	//		}
	//	}
	//	else{
	//		QRect rt;
	//		switch (m_nSelectedCtrl)
	//		{
	//		case 0:
	//			rt = m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER];
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setLeft(rt.left() - trans.x());
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].left() < 0)	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setLeft(0);
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].left() > m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].right() - dInnerGap * 2)
	//				m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setLeft(m_rectGuide->right() - dInnerGap * 2);
	//			break;
	//		case 1:
	//			rt = m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER];
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setTop(rt.top() - trans.y());
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].top() < 0) m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setTop(0);
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].top() > m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].bottom() - dInnerGap * 2)
	//				m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setTop(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].bottom() - dInnerGap * 2);
	//			break;
	//		case 2:
	//			rt = m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER];
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setRight(rt.right() - trans.x());
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].right() > m_nWidth)	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setRight(m_nWidth);
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].right() < m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].left() + dInnerGap * 2)
	//				m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setRight(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].left() + dInnerGap * 2);
	//			break;
	//		case 3:
	//			rt = m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER];
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setBottom(rt.bottom() - trans.y());
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].bottom() > m_nHeight)	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setBottom(m_nHeight);
	//			if (m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].bottom() < m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].top() + dInnerGap * 2)
	//				m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].setBottom(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].top() + dInnerGap * 2);
	//			break;
	//		}

	//		m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::INNER] = QRect(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().x() - dInnerGap,
	//			m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().y() - dInnerGap,
	//			dInnerGap * 2, dInnerGap * 2);
	//	}
	//	this->scene()->update(this->sceneRect());
	//}
	//m_ptClickedPos = mapToScene(event->pos());
	//emit sigSyncDicomLoaderView(m_nControlPos);
}

void CW3DicomLoaderThumbnailView::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);

	if (!m_pRenderPixmap)
		return;

	if (m_nControlPos == -1)
		return;

	m_ptOld = m_ptScene = mapToScene(event->pos());

	m_eSelectedElement = m_pGuideRect->getElement(m_ptScene);
}

void CW3DicomLoaderThumbnailView::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	if (!m_pRenderPixmap)
		return;

	if (m_nControlPos == -1)
		return;

	m_eSelectedElement = EGUIDERECT_ELEMENT::NONE;

	//event->ignore();
	////this->setCursor(Qt::CursorShape::ArrowCursor);
	//m_nSelectedCtrl = -1;
}

void CW3DicomLoaderThumbnailView::slotSyncThumbnailRect(QRectF rect, int index) {
	//m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER] = rt;
	//int dInnerGap = m_nWidth > m_nHeight ? m_nWidth * 0.025f : m_nHeight * 0.025f;
	//m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::INNER] = QRect(m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().x() - dInnerGap,
	//	m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER].center().y() - dInnerGap,
	//	dInnerGap * 2, dInnerGap * 2);

	m_pGuideRect->setRect(rect, false);

	this->scene()->update();
}

void CW3DicomLoaderThumbnailView::resizeEvent(QResizeEvent *event) {
	//	fitInView(m_pRenderPixmap, Qt::AspectRatioMode::KeepAspectRatio);
	QGraphicsView::resizeEvent(event);

	//if (m_nPreWidth != this->width() || m_nPreHeight != this->height())
	{
		if (m_pRenderPixmap) {
			//m_viewMargin = mapToScene(QPoint(20, 20));

			this->fitInView(m_pRenderPixmap->boundingRect().x() - m_viewMargin.x(),
				m_pRenderPixmap->boundingRect().y() - m_viewMargin.y(),
				m_pRenderPixmap->boundingRect().width() + m_viewMargin.x(),
				m_pRenderPixmap->boundingRect().height() + m_viewMargin.y(),
				Qt::AspectRatioMode::KeepAspectRatio);
		}
	}

	//m_nPreWidth = this->width();
	//m_nPreHeight = this->height();
}

void CW3DicomLoaderThumbnailView::wheelEvent(QWheelEvent *event) {
	QGraphicsView::wheelEvent(event);

	if (!m_pRenderPixmap)
		return;

	emit sigWheelTranslate(m_nControlPos, event->delta() / 120);
}

void CW3DicomLoaderThumbnailView::setEmpty() {
	m_bDisplayed = false;

	m_nWidth = 0;
	m_nHeight = 0;
	//m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::OUTER] = QRect(0, 0, 0, 0);
	//m_rectGuide[DICOMLOADER_VIEWRECT_TYPE::INNER] = QRect(0, 0, 0, 0);

	if (m_pGuideRect) {
		//m_pGuideRect->setRect(QRectF(0.0f, 0.0f, 0.0f, 0.0f));
		m_pGuideRect->setVisible(false);
	}

	if (m_pRenderPixmap) {
		this->scene()->removeItem(m_pRenderPixmap);
		SAFE_DELETE_OBJECT(m_pRenderPixmap);
	}
}

QRect CW3DicomLoaderThumbnailView::getSelectedRect() {
	return QRect((int)m_pGuideRect->boundingRect().x(), (int)m_pGuideRect->boundingRect().y(),
		(int)m_pGuideRect->boundingRect().width(), (int)m_pGuideRect->boundingRect().height());
}

void CW3DicomLoaderThumbnailView::limiteRectLeft(QRectF &rect, const int &minRectSize) {
	if (rect.x() < this->sceneRect().x())
		rect.setX(this->sceneRect().x());
	if (rect.x() > rect.x() + rect.width() - minRectSize)
		rect.setX(rect.x() + rect.width() - minRectSize);
}

void CW3DicomLoaderThumbnailView::limiteRectRight(QRectF &rect, const int &minRectSize) {
	if (rect.x() + rect.width() > this->sceneRect().x() + this->sceneRect().width())
		rect.setWidth(this->sceneRect().x() + this->sceneRect().width() - rect.x());
	if (rect.width() < minRectSize)
		rect.setWidth(minRectSize);
}

void CW3DicomLoaderThumbnailView::limiteRectTop(QRectF &rect, const int &minRectSize) {
	if (rect.y() < this->sceneRect().y())
		rect.setY(this->sceneRect().y());
	if (rect.y() > rect.y() + rect.height() - minRectSize)
		rect.setY(rect.y() + rect.height() - minRectSize);
}

void CW3DicomLoaderThumbnailView::limiteRectBottom(QRectF &rect, const int &minRectSize) {
	if (rect.y() + rect.height() > this->sceneRect().y() + this->sceneRect().height())
		rect.setHeight(this->sceneRect().y() + this->sceneRect().height() - rect.y());
	if (rect.height() < minRectSize)
		rect.setHeight(minRectSize);
}

void CW3DicomLoaderThumbnailView::slotSetGuideRectLeft(int left) {
	QRectF rect = m_pGuideRect->boundingRect();
	rect.setLeft(left - 1);
	m_pGuideRect->setRect(rect, false);
	update();
}

void CW3DicomLoaderThumbnailView::slotSetGuideRectRight(int right) {
	QRectF rect = m_pGuideRect->boundingRect();
	rect.setRight(right);
	m_pGuideRect->setRect(rect, false);
	update();
}

void CW3DicomLoaderThumbnailView::slotSetGuideRectTop(int top) {
	QRectF rect = m_pGuideRect->boundingRect();
	rect.setTop(top - 1);
	m_pGuideRect->setRect(rect, false);
	update();
}

void CW3DicomLoaderThumbnailView::slotSetGuideRectBottom(int bottom) {
	QRectF rect = m_pGuideRect->boundingRect();
	rect.setBottom(bottom);
	m_pGuideRect->setRect(rect, false);
	update();
}
