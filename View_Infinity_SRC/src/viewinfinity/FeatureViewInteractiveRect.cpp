

#include <QGraphicsItem>
#include <QPainter>
#include "SMVMain.h"
#include "FeatureMiniMap.h"
#include "FeatureView.h"
#include "FeatureViewInteractiveRect.h"


FeatureViewInteractiveRect::FeatureViewInteractiveRect(FeatureMiniMap *minimap)
    : m_featureminimap(minimap)
{
	setFlags(ItemIsMovable);
	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

	rect_active = false;

	rect_height = 0.0;
	rect_width = 0.0;
}
QRectF FeatureViewInteractiveRect::boundingRect() const
{
	// get Bounding Rectangle
	return QRectF(-rect_width/2, -rect_height/2, rect_width, rect_height);
}

void FeatureViewInteractiveRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{
	painter->setPen(QPen(QBrush(Qt::red,Qt::BrushStyle(Qt::SolidLine)),0,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
	painter->drawRect(-rect_width/2+2, -rect_height/2+2, rect_width-4, rect_height-4);
}

void FeatureViewInteractiveRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	if (this->m_featureminimap->mainWidget->mViewMode == 0)		// if view mode is FEATURE_VIEW
	{
		rect_active = true;
		m_featureminimap->setCursor(Qt::PointingHandCursor);
		QGraphicsItem::hoverEnterEvent(event);
	}
}

void FeatureViewInteractiveRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	rect_active = false;
	m_featureminimap->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}

void FeatureViewInteractiveRect::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsItem::hoverMoveEvent(event);
}

void FeatureViewInteractiveRect::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (this->m_featureminimap->mainWidget->mViewMode == 0)		// if view mode is FEATURE_VIEW
	{
		m_featureminimap->setCursor(Qt::ClosedHandCursor);
		QGraphicsItem::mousePressEvent(event);
	}
}

