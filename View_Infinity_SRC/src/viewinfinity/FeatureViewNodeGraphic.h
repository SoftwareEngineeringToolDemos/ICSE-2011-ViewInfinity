

#pragma once

#ifndef FEATUREVIEWNODEGRAPHIC_H
#define FEATUREVIEWNODEGRAPHIC_H

#include <QtGui>
#include <QtGlobal>
#include <QGraphicsItem>
#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
//#include "SMVMain.h"

class SMVMain;

class FeatureViewNodeGraphic : public QGraphicsItem
{
public:
	FeatureViewNodeGraphic(SMVMain *graphWidget);

	QRectF			boundingRect() const;
	void			paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void			setFeatureGraphNode(ogdf::node n);
	ogdf::node		getFeatureGraphNode();
	int				getObjectWidth();
	bool			setObjectWidth(int w);
	int				getObjectHeight();
	bool			setObjectHeight(int h);

	bool			setLabel(QString text);
	QString			Label();
	bool			setType(int nodetype);
	int				Type();
	void			setActivation(bool active);
	bool			getActivation();
	void			setIsFeature(bool nodetype);
	bool			isFeature();
	void			setFeatureColor(QColor col);
	QColor			getFeatureColor();

	QPixmap&		setAlpha(QPixmap &px, int val);


	void			calculateDimensions();
	
	static const int BORDER_MARGIN			= 5;

	QGraphicsView	*m_parent;

	bool			drawfilescenepreview;
	int				preview_pixmap_height, preview_pixmap_y_start;
	int				preview_pixmap_width, preview_pixmap_x_start;

	int				preview_pixmap_alpha;

protected:
	//void			hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	//void			hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void			mousePressEvent(QGraphicsSceneMouseEvent* event);
	void			mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event);
	//void			hoverMoveEvent(QGraphicsSceneHoverEvent* event);
	void			dragEnterEvent(QGraphicsSceneDragDropEvent *event);
	void			dragMoveEvent(QGraphicsSceneDragDropEvent *event);
	void			dropEvent(QGraphicsSceneDragDropEvent *event);

	QVariant FeatureViewNodeGraphic::itemChange(GraphicsItemChange change, const QVariant &value);

 private:
	 SMVMain					*graph;
	 ogdf::node					mNode;
	 int						mHeight, mWidth, mHalfHeight, mHalfWidth;
	 QGraphicsSimpleTextItem	*textitem;
	 QPointF					textpos;
	 //QGraphicsPixmapItem		*icon;
	 //QPixmap					pixmap;
	 QColor						mLineColor;
	 QColor						featureColor;

	 QString					label;
	 int						type;
	 bool						mActive;
	 bool						mIsFeature;
};

#endif