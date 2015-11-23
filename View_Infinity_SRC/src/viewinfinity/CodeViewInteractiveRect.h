

#pragma once

#ifndef CODEVIEWINTERACTIVERECT_H
#define CODEVIEWINTERACTIVERECT_H

class CodeMiniMap;

class CodeViewInteractiveRect : public QGraphicsItem
{
public:
	CodeViewInteractiveRect(CodeMiniMap *minimap);

	QRectF			boundingRect() const;
	void			paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void			updateIntRectHeight();
	void			updateIntRectPosFromEditor();
	void			updateIntRectPos( QGraphicsSceneMouseEvent * event);

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
	//QVariant		CodeViewInteractiveRect::itemChange(GraphicsItemChange change, const QVariant &value);

private:
	CodeMiniMap	*m_codeminimap;
};

#endif