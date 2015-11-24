

#pragma once

#ifndef LINEAREDGEGRAPHIC_H
#define LINEAREDGEGRAPHIC_H

#include <QtGui>
#include <QPoint>
#include <QtGlobal>
#include <QGraphicsItem>
#include "SMVMain.h"

class LinearEdgeGraphic : public QGraphicsItem
{
public:
	LinearEdgeGraphic(SMVMain *graphWidget);
	LinearEdgeGraphic(SMVMain *graphWidget, QPointF s, QPointF t);

	QRectF	boundingRect() const;
	void	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void	setSource(QPointF s);
	QPointF	getSource();
	void	setTarget(QPointF t);
	QPointF	getTarget();

private:
	SMVMain					*graph;
	QPointF						mSource, mTarget;
};

#endif