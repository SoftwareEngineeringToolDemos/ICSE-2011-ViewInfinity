

#pragma once

#ifndef FILEMINIMAP_H
#define FILEMINIMAP_H

#include <QApplication>
#include <QtGui>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
#include <iostream>

class FileViewInteractiveRect;
class FileView;
class SMVMain;
class FileMiniMapNodeGraphic;

class FileMiniMap : public QGraphicsView
{
	Q_OBJECT

public:
	FileMiniMap(QWidget  * parent = 0);
	~FileMiniMap(void);
	void showFullScene();
	unsigned int getLOD();
	void setMainWidget(SMVMain *mWidget);
	void setFileView(FileView *view);
	//void setScrollBarValue(qreal val);
	void updateContent();

	void calculateScalingLimits();

	SMVMain						*mainWidget;
	FileViewInteractiveRect		*m_intrect;			// navigator
	FileView					*mFileView;		// pointer on file view

protected:
	void drawBackground(QPainter *painter, const QRectF &rect);
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    void scaleView(qreal scaleFactor);
	QList<FileMiniMapNodeGraphic*> getVisibleNodeGraphics();
	void resizeEvent(QResizeEvent *event);
	void scrollContentsBy(int dx, int dy);

private:
	void					calculateLevelOfDetail();
	QWidget					*mParent;
	QGraphicsScene			*mScene;
	unsigned int			LOD; // level of detail
	
	// variables for zooming
	int									timerId;
	qreal								initial_scale;
	qreal								current_scale;
	qreal								wished_scale;
	qreal								zoom_speed;
	bool								wished_zoomed_in;

	qreal								lower_scaling_limit;
	bool								lower_scaling_zoomed_in;
	qreal								upper_scaling_limit;
	bool								upper_scaling_zoomed_in;
	qreal								scale_after_zoom_from_other_view;

	int									aktivButton;

	// variables for panning
	bool								isPanning;
	QPointF								dragStartPosition;
	
	// variables for navigator
	float					zoomlevel;

};

#endif