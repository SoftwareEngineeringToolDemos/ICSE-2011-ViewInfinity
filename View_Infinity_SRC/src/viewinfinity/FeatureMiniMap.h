

#pragma once

#ifndef FEATUREMINIMAP_H
#define FEATUREMINIMAP_H

#include <QApplication>
#include <QtGui>
#include <QDockWidget>

class FeatureViewInteractiveRect;
class FeatureView;
class SMVMain;

class FeatureMiniMap : public QGraphicsView
{
	Q_OBJECT

public:
	FeatureMiniMap(QWidget  * parent = 0);
	~FeatureMiniMap(void);
	void setMainWidget(SMVMain *mWidget);
	void showFullScene();
	void setFeatureView(FeatureView *view);
	//void setScrollBarValue(qreal val);
	void updateContent();
	//void setInteractiveRectToSize();

	void calculateScalingLimits();

	FeatureViewInteractiveRect	*m_intrect;			// navigator
	FeatureView					*mFeatureView;		// pointer on feature view
	SMVMain						*mainWidget;

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
	void scrollContentsBy(int dx, int dy);

private:

	QWidget					*mParent;
	QGraphicsScene			*mScene;

	// variables for zooming
	qreal								lower_scaling_limit;
	bool								lower_scaling_zoomed_in;
	qreal								upper_scaling_limit;
	bool								upper_scaling_zoomed_in;
	qreal								scale_after_zoom_from_other_view;

	int									timerId;
	qreal								initial_scale;
	qreal								current_scale;
	qreal								wished_scale;
	qreal								zoom_speed;
	bool								wished_zoomed_in;
	int									aktivButton;

	// variables for panning
	bool								isPanning;
	QPointF								dragStartPosition;

	// variables for navigator
	float					zoomlevel;
};

#endif