

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsItem>

#include "LinearEdgeGraphic.h"


LinearEdgeGraphic::LinearEdgeGraphic(SMVMain *graphWidget)
    : graph(graphWidget)
{
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
	mSource = QPointF(0,0);
	mTarget = mSource;
}

LinearEdgeGraphic::LinearEdgeGraphic(SMVMain *graphWidget, QPointF s, QPointF t)
    : graph(graphWidget)
{
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-2);

	mSource = s;
	mTarget = t;
}

void	LinearEdgeGraphic::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	//painter->setClipRect( option->exposedRect );

	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
	QLineF line(mSource.x(), mSource.y(), mTarget.x(), mTarget.y());
	painter->drawLine(line);
	/*
	//debug
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

QRectF LinearEdgeGraphic::boundingRect() const
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

void	LinearEdgeGraphic::setSource(QPointF s)
{
	mSource = s;
}

QPointF	LinearEdgeGraphic::getSource()
{
	return mSource;
}
void	LinearEdgeGraphic::setTarget(QPointF t)
{
	mTarget = t;
}

QPointF	LinearEdgeGraphic::getTarget()
{
	return mTarget;
}