

#pragma once

#ifndef FEATUREEDGEGRAPHIC_H
#define FEATUREDGEGRAPHIC_H

#include <QtGui>
#include <QPoint>
#include <QtGlobal>
#include <QGraphicsItem>
#include "SMVMain.h"

class FeatureEdgeGraphic : public QGraphicsItem
{
public:
	FeatureEdgeGraphic(SMVMain *graphWidget);
	FeatureEdgeGraphic(SMVMain *graphWidget, QPointF s, QPointF t);

	QRectF	boundingRect() const;
	void	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void	setSource(QPointF s);
	QPointF	getSource();
	void	setTarget(QPointF t);
	QPointF	getTarget();

private:
	SMVMain						*graph;
	QPointF						mSource, mTarget;
};

#endif