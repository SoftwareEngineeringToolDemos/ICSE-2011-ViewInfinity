

#pragma once

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QApplication>
#include <QtGui>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
#include <iostream>

class SMVMain;
class FileViewNodeGraphic;

class FileView : public QGraphicsView
{
	Q_OBJECT

public:
	FileView(SMVMain *graphWidget);
	~FileView(void);
	void showFullScene();
	void showFullSceneInstant();
	unsigned int getLOD();
	void zoomToCodeView();
	void zoomToCodeViewFile(FileViewNodeGraphic *graphic);
	void setScrollBarValue(qreal x_val, qreal y_val);
	void fadeOut();
	void fadeIn();
	void setForegroundTransparent();
	void setForegroundOpaque();
	void setNextWidgetToFadeIn(int widget_index);

	void calculateScalingLimits();

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

	int									aktivButton;
	FileViewNodeGraphic					*wishedZoomInFile;

	FileViewNodeGraphic					*zoomIntoFileGraphic;

	bool								fading_in_active;
	bool								fading_out_active;
	bool								fading_in_from_codeview;
	bool								fading_in_from_featureview;

	//qreal								scale_at_view_switch;

public slots:
	void stripe0();
	void stripe1();
	void stripe2();
	void stripe3();
	void stripe4();
	void stripe5();
	void stripe6();
	void stripe7();
	void stripe8();
	void stripe9();
	void stripe10();
	void zmButton();
	void zpButton();

protected:
	void drawBackground(QPainter *painter, const QRectF &rect);
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    void scaleView(qreal scaleFactor);
	void resizeEvent(QResizeEvent *event);

private:

	void updateForegroundBrush();
	void updateButtons();

	QList<FileViewNodeGraphic*> getVisibleNodeGraphics();
	void					calculateLevelOfDetail();

	SMVMain					*mainWidget;
	QGraphicsScene			*mScene;

	unsigned int			LOD; // level of detail

	// variables for panning
	bool								isPanning;
	QPointF								dragStartPosition;
	
	// variables for fading
	int						nextWidgetIndexToFadeIn;
	int mTimerID;
	int timeinterval;
	int currentinterval;
    QColor startColor;
    int currentAlpha;
    int duration;

	// variables for buttons

	qreal lower_scaling_interval;
	qreal upper_scaling_interval;

	qreal interval_button0_lower;
	qreal interval_button1_lower;
	qreal interval_button2_lower;
	qreal interval_button3_lower;
	qreal interval_button4_lower;
	qreal interval_button5_lower;
	qreal interval_button6_lower;
	qreal interval_button7_upper;
	qreal interval_button8_upper;
	qreal interval_button9_upper;

};

#endif