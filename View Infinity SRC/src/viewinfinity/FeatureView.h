

#pragma once

#ifndef FEATUREVIEW_H
#define FEATUREVIEW_H

#include <QApplication>
#include <QtGui>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
#include <iostream>

class SMVMain;
class FeatureViewNodeGraphic;

class FeatureView : public QGraphicsView
{
	Q_OBJECT

public:
	FeatureView(SMVMain *graphWidget);
	~FeatureView(void);
	void showFullScene();
	void setScrollBarValue(qreal x_val, qreal y_val);
	
	void fadeOut();
	void fadeIn();

	void zoomIntoFileView(qreal wished_scale, bool wished_zoom_in, ogdf::node feature_node);
	void zoomIntoFeatureView();

	void setForegroundTransparent();
	void setForegroundOpaque();
	void setNextWidgetToFadeIn(int widget_index);

	void calculateScalingLimits();

	// variables for zooming
	qreal								initial_scale;
	qreal								current_scale;
	qreal								wished_scale;
	qreal								zoom_speed;
	bool								wished_zoomed_in;
	int									timerId;

	qreal								lower_scaling_limit;
	bool								lower_scaling_zoomed_in;
	qreal								upper_scaling_limit;
	bool								upper_scaling_zoomed_in;
	qreal								scale_after_zoom_from_other_view;

	int									aktivButton;
	QString								wishedZoomInFeature;
	FeatureViewNodeGraphic*				wishedZoomInFeatureGraphic;

	FeatureViewNodeGraphic*				zoomIntoFeatureGraphic;

	//bool								zooming_active;
	bool								fading_in_active;
	bool								fading_out_active;

	ogdf::node							filezoom_feature_node;


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
	//void drawForeground(QPainter *painter, const QRectF &rect);
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    void scaleView(qreal scaleFactor);
	void resizeEvent(QResizeEvent *event);
    //void paintEvent(QPaintEvent *event);
	

private:

	void updateForegroundBrush();
	void updateButtons();

	QList<FeatureViewNodeGraphic*>		getVisibleNodeGraphics();

	SMVMain								*mainWidget;
	QGraphicsScene						*mScene;

	// variables for panning
	bool								isPanning;
	QPointF								dragStartPosition;

	// variables for fading
	int									nextWidgetIndexToFadeIn;
	int mTimerID;

	// variables for animation: zoom into feature node
	int									filezoom_steps;
	qreal								filezoom_current_scale;
	qreal								filezoom_scale_interval;
	qreal								filezoom_fade_interval;
	bool								filezoom_wished_zoomed_in;
	int									filezoom_counter;
	int									filezoom_TimerID;

	int									timeinterval;
	int									currentinterval;

    QColor								startColor;
    int									currentAlpha;
    int									duration;

	// variables for buttons

	qreal								interval_button0_upper;
	qreal								interval_button1_upper;
	qreal								interval_button2_upper;
	qreal								interval_button3_upper;
	qreal								interval_button4_upper;
	qreal								interval_button5_upper;
	qreal								interval_button6_upper;
	qreal								interval_button7_upper;
	qreal								interval_button8_upper;
	qreal								interval_button9_upper;

	qreal								interval_button0_lower;
	qreal								interval_button1_lower;
	qreal								interval_button2_lower;
	qreal								interval_button3_lower;
	qreal								interval_button4_lower;
	qreal								interval_button5_lower;
	qreal								interval_button6_lower;

};

#endif