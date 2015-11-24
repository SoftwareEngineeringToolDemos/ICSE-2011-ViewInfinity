

#pragma once

#ifndef FILEVIEWINTERACTIVERECT_H
#define FILEVIEWINTERACTIVERECT_H

class FileMiniMap;

class FileViewInteractiveRect : public QGraphicsItem
{
public:
	FileViewInteractiveRect(FileMiniMap *minimap);

	QRectF			boundingRect() const;
	void			paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void			updateIntRect();
	//void			updateIntRectFromView();
	void			updateIntRectPos( QGraphicsSceneMouseEvent * event);

	void			setRectInScene(QPointF item_topleft, QPointF item_bottomright);

	qreal			rect_height;	// height of rectangle in item coordinates
	qreal			rect_width;		// width of rectangle in item coordinates

	bool			rect_active;

protected:
	void			hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void			hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void			mousePressEvent(QGraphicsSceneMouseEvent* event);
	void			mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	void			mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event);
	void			dragLeaveEvent ( QGraphicsSceneDragDropEvent * event );
	void			mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	void			wheelEvent(QGraphicsSceneWheelEvent *event);
	void			hoverMoveEvent(QGraphicsSceneHoverEvent* event);
	//QVariant		FileViewInteractiveRect::itemChange(GraphicsItemChange change, const QVariant &value);

private:
	FileMiniMap	*m_fileminimap;
};

#endif