

#pragma once

#ifndef FILEVIEWNODEGRAPHIC_H
#define FILEVIEWNODEGRAPHIC_H

#include <QtGui>
#include <QtGlobal>
#include <QGraphicsItem>
#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
//#include "SMVMain.h"

class SMVMain;

class FileViewNodeGraphic : public QGraphicsItem
{
public:
	FileViewNodeGraphic(SMVMain *graphWidget);

	QRectF			boundingRect() const;
	void			paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void			setFileGraphNode(ogdf::node n);
	ogdf::node		getFileGraphNode();
	int				getObjectWidth();
	bool			setObjectWidth(int w);
	int				getObjectHeight();
	bool			setObjectHeight(int h);

	bool			setLabel(QString text);
	QString			Label();
	bool			setType(int nodetype);
	int				Type();
	void			setGreyed(bool is_gray);
	bool			isGreyed();
	void			calculateDimensions();
	void			createFragmentRects(unsigned int off,
		unsigned int len, qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
		std::vector<bool> activeStatus);

	void			createHistogramRects(unsigned int off,
		unsigned int len, qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
		std::vector<bool> activeStatus);

	void			hideText();
	void			showText();
	void			hideIcon();
	void			showIcon();

	void			updateLOD();
	void			setToolTipContent(QString str);

	void			setNodePixmap(QPixmap pm);

	std::vector<unsigned int>					offset;
	std::vector<unsigned int>					length;
	std::vector<int>							fragYStart;
	std::vector<int>							fragYEnd;
	std::vector<std::vector<QColor>>			fragmentColors;
	std::vector<std::vector<QString>>			fragmentNames;
	std::vector<std::vector<bool>>				isActive;
	bool										greyed;

	std::vector<unsigned int>					histogram_offset;
	std::vector<unsigned int>					histogram_length;
	std::vector<qreal>							histogram_yStart;
	std::vector<qreal>							histogram_yEnd;
	std::vector<std::vector<QColor>>			histogram_Colors;
	std::vector<std::vector<QString>>			histogram_Names;
	std::vector<std::vector<bool>>				histogram_isActive;

	QPixmap					codePreviewPixmap;

protected:
	void			hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void			hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void			hoverMoveEvent(QGraphicsSceneHoverEvent* event);
	void			mousePressEvent(QGraphicsSceneMouseEvent* event);
	void			mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event);
	QVariant FileViewNodeGraphic::itemChange(GraphicsItemChange change, const QVariant &value);

 private:
	SMVMain						*graph;
	ogdf::node					mNode;
	int						mHeight, mWidth, mHalfHeight, mHalfWidth;
	QGraphicsSimpleTextItem	*textitem;
	QGraphicsPixmapItem		*icon;
	QColor						mLineColor;

	QString					label;
	int						type;
	bool					showPreviewPixmap;
	int						pixmap_height, pixmap_y_start;
	int						pixmap_width, pixmap_x_start;



};

#endif