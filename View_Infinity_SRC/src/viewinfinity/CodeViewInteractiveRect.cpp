

#include <QGraphicsItem>
#include <QPainter>
#include "CodeMiniMap.h"
#include "CodeView.h"
#include "CodeViewInteractiveRect.h"


CodeViewInteractiveRect::CodeViewInteractiveRect(CodeMiniMap *minimap)
    : m_codeminimap(minimap)
{
	setFlags(ItemIsMovable);
	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

	rect_active = false;

	//rect_height = 0.0;
	//rect_width = 0.0;
}

/*
QVariant CodeViewInteractiveRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionChange)
	{
	
	}
	return QGraphicsItem::itemChange(change, value);
}

*/
QRectF CodeViewInteractiveRect::boundingRect() const
{
 // get Bounding Rectangle
 return QRectF(-rect_width/2, -rect_height/2, rect_width, rect_height);
/*
return QRectF(
	this->mapFromScene(m_codeminimap->mapToScene(m_codeminimap->viewport()->rect().topLeft())),
	this->mapFromScene(m_codeminimap->mapToScene(m_codeminimap->viewport()->rect().bottomRight())));
*/

}

void CodeViewInteractiveRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{
	painter->setPen(QPen(QBrush(Qt::red,Qt::BrushStyle(Qt::SolidLine)),2,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
	//painter->fillRect(-rect_width/2, -rect_height/2, rect_width, rect_height,QBrush(QColor::fromRgbF(0, 1, 0, 0.5)));
	painter->drawRect(-rect_width/2+2, -rect_height/2+2, rect_width-4, rect_height-4);
}

void CodeViewInteractiveRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	rect_active = true;
	m_codeminimap->setCursor(Qt::PointingHandCursor);
	QGraphicsItem::hoverEnterEvent(event);
}

void CodeViewInteractiveRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	rect_active = false;
	m_codeminimap->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}

void CodeViewInteractiveRect::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	QGraphicsItem::hoverMoveEvent(event);
}

void CodeViewInteractiveRect::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_codeminimap->setCursor(Qt::ClosedHandCursor);
	QGraphicsItem::mousePressEvent(event);
}

// override mouse movement
void CodeViewInteractiveRect::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
	if (rect_active)
	{
		updateIntRectPos(event);
	}
}
void CodeViewInteractiveRect::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )
{
	QGraphicsItem::dragLeaveEvent ( event );
	rect_active = false;
}
void CodeViewInteractiveRect::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
	QGraphicsItem::mouseReleaseEvent ( event );
	if (rect_active == false)
		m_codeminimap->setCursor(Qt::ArrowCursor);
	else m_codeminimap->setCursor(Qt::PointingHandCursor);
}
void CodeViewInteractiveRect::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
}

void CodeViewInteractiveRect::updateIntRectHeight()
{
	// calculate height

	// get total number of lines
	int loc = m_codeminimap->mCodeView->document()->lineCount();
	
	QPoint origin(0,0);
	QTextCursor curs_origin = m_codeminimap->mCodeView->cursorForPosition(origin);
	int start_loc = m_codeminimap->mCodeView->document()->findBlock(curs_origin.position()).firstLineNumber()+1;
	origin.setY(m_codeminimap->mCodeView->rect().height());
	curs_origin = m_codeminimap->mCodeView->cursorForPosition(origin);
	
	QTextBlock block = m_codeminimap->mCodeView->document()->findBlock(curs_origin.position());
	
	int end_loc;
	if (block.blockNumber()+1 == m_codeminimap->mCodeView->document()->blockCount()) // last block
	{
		end_loc = loc+1;
	}
	else
	{
		end_loc = m_codeminimap->mCodeView->document()->findBlock(curs_origin.position()).firstLineNumber()+2;
	}

	rect_height = (qreal(end_loc)-qreal(start_loc))/qreal(loc) * qreal(scene()->sceneRect().height());

	// update content
	m_codeminimap->update(m_codeminimap->rect());
	this->update(this->boundingRect());
	
}
void CodeViewInteractiveRect::updateIntRectPosFromEditor()
{
	// 1: get upper y

	// get total number of lines
	int loc = m_codeminimap->mCodeView->document()->lineCount();
	QPoint origin(0,0);
	QTextCursor curs_origin = m_codeminimap->mCodeView->cursorForPosition(origin);
	int start_loc = m_codeminimap->mCodeView->document()->findBlock(curs_origin.position()).firstLineNumber();
	qreal y = qreal(start_loc) / qreal(loc) * m_codeminimap->sceneRect().height() - m_codeminimap->sceneRect().height()/2 + rect_height/2;
	// 2: set position
	this->setPos(this->pos().x(),y);

	// TODO check rectangle to fit in view	
	
	// 3: update content
	m_codeminimap->update(m_codeminimap->rect());
	this->update(this->boundingRect());
}

