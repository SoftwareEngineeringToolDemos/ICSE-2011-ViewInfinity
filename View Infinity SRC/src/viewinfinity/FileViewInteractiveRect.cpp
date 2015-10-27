

#include <QGraphicsItem>
#include <QPainter>
#include "SMVMain.h"
#include "FileMiniMap.h"
#include "FileView.h"
#include "FileViewInteractiveRect.h"


FileViewInteractiveRect::FileViewInteractiveRect(FileMiniMap *minimap)
    : m_fileminimap(minimap)
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
QRectF FileViewInteractiveRect::boundingRect() const
{
	// get Bounding Rectangle
	return QRectF(-rect_width/2, -rect_height/2, rect_width, rect_height);
}

void FileViewInteractiveRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{
	painter->setPen(QPen(QBrush(Qt::red,Qt::BrushStyle(Qt::SolidLine)),0,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
	painter->drawRect(-rect_width/2+2, -rect_height/2+2, rect_width-4, rect_height-4);
}

void FileViewInteractiveRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	if (this->m_fileminimap->mainWidget->mViewMode == 1)		// if view mode is FILE_VIEW
	{	
		rect_active = true;
		m_fileminimap->setCursor(Qt::PointingHandCursor);
		QGraphicsItem::hoverEnterEvent(event);
	}
}

void FileViewInteractiveRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	rect_active = false;
	m_fileminimap->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}

void FileViewInteractiveRect::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsItem::hoverMoveEvent(event);
}

void FileViewInteractiveRect::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (this->m_fileminimap->mainWidget->mViewMode == 1)		// if view mode is FILE_VIEW
	{
		m_fileminimap->setCursor(Qt::ClosedHandCursor);
		QGraphicsItem::mousePressEvent(event);
	}
}

// override mouse movement
void FileViewInteractiveRect::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
	if (rect_active)
	{
		updateIntRectPos(event);
	}
}
void FileViewInteractiveRect::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )
{
	QGraphicsItem::dragLeaveEvent ( event );
	rect_active = false;
}
void FileViewInteractiveRect::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
	QGraphicsItem::mouseReleaseEvent ( event );
	if (rect_active == false)
		m_fileminimap->setCursor(Qt::ArrowCursor);
	else m_fileminimap->setCursor(Qt::PointingHandCursor);
}
void FileViewInteractiveRect::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
}
// calculate size of navigator rectangle
void FileViewInteractiveRect::updateIntRect()
{
	if (!m_fileminimap->isEnabled())
		return;
	// get view rect in scene coordinates
	QPointF item_topleft = m_fileminimap->mFileView->mapToScene(m_fileminimap->mFileView->rect()).boundingRect().topLeft();
	QPointF item_bottomright = m_fileminimap->mFileView->mapToScene(m_fileminimap->mFileView->rect()).boundingRect().bottomRight();
	
	// clip at scene borders
	if (item_topleft.y() < m_fileminimap->mFileView->sceneRect().topLeft().y())
	{	
		//qDebug() << "rect above scene !";
		item_topleft.setY(m_fileminimap->mFileView->sceneRect().topLeft().y());
	}
	if (item_bottomright.y() > m_fileminimap->mFileView->sceneRect().bottomRight().y())
	{	
		//qDebug() << "rect below scene !";
		item_bottomright.setY(m_fileminimap->mFileView->sceneRect().bottomRight().y());
	}
	if (item_topleft.x() < m_fileminimap->mFileView->sceneRect().topLeft().x())
	{	
		//qDebug() << "rect too much on left !";
		item_topleft.setX(m_fileminimap->mFileView->sceneRect().topLeft().x());

	}
	if (item_bottomright.x() > m_fileminimap->mFileView->sceneRect().bottomRight().x())
	{	
		//qDebug() << "rect too much on right !";
		item_bottomright.setX(m_fileminimap->mFileView->sceneRect().bottomRight().x());
	}

	rect_height = item_bottomright.y() - item_topleft.y();
	rect_width = item_bottomright.x() - item_topleft.x();

	QPointF newpos;
	this->setPos(item_topleft.x() + ( rect_width / 2 ), item_topleft.y() + ( rect_height / 2 ) );

	// update content
	m_fileminimap->update(m_fileminimap->rect());
	this->update(this->boundingRect());
}
void FileViewInteractiveRect::updateIntRectPos( QGraphicsSceneMouseEvent * event)
{
	// calculate position

	QPointF oldpos = this->pos();
	QPointF newpos(oldpos);
	newpos.setX(newpos.x()+(event->pos().x()-event->lastPos().x()));
	newpos.setY(newpos.y()+(event->pos().y()-event->lastPos().y()));
	this->setPos(newpos);
	
	if (this->mapRectToScene(this->boundingRect()).topLeft().y() < m_fileminimap->sceneRect().topLeft().y())
	//if (this->scenePos().y() - rect_height/2 < -m_fileminimap->sceneRect().height()/2)
	{
		// item moved over top border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) < 0)	// mouse moving to top
		{
			qreal corrected_position = m_fileminimap->sceneRect().topLeft().y() + rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}
	else if (this->mapRectToScene(this->boundingRect()).bottomLeft().y() > m_fileminimap->sceneRect().bottomLeft().y())
	{
		// item moved over bottom border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) > 0)	// mouse moving to buttom
		{
			qreal corrected_position = m_fileminimap->sceneRect().bottomLeft().y() - rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}
	if (this->mapRectToScene(this->boundingRect()).topLeft().x() < m_fileminimap->sceneRect().topLeft().x())
	{
		// item moved over left border -> limit movement
		if ((event->pos().x()-event->lastPos().x()) < 0)	// mouse moving to left
		{
			qreal corrected_position = m_fileminimap->sceneRect().topLeft().x() + rect_width/2;
			newpos.setX(corrected_position);
			this->setPos(newpos);
		}
	}
	else if (this->mapRectToScene(this->boundingRect()).topRight().x() > m_fileminimap->sceneRect().topRight().x())
	{
		// item moved over right border -> limit movement
		if ((event->pos().x()-event->lastPos().x()) > 0)	// mouse moving to right
		{
			qreal corrected_position = m_fileminimap->sceneRect().topRight().x() - rect_width/2;
			newpos.setX(corrected_position);
			this->setPos(newpos);
		}
	}
	
	// calculate corresponding scroll bar position for file view
	qreal total_dist_y = m_fileminimap->sceneRect().height()-rect_height;
	qreal diff_y = m_fileminimap->sceneRect().topLeft().y() + rect_height / 2;
	qreal y_val = ( scenePos().y() - diff_y ) / total_dist_y;

	qreal total_dist_x = m_fileminimap->sceneRect().width()-rect_width;
	qreal diff_x = m_fileminimap->sceneRect().topLeft().x() + rect_width / 2;
	qreal x_val = ( scenePos().x() - diff_x ) / total_dist_x;
	
	// set scroll position
	m_fileminimap->mFileView->setScrollBarValue(x_val, y_val);

	// with new position update file view
	m_fileminimap->update(m_fileminimap->rect());
	this->update(this->boundingRect());
}
void FileViewInteractiveRect::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	QGraphicsItem::wheelEvent(event);
}
void FileViewInteractiveRect::setRectInScene(QPointF item_topleft, QPointF item_bottomright)
{
	//qDebug() << "setRectInScene";

	// clip at scene borders
	if (item_topleft.y() < m_fileminimap->mFileView->sceneRect().topLeft().y())
	{	
		//qDebug() << "rect above scene !";
		item_topleft.setY(m_fileminimap->mFileView->sceneRect().topLeft().y());
	}
	if (item_bottomright.y() > m_fileminimap->mFileView->sceneRect().bottomRight().y())
	{	
		//qDebug() << "rect below scene !";
		item_bottomright.setY(m_fileminimap->mFileView->sceneRect().bottomRight().y());
	}
	if (item_topleft.x() < m_fileminimap->mFileView->sceneRect().topLeft().x())
	{	
		//qDebug() << "rect too much on left !";
		item_topleft.setX(m_fileminimap->mFileView->sceneRect().topLeft().x());

	}
	if (item_bottomright.x() > m_fileminimap->mFileView->sceneRect().bottomRight().x())
	{	
		//qDebug() << "rect too much on right !";
		item_bottomright.setX(m_fileminimap->mFileView->sceneRect().bottomRight().x());
	}

	rect_height = item_bottomright.y() - item_topleft.y();
	rect_width = item_bottomright.x() - item_topleft.x();

	QPointF newpos;
	this->setPos(item_topleft.x() + ( rect_width / 2 ), item_topleft.y() + ( rect_height / 2 ) );

	// update content
	m_fileminimap->update(m_fileminimap->rect());
	this->update(this->boundingRect());
}