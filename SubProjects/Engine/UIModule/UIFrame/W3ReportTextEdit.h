#pragma once

#include <QTextEdit>
#include <QList>
#include "uiframe_global.h"

class QMemeData;

class UIFRAME_EXPORT CW3ReportTextEdit : public QTextEdit {
    Q_OBJECT
	public:
	enum eImage_layout { LAYOUT_1BY1, LAYOUT_2BY2, LAYOUT_3BY3};

	CW3ReportTextEdit(QWidget *parent);

	void        dropImage(const QString& filePath, int imgWidth, int imgHeight, const QString& format);
	void		setInformation( const QString& patientName,
								const QString& patientID,
								const QString& patientAge,
								const QString& patientSex,
								const QString& studyID,
								const QString& studyDate,
								const QString& description);

	//20250216 LIN editor focus해서 마지막 페이지안 보이는 걸 방지
	void		updatePages();
	//=====
	protected:
	void		dragEnterEvent(QDragEnterEvent *event) override;
	void		dragMoveEvent(QDragMoveEvent *event) override;
	void		dropEvent(QDropEvent *event) override;
	bool        canInsertFromMimeData(const QMimeData *source) const;
	void        insertFromMimeData(const QMimeData *source);
	QMimeData  *createMimeDataFromSelection() const;



	//20250124 LIN
	//로직은: 1. 우클릭해서 page 추가 및 삭제 
	//		 2. wheel event 통해서 page 바꿈
	void		wheelEvent(QWheelEvent * event) override;
	void		contextMenuEvent(QContextMenuEvent *event) override;

	//signals:
	//	void wheelScrolled();
	private:
	void		updatePageIndexes();
	void		setHtmlPage();

	private slots:
	void		addPage();
	void		showPage(int index, bool isUpdatePatientInfo);
	void		deletePage();
	void		previousPage();
	void		nextPage();
	void		setImageLayout(int layout_index);
	
	public:
	QList<QString> pages;
	QString selected_layout_url_;
	private:
		int currentPageIndex = 0;
		int wheelCounter = 0;
	//====
};