void CodeViewInteractiveRect::updateIntRectPos( QGraphicsSceneMouseEvent * event)
{
	// calculate position

	QPointF oldpos = this->pos();
	QPointF newpos(oldpos);
	newpos.setY(newpos.y()+(event->pos().y()-event->lastPos().y()));
	this->setPos(newpos);

	if (this->scenePos().y() - rect_height/2 < -m_codeminimap->sceneRect().height()/2)
	{
		// item moved over top border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) < 0)
		{
			qreal corrected_position = -m_codeminimap->sceneRect().height()/2 + rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}
	else if (this->scenePos().y() + rect_height/2 > m_codeminimap->sceneRect().height()/2)
	{
		// item moved over bottom border -> limit movement
		if ((event->pos().y()-event->lastPos().y()) > 0)
		{
			qreal corrected_position = m_codeminimap->sceneRect().height()/2 - rect_height/2;
			newpos.setY(corrected_position);
			this->setPos(newpos);
		}
	}

	

	// TODO: scroll up or down to show next text content

	// calculate corresponding scroll bar position for code view
	qreal total_dist = m_codeminimap->sceneRect().height()-rect_height;
	qreal val = (scenePos().y() + (total_dist/2)) / total_dist;
	m_codeminimap->mCodeView->setScrollBarValue(val);

#if 0

	qreal y = this->pos().y() - rect_height / 2;
	// check if rect is fully visible
	qreal calc_rect_height = mapToScene(QPointF(0,rect_height)).y() - this->mapToScene(QPointF(0,0)).y();
	// check if rect is too high
	qreal top_y = m_codeminimap->mapToScene(QPoint(0,0)).y();
	qreal bottom_y = m_codeminimap->mapToScene(QPoint(0,m_codeminimap->height())).y();
	if ((top_y - y) > 0.0)	// rect is too high
	{
		// correct position
		qreal corrected_position = top_y + rect_height/2;
		newpos.setY(corrected_position);
		this->setPos(newpos);

		// update content
		m_codeminimap->update(m_codeminimap->rect());
		this->update(this->boundingRect());

		return;

		// TODO: limit movement !!!

		// can we scroll up?
		/*
		if (m_codeminimap->mCodeView->verticalScrollBar()->maximum() > m_codeminimap->mCodeView->verticalScrollBar()->minimum())
		{
			qreal correctedvalue = qreal(m_codeminimap->mCodeView->verticalScrollBar()->value())-qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum());
			qreal interval = qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum()) - qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum());
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << " calculated value " << val;
			val = val - 0.001;
			if (val < 0.0) val = 0.0;
			qDebug() << " set scroll value to " << val;
			m_codeminimap->mCodeView->setScrollBarValue(val);
			return;
		}
		else if (m_codeminimap->mCodeView->verticalScrollBar()->minimum() > m_codeminimap->mCodeView->verticalScrollBar()->maximum())
		{
			qreal correctedvalue = qreal(m_codeminimap->mCodeView->verticalScrollBar()->value())-qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum());
			qreal interval = qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum()) - qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum());
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << " calculated value " << val;
			val -= 0.001;
			if (val < 0.0) val = 0.0;
			qDebug() << " set scroll value to " << val;
			m_codeminimap->mCodeView->setScrollBarValue(val);
			return;
		}
		*/
	}

	// check if rect is too low
	if (((y+rect_height) - bottom_y) > 0.0)
	{
		qDebug() << "rect too low!!!";

		qreal corrected_position = bottom_y - rect_height/2;
		newpos.setY(corrected_position);
		this->setPos(newpos);

		// TODO: limit movement !!!

		/*
		// can we scroll down?
		if (m_codeminimap->mCodeView->verticalScrollBar()->maximum() > m_codeminimap->mCodeView->verticalScrollBar()->minimum())
		{
			qreal correctedvalue = qreal(m_codeminimap->mCodeView->verticalScrollBar()->value())-qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum());
			qreal interval = qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum()) - qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum());
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << " calculated value " << val;
			val += 0.001;
			if (val > 1.0) val = 1.0;
			qDebug() << " set scroll value to " << val;
			m_codeminimap->mCodeView->setScrollBarValue(val);
			return;
		}
		else if (m_codeminimap->mCodeView->verticalScrollBar()->minimum() > m_codeminimap->mCodeView->verticalScrollBar()->maximum())
		{
			qreal correctedvalue = qreal(m_codeminimap->mCodeView->verticalScrollBar()->value())-qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum());
			qreal interval = qreal(m_codeminimap->mCodeView->verticalScrollBar()->minimum()) - qreal(m_codeminimap->mCodeView->verticalScrollBar()->maximum());
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << " calculated value " << val;
			val += 0.001;
			if (val > 1.0) val = 1.0;
			qDebug() << " set scroll value to " << val;
			m_codeminimap->mCodeView->setScrollBarValue(val);
			return;
		}
		*/
	}
	
	
	qreal val = (pos().y()-(rect_height/2) + (m_codeminimap->sceneRect().height()/2)) / m_codeminimap->sceneRect().height();

	qDebug() << "value = " << val;
	m_codeminimap->mCodeView->setScrollBarValue(val);

#endif

	// with new position update code view
	m_codeminimap->update(m_codeminimap->rect());
	this->update(this->boundingRect());
}
void CodeViewInteractiveRect::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	QGraphicsItem::wheelEvent(event);
}