// override mouse movement
void FeatureViewInteractiveRect::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
	if (rect_active)
	{
		updateIntRectPos(event);
	}
}
void FeatureViewInteractiveRect::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )
{
	QGraphicsItem::dragLeaveEvent ( event );
	rect_active = false;
}
void FeatureViewInteractiveRect::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
	QGraphicsItem::mouseReleaseEvent ( event );
	if (rect_active == false)
		m_featureminimap->setCursor(Qt::ArrowCursor);
	else m_featureminimap->setCursor(Qt::PointingHandCursor);
}
void FeatureViewInteractiveRect::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
}
// calculate size of navigator rectangle
void FeatureViewInteractiveRect::updateIntRect()
{
	// get view rect in scene coordinates
	QPointF item_topleft = m_featureminimap->mFeatureView->mapToScene(m_featureminimap->mFeatureView->rect()).boundingRect().topLeft();
	QPointF item_bottomright = m_featureminimap->mFeatureView->mapToScene(m_featureminimap->mFeatureView->rect()).boundingRect().bottomRight();
	
	// clip at scene borders
	if (item_topleft.y() < m_featureminimap->mFeatureView->sceneRect().topLeft().y())
	{	
		//qDebug() << "rect above scene !";
		item_topleft.setY(m_featureminimap->mFeatureView->sceneRect().topLeft().y());
	}
	if (item_bottomright.y() > m_featureminimap->mFeatureView->sceneRect().bottomRight().y())
	{	
		//qDebug() << "rect below scene !";
		item_bottomright.setY(m_featureminimap->mFeatureView->sceneRect().bottomRight().y());
	}
	if (item_topleft.x() < m_featureminimap->mFeatureView->sceneRect().topLeft().x())
	{	
		//qDebug() << "rect too much on left !";
		item_topleft.setX(m_featureminimap->mFeatureView->sceneRect().topLeft().x());

	}
	if (item_bottomright.x() > m_featureminimap->mFeatureView->sceneRect().bottomRight().x())
	{	
		//qDebug() << "rect too much on right !";
		item_bottomright.setX(m_featureminimap->mFeatureView->sceneRect().bottomRight().x());
	}

	rect_height = item_bottomright.y() - item_topleft.y();
	rect_width = item_bottomright.x() - item_topleft.x();

	QPointF newpos;
	this->setPos(item_topleft.x() + ( rect_width / 2 ), item_topleft.y() + ( rect_height / 2 ) );

	// update content
	m_featureminimap->update(m_featureminimap->rect());
	this->update(this->boundingRect());
}
void FeatureViewInteractiveRect::updateIntRectPos( QGraphicsSceneMouseEvent * event)
{
	// calculate position

	QPointF oldpos = this->pos();
	QPointF newpos(oldpos);
	newpos.setX(newpos.x()+(event->pos().x()-event->lastPos().x()));
	newpos.setY(newpos.y()+(event->pos().y()-event->lastPos().y()));
	this->setPos(newpos);
	
	if (this->mapRectToScene(this->boundingRect()).topLeft().y() < m_featureminimap->sceneRect().topLeft().y())
	//if (this->scenePos().y() - rect_height/2 < -m_featureminimap->sceneRect().height()/2)
	{
		// item moved over top border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) < 0)	// mouse moving to top
		{
			qreal corrected_position = m_featureminimap->sceneRect().topLeft().y() + rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}
	else if (this->mapRectToScene(this->boundingRect()).bottomLeft().y() > m_featureminimap->sceneRect().bottomLeft().y())
	{
		// item moved over bottom border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) > 0)	// mouse moving to buttom
		{
			qreal corrected_position = m_featureminimap->sceneRect().bottomLeft().y() - rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}
	if (this->mapRectToScene(this->boundingRect()).topLeft().x() < m_featureminimap->sceneRect().topLeft().x())
	{
		// item moved over left border -> limit movement
		if ((event->pos().x()-event->lastPos().x()) < 0)	// mouse moving to left
		{
			qreal corrected_position = m_featureminimap->sceneRect().topLeft().x() + rect_width/2;
			newpos.setX(corrected_position);
			this->setPos(newpos);
		}
	}
	else if (this->mapRectToScene(this->boundingRect()).topRight().x() > m_featureminimap->sceneRect().topRight().x())
	{
		// item moved over right border -> limit movement
		if ((event->pos().x()-event->lastPos().x()) > 0)	// mouse moving to right
		{
			qreal corrected_position = m_featureminimap->sceneRect().topRight().x() - rect_width/2;
			newpos.setX(corrected_position);
			this->setPos(newpos);
		}
	}
	
	// calculate corresponding scroll bar position for feature view
	qreal total_dist_y = m_featureminimap->sceneRect().height()-rect_height;
	qreal diff_y = m_featureminimap->sceneRect().topLeft().y() + rect_height / 2;
	qreal y_val = ( scenePos().y() - diff_y ) / total_dist_y;

	qreal total_dist_x = m_featureminimap->sceneRect().width()-rect_width;
	qreal diff_x = m_featureminimap->sceneRect().topLeft().x() + rect_width / 2;
	qreal x_val = ( scenePos().x() - diff_x ) / total_dist_x;
	
	// set scroll position
	m_featureminimap->mFeatureView->setScrollBarValue(x_val, y_val);

	// with new position update feature view
	m_featureminimap->update(m_featureminimap->rect());
	this->update(this->boundingRect());
}
void FeatureViewInteractiveRect::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	QGraphicsItem::wheelEvent(event);
}

void FeatureViewInteractiveRect::setRectInScene(QPointF item_topleft, QPointF item_bottomright)
{
	//qDebug() << "setRectInScene";

	// clip at scene borders
	if (item_topleft.y() < m_featureminimap->mFeatureView->sceneRect().topLeft().y())
	{	
		//qDebug() << "rect above scene !";
		item_topleft.setY(m_featureminimap->mFeatureView->sceneRect().topLeft().y());
	}
	if (item_bottomright.y() > m_featureminimap->mFeatureView->sceneRect().bottomRight().y())
	{	
		//qDebug() << "rect below scene !";
		item_bottomright.setY(m_featureminimap->mFeatureView->sceneRect().bottomRight().y());
	}
	if (item_topleft.x() < m_featureminimap->mFeatureView->sceneRect().topLeft().x())
	{	
		//qDebug() << "rect too much on left !";
		item_topleft.setX(m_featureminimap->mFeatureView->sceneRect().topLeft().x());

	}
	if (item_bottomright.x() > m_featureminimap->mFeatureView->sceneRect().bottomRight().x())
	{	
		//qDebug() << "rect too much on right !";
		item_bottomright.setX(m_featureminimap->mFeatureView->sceneRect().bottomRight().x());
	}

	rect_height = item_bottomright.y() - item_topleft.y();
	rect_width = item_bottomright.x() - item_topleft.x();

	QPointF newpos;
	this->setPos(item_topleft.x() + ( rect_width / 2 ), item_topleft.y() + ( rect_height / 2 ) );

	// update content
	m_featureminimap->update(m_featureminimap->rect());
	this->update(this->boundingRect());
}