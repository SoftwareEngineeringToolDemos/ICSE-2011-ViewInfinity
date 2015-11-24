

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsItem>

#include "FeatureEdgeGraphic.h"


FeatureEdgeGraphic::FeatureEdgeGraphic(SMVMain *graphWidget)
    : graph(graphWidget)
{
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
	mSource = QPointF(0,0);
	mTarget = mSource;
}

FeatureEdgeGraphic::FeatureEdgeGraphic(SMVMain *graphWidget, QPointF s, QPointF t)
    : graph(graphWidget)
{
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-2);

	mSource = s;
	mTarget = t;
}

void	FeatureEdgeGraphic::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	//painter->setClipRect( option->exposedRect );
	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
	
	QLineF line(mSource.x(), mSource.y(), mTarget.x(), mTarget.y());
	painter->drawLine(line);
	//widget->repaint(this->mapRectToParent(this->boundingRect()).toRect());
	/*
	line.setPoints(this->boundingRect().topLeft(),this->boundingRect().topRight());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().topRight(),this->boundingRect().bottomRight());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().bottomRight(),this->boundingRect().bottomLeft());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().bottomLeft(),this->boundingRect().topLeft());
	painter->drawLine(line);
	*/
}

QRectF FeatureEdgeGraphic::boundingRect() const
 {
	// get Bounding Rectangle
	qreal b_x, b_y, b_w, b_h;
	if (mSource.x() < mTarget.x())
	{
		 b_x = mSource.x();
		 b_w = mTarget.x() - mSource.x();
	}
	else
	{
		b_x = mTarget.x();
		b_w = mSource.x() - mTarget.x();
	}
	if (mSource.y() < mTarget.y())
	{
		b_y = mSource.y();
		b_h = mTarget.y() - mSource.y();
	}
	else
	{
		b_y = mTarget.y();
		b_h = mSource.y() - mTarget.y();
	}
	return QRectF(b_x, b_y, b_w, b_h);
}

void	FeatureEdgeGraphic::setSource(QPointF s)
{
	mSource = s;
}

QPointF	FeatureEdgeGraphic::getSource()
{
	return mSource;
}
void	FeatureEdgeGraphic::setTarget(QPointF t)
{
	mTarget = t;
}

QPointF	FeatureEdgeGraphic::getTarget()
{
	return mTarget;
}