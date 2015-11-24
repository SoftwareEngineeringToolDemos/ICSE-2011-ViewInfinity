

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsItem>

#include "SMVMain.h"
#include "FileView.h"
#include "FileMiniMap.h"
#include "FileMiniMapNodeGraphic.h"
#include "FileTreeModel.h"


const int MARGIN				= 5;
const int PREVIEW_MARGIN		= 2;

FileMiniMapNodeGraphic::FileMiniMapNodeGraphic(SMVMain *graphWidget)
    : graph(graphWidget)
{
	//setFlags(ItemIsMovable);
	//this->setAcceptHoverEvents(true);
	//this->setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

	// initialize node icon
	//pixmap.load(EMPTY_ICON);
	icon = new QGraphicsPixmapItem(this);
	showPreviewPixmap = false;
	//icon->setPixmap(pixmap);

	// initialize node label
	textitem = new QGraphicsSimpleTextItem(this);

	greyed = false;
}	

QVariant FileMiniMapNodeGraphic::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//if (change == ItemPositionChange)
	if (change == ItemPositionHasChanged)
	{
		graph->updateFileMiniMapNodePosition(mNode);
	}
	return QGraphicsItem::itemChange(change, value);
}

void FileMiniMapNodeGraphic::setFileGraphNode(ogdf::node n)
{
	mNode = n;
}

ogdf::node FileMiniMapNodeGraphic::getFileGraphNode()
{
	return mNode;
}

int FileMiniMapNodeGraphic::getObjectWidth()
{
	return mWidth;
}

bool FileMiniMapNodeGraphic::setObjectWidth(int w)
{
	if (w>0)
	{
		mWidth = w;
		mHalfWidth = mWidth / 2;
		return true;
	}
	return false;
}

int FileMiniMapNodeGraphic::getObjectHeight()
{
	return mHeight;
}

bool FileMiniMapNodeGraphic::setObjectHeight(int h)
{
	if (h>0)
	{
		mHeight = h;
		mHalfHeight = mHeight / 2;
		return true;
	}
	return false;
}

bool FileMiniMapNodeGraphic::setLabel(QString text)
{
	label = text;
	textitem->setText(label);
	return true;
}

QString	FileMiniMapNodeGraphic::Label()
{
	return label;
}

void  FileMiniMapNodeGraphic::setNodePixmap(QPixmap pm)
{
	codePreviewPixmap = pm;
	if (pm.size().width() > 0)
	{
		showPreviewPixmap = true;
		qreal pixmap_ratio = (double)codePreviewPixmap.width() / (double)codePreviewPixmap.height();
		//qDebug() << "pixmap ratio = " << pixmap_ratio;
		if (pixmap_ratio > 1.0)	// pixmap is wider than high
		{
			pixmap_x_start = (int)(this->boundingRect().topLeft().x()) + PREVIEW_MARGIN;
			pixmap_width = (int)(this->boundingRect().width())-(2*PREVIEW_MARGIN);
			pixmap_height = (int)(pixmap_width/pixmap_ratio);
			pixmap_y_start = (int)((this->boundingRect().height()-(qreal)pixmap_height)/2 + this->boundingRect().topLeft().y());
		}
		else // pixmap is higher than wide
		{
			pixmap_y_start = (int)(this->boundingRect().topLeft().y()) + PREVIEW_MARGIN;
			pixmap_height = (int)(this->boundingRect().height())-(2*PREVIEW_MARGIN);
			pixmap_width = (int)(pixmap_height*pixmap_ratio);
			pixmap_x_start = (int)((this->boundingRect().width()-(qreal)pixmap_width)/2 + this->boundingRect().topLeft().x());

		}
	}
}

bool FileMiniMapNodeGraphic::setType(int nodetype)
{
	if (nodetype >= 0)
	{
		type = nodetype;
		
		// project
		if (type == 0)
		{
			mLineColor = Qt::blue;
			QPixmap pm = *graph->icons[0].icon_pm;
			icon->setPixmap(pm.scaledToWidth(64,Qt::SmoothTransformation));
		}
		// file
		else if (type == 1)
		{
			mLineColor = Qt::darkGreen;
			//QPixmap pm = *graph->icons[1].icon_pm;
			//icon->setPixmap(pm.scaledToWidth(64,Qt::SmoothTransformation));
		}
		// folder
		else if (type == 2)
		{		
			mLineColor = Qt::darkRed;
			QPixmap pm = *graph->icons[1].icon_pm;
			icon->setPixmap(pm.scaledToWidth(64,Qt::SmoothTransformation));
		}
		if (this->greyed)
			mLineColor = Qt::lightGray;
		return true;
	}
	return false;
}

int FileMiniMapNodeGraphic::Type()
{
	return type;
}

void FileMiniMapNodeGraphic::setGreyed(bool is_gray)
{
	greyed = is_gray;
	setType(this->type);
}
bool FileMiniMapNodeGraphic::isGreyed()
{
	return greyed;
}

void FileMiniMapNodeGraphic::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{
	painter->setClipRect( option->exposedRect );

	if (graph->mFileMiniMap->getLOD() == 0)	// fill node
	{
		painter->setBrush(QBrush(mLineColor, Qt::SolidPattern));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		// draw folding symbol border
		if ((type == 0) || (type == 2))
		{
			painter->setPen(QPen(Qt::white, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth, mHalfHeight - 10);
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth - 10, mHalfHeight);
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 7, mHalfHeight - 5, mHalfWidth - 3, mHalfHeight - 5);
			if (graph->mFileNodeCollapsed[mNode])
			{
				painter->drawLine(mHalfWidth - 5, mHalfHeight - 7, mHalfWidth - 5, mHalfHeight - 3);
			}
		}

		return;
	}
	else if (graph->mFileMiniMap->getLOD() == 1) // draw node border with icon only
	{
		// draw border
		painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		// draw folding symbol border
		if ((type == 0) || (type == 2))
		{
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth, mHalfHeight - 10);
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth - 10, mHalfHeight);
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 7, mHalfHeight - 5, mHalfWidth - 3, mHalfHeight - 5);
			if (graph->mFileNodeCollapsed[mNode])
			{
				painter->drawLine(mHalfWidth - 5, mHalfHeight - 7, mHalfWidth - 5, mHalfHeight - 3);
			}
		}
		else if (type == 1)
		{
			if (histogram_yStart.size() > 0)
			{
				// for each fragment
				for (unsigned int i = 0; i < histogram_yStart.size(); i++)
				{
					qreal divider = 1./qreal(histogram_Colors[i].size());
					for (unsigned int j = 0; j < histogram_Colors[i].size(); j++)
					{
						if (this->histogram_isActive[i][j] == true)
						{
							painter->setPen(QPen(histogram_Colors[i][j], 2,Qt::NoPen));
							painter->setBrush(QBrush(histogram_Colors[i][j], Qt::SolidPattern));
							QPointF tL(-qreal(mHalfWidth),histogram_yStart[i]);
							QPointF sizeRect(qreal(mWidth),histogram_yEnd[i]);
							// debug
							/*
							qDebug(label.toLatin1());
							qDebug("draw rect from");
							qDebug(QString::number(tL.x()).toLatin1());
							qDebug(QString::number(tL.y()).toLatin1());
							qDebug("with width:");
							qDebug(QString::number(sizeRect.x()).toLatin1());
							qDebug(QString::number(sizeRect.y()).toLatin1());
							

							// debug
							//for (unsigned int i = 0; i < colors.size(); i++)
							//{
							qDebug("in color");
							QString str;
							str.append(QString::number(fragmentColors[i][j].red()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].green()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].blue()));
							qDebug(str.toLatin1());
							//}
							*/
							
							//QRectF rec(tL,sizeRect);
							//painter->drawRect(rec);
							painter->drawRoundRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y(),1,1);
						}
						else
						{
							painter->setPen(QPen(Qt::lightGray, 2,Qt::NoPen));
							painter->setBrush(QBrush(Qt::gray, Qt::SolidPattern));
							QPointF tL(-qreal(mHalfWidth),histogram_yStart[i]);
							QPointF sizeRect(qreal(mWidth),histogram_yEnd[i]);
							painter->drawRoundRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y(),1,1);
						}
					}
				}
			}
			else		// draw file not found
			{
				painter->setBrush(QBrush(mLineColor, Qt::SolidPattern));
				painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
			}
		}

		// draw border
		painter->setBrush(Qt::NoBrush);
		if (greyed)
			painter->setPen(QPen(Qt::darkGray, 3, Qt::SolidLine));
		else painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		return;
	}
	else if ((graph->mFileMiniMap->getLOD() == 2) || (this->greyed == true)) // draw node border with icon and text
	{
		// draw border
		painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		// draw folding symbol border
		if ((type == 0) || (type == 2))
		{
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth, mHalfHeight - 10);
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth - 10, mHalfHeight);
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 7, mHalfHeight - 5, mHalfWidth - 3, mHalfHeight - 5);
			if (graph->mFileNodeCollapsed[mNode])
			{
				painter->drawLine(mHalfWidth - 5, mHalfHeight - 7, mHalfWidth - 5, mHalfHeight - 3);
			}
		}
		else if ( (type == 1) && (!greyed) )
		{
			if (histogram_yStart.size() > 0)
			{
				// for each fragment
				for (unsigned int i = 0; i < histogram_yStart.size(); i++)
				{
					qreal divider = 1./qreal(histogram_Colors[i].size());
					for (unsigned int j = 0; j < histogram_Colors[i].size(); j++)
					{
						if (this->histogram_isActive[i][j] == true)
						{
							painter->setPen(QPen(histogram_Colors[i][j], 2,Qt::NoPen));
							painter->setBrush(QBrush(histogram_Colors[i][j], Qt::SolidPattern));
							//QPointF tL(-qreal(mHalfWidth)+(qreal(j)*divider*qreal(mWidth)),histogram_yEnd[i]);
							//QPointF sizeRect(divider*qreal(mWidth),histogram_yStart[i]);
							QPointF tL(-qreal(mHalfWidth),histogram_yStart[i]);
							QPointF sizeRect(qreal(mWidth),histogram_yEnd[i]);
							// debug
							/*
							qDebug(label.toLatin1());
							qDebug("draw rect from");
							qDebug(QString::number(tL.x()).toLatin1());
							qDebug(QString::number(tL.y()).toLatin1());
							qDebug("with width:");
							qDebug(QString::number(sizeRect.x()).toLatin1());
							qDebug(QString::number(sizeRect.y()).toLatin1());
							

							// debug
							//for (unsigned int i = 0; i < colors.size(); i++)
							//{
							qDebug("in color");
							QString str;
							str.append(QString::number(fragmentColors[i][j].red()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].green()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].blue()));
							qDebug(str.toLatin1());
							//}
							*/
							
							//QRectF rec(tL,sizeRect);
							//painter->drawRect(rec);
							//painter->drawRoundRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y(),1,1);
							painter->drawRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y());
						}
						else
						{
							painter->setPen(QPen(Qt::lightGray, 2,Qt::NoPen));
							painter->setBrush(QBrush(Qt::gray, Qt::SolidPattern));
							QPointF tL(-qreal(mHalfWidth),histogram_yStart[i]);
							QPointF sizeRect(qreal(mWidth),histogram_yEnd[i]);
							painter->drawRoundRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y(),1,1);
						}
					}
				}

				painter->setPen(Qt::NoPen);
				painter->setBrush(QBrush(Qt::white));
				QPointF textitem_rect_topleft(textitem->pos().x(),
					textitem->pos().y());
				QRectF textitem_rect(textitem_rect_topleft, textitem->boundingRect().size());
				painter->drawRect(textitem_rect);

			}
			else		// draw file not found
			{
				painter->setBrush(QBrush(mLineColor, Qt::SolidPattern));
				painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
			}
		}

		// draw border
		painter->setBrush(Qt::NoBrush);
		if (greyed)
			painter->setPen(QPen(Qt::darkGray, 3, Qt::SolidLine));
		else painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		return;
	}
	else if (graph->mFileMiniMap->getLOD() == 3) // draw node border with code previews
	{


		// draw folding symbol border for folders and projects
		if ((type == 0) || (type == 2))
		{
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth, mHalfHeight - 10);
			painter->drawLine(mHalfWidth - 10, mHalfHeight - 10, mHalfWidth - 10, mHalfHeight);
			painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
			painter->drawLine(mHalfWidth - 7, mHalfHeight - 5, mHalfWidth - 3, mHalfHeight - 5);
			if (graph->mFileNodeCollapsed[mNode])
			{
				painter->drawLine(mHalfWidth - 5, mHalfHeight - 7, mHalfWidth - 5, mHalfHeight - 3);
			}
		}
		else if ( (type == 1)  && (!greyed) ) // draw code preview for files
		{

			if (showPreviewPixmap == true)
			{
				//QPixmap pm = codePreviewPixmap.scaled(int(this->boundingRect().width()),int(this->boundingRect().height()),Qt::KeepAspectRatio,Qt::FastTransformation);
				

				//qDebug() << "width of pixmap " << codePreviewPixmap.width();
				
				//QPixmap pm(codePreviewPixmap.scaled(int(this->boundingRect().width()),int(this->boundingRect().height()),Qt::KeepAspectRatio,Qt::SmoothTransformation));
				painter->fillRect(this->boundingRect(),QBrush(Qt::white));
				//painter->drawPixmap(this->boundingRect().topLeft(),pm);
				//painter->drawPixmap(int(this->boundingRect().topLeft().x()),int(this->boundingRect().topLeft().y()), codePreviewPixmap.scaled(int(this->boundingRect().width()),int(this->boundingRect().height()),Qt::KeepAspectRatio,Qt::SmoothTransformation));
				/*
				painter->drawPixmap(
					int(this->boundingRect().topLeft().x()),
					int(this->boundingRect().topLeft().y()),
					int(this->boundingRect().width()),
					int(this->boundingRect().height()),
					codePreviewPixmap);
				*/
				painter->drawPixmap(pixmap_x_start, pixmap_y_start, pixmap_width, pixmap_height, codePreviewPixmap);
					
				/*
				painter->drawPixmap(
					int(this->boundingRect().topLeft().x()),
					int(this->boundingRect().topLeft().y()),
					int(this->boundingRect().width()),
					int(this->boundingRect().height()),
					codePreviewPixmap.scaledToWidth(this->boundingRect().width(),Qt::SmoothTransformation));
				*/

				painter->setPen(Qt::NoPen);
				painter->setBrush(QBrush(Qt::white));
				QPointF textitem_rect_topleft(textitem->pos().x(),
					textitem->pos().y());
				QRectF textitem_rect(textitem_rect_topleft, textitem->boundingRect().size());
				painter->drawRect(textitem_rect);
			}
			else		// draw file not found
			{

			}

#if 0
			if (fragYStart.size() > 0)
			{
				// for each fragment
				for (unsigned int i = 0; i < fragYStart.size(); i++)
				{
					qreal divider = 1./qreal(fragmentColors[i].size());
					for (unsigned int j = 0; j < fragmentColors[i].size(); j++)
					{
						if (this->isActive[i][j] == true)
						{
							painter->setPen(QPen(fragmentColors[i][j], 2,Qt::NoPen));
							painter->setBrush(QBrush(fragmentColors[i][j], Qt::SolidPattern));
							QPointF tL(-qreal(mHalfWidth)+(qreal(j)*divider*qreal(mWidth)),fragYStart[i]);
							QPointF sizeRect(divider*qreal(mWidth),fragYEnd[i]);
							// debug
							/*
							qDebug(label.toLatin1());
							qDebug("draw rect from");
							qDebug(QString::number(tL.x()).toLatin1());
							qDebug(QString::number(tL.y()).toLatin1());
							qDebug("with width:");
							qDebug(QString::number(sizeRect.x()).toLatin1());
							qDebug(QString::number(sizeRect.y()).toLatin1());
							

							// debug
							//for (unsigned int i = 0; i < colors.size(); i++)
							//{
							qDebug("in color");
							QString str;
							str.append(QString::number(fragmentColors[i][j].red()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].green()));
							str.append("/");
							str.append(QString::number(fragmentColors[i][j].blue()));
							qDebug(str.toLatin1());
							//}
							*/
							
							//QRectF rec(tL,sizeRect);
							//painter->drawRect(rec);
							painter->drawRoundRect(tL.x(), tL.y(),sizeRect.x(), sizeRect.y(),1,1);
						}
					}
				}
			}
#endif
		}

	// debug: draw bounding rect
	/*
	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
	QLine line;
	line.setPoints(this->boundingRect().topLeft().toPoint(),this->boundingRect().topRight().toPoint());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().topRight().toPoint(),this->boundingRect().bottomRight().toPoint());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().bottomRight().toPoint(),this->boundingRect().bottomLeft().toPoint());
	painter->drawLine(line);
	line.setPoints(this->boundingRect().bottomLeft().toPoint(),this->boundingRect().topLeft().toPoint());
	painter->drawLine(line);
	*/

	// draw border
	painter->setBrush(Qt::NoBrush);
	if (greyed)
		painter->setPen(QPen(Qt::darkGray, 3, Qt::SolidLine));
	else painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
	painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

	return;
	}
	
	// Debug
	//painter->drawRect(int(icon->pos().x()),int(icon->pos().y()),icon->boundingRect().width(),icon->boundingRect().height());
	//painter->drawRect(int(textitem->pos().x()), int(textitem->pos().y()), textitem->boundingRect().width(), textitem->boundingRect().height());

	return;
}

void FileMiniMapNodeGraphic::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	//graph->showToolTip(this->mNode, this->mapToParent(this->pos()));
	QGraphicsItem::hoverEnterEvent(event);
}

void FileMiniMapNodeGraphic::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	graph->mFileMiniMap->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}

void FileMiniMapNodeGraphic::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if ((type == 0) || (type == 2))
	{
		QPointF p = event->pos();
		//qDebug(QString::number(p.x()).toLatin1());
		//qDebug(QString::number(p.y()).toLatin1());
		if ((p.x() >= float(mHalfWidth - 10)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(mHalfHeight - 10)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFileMiniMap->cursor().shape() != Qt::PointingHandCursor)
				{
					graph->mFileMiniMap->setCursor(Qt::PointingHandCursor);
				}
			}
			else
			{
				if (graph->mFileMiniMap->cursor().shape() != Qt::ArrowCursor)
				{
					graph->mFileMiniMap->setCursor(Qt::ArrowCursor);
				}

			}
		}
		else
		{
				if (graph->mFileMiniMap->cursor().shape() != Qt::ArrowCursor)
				{
					graph->mFileMiniMap->setCursor(Qt::ArrowCursor);
				}
		}
	}
	QGraphicsItem::hoverMoveEvent(event);
}

void FileMiniMapNodeGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if ((type == 0) || (type == 2))
	{
		QPointF p = event->pos();
		//qDebug(QString::number(p.x()).toLatin1());
		//qDebug(QString::number(p.y()).toLatin1());
		if ((p.x() >= float(mHalfWidth - 10)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(mHalfHeight - 10)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFileMiniMap->cursor().shape() != Qt::ArrowCursor)
				{
					graph->mFileMiniMap->setCursor(Qt::ArrowCursor);
				}

				if (graph->mNodeCollapsed[mNode] == true)
				{
					graph->mNodeCollapsed[mNode] = false;
				}
				else
				{
					graph->mNodeCollapsed[mNode] = true;
				}

				//graph->mFileMiniMap->centerOn(this->scenePos());

				// calculate index in graph to avoid ambiguity
				graph->mFileViewEventNodeAmbigPosition = graph->mNodeAmbiguityPositions[mNode];

				// save position of node in view
				graph->mFileViewEventNodePosition = scenePos();
				graph->mFileViewEventNodeLabel = ogdf::String(graph->mFileGA.labelNode(mNode));

				// calculate collapsed/expanded view
				(graph->mNodeCollapsed[mNode] == false) ? graph->expandFileNode(label) : graph->collapseFileNode(label);
			}
			else
			{
				QGraphicsItem::mousePressEvent(event);
			}
		}
		else
		{
			QGraphicsItem::mousePressEvent(event);
		}
	}
	else
	{
		QGraphicsItem::mousePressEvent(event);
	}
	
	
}

void FileMiniMapNodeGraphic::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
	if (graph->mViewMode == 1)
	{
		graph->mFileView->zoomToCodeViewFile(graph->mFileViewGraphicData[this->mNode].graphics);
	}
}
QRectF FileMiniMapNodeGraphic::boundingRect() const
 {
	 // get Bounding Rectangle
     return QRectF(-mHalfWidth, -mHalfHeight, mWidth, mHeight);
 }
void FileMiniMapNodeGraphic::calculateDimensions()
{
	// scale pixmap
	//icon->setPixmap(pixmap.scaledToWidth(64,Qt::SmoothTransformation));

	// calculate width
	if ((type == 0) || (type == 2))
	{
		//if (icon->boundingRect().width() > (textitem->boundingRect().width() + 10))
		if (64 > (textitem->boundingRect().width() + 10))
			//this->setObjectWidth(int(icon->boundingRect().width()) + 2 * MARGIN);
			this->setObjectWidth(64 + 2 * MARGIN);
		else this->setObjectWidth(int(textitem->boundingRect().width()) + 10 + 2 * MARGIN);
	}
	else
	{
		//if (icon->boundingRect().width() > (textitem->boundingRect().width() ))
		if (64 > (textitem->boundingRect().width() ))
			//this->setObjectWidth(int(icon->boundingRect().width()) + 2 * MARGIN);
			this->setObjectWidth(64 + 2 * MARGIN);
		else this->setObjectWidth(int(textitem->boundingRect().width()) + 2 * MARGIN);
	}
	mHalfWidth = mWidth / 2;

	// calculate height
	//this->setObjectHeight(int(icon->boundingRect().height())+int(textitem->boundingRect().height())+3*MARGIN);
	this->setObjectHeight(64 + int(textitem->boundingRect().height())+3*MARGIN);
	mHalfHeight = mHeight / 2;

	// position icon
	if ((type == 0) || (type == 2))
		//icon->setPos(QPointF(-icon->boundingRect().width()/2, -qreal(mHalfHeight) + qreal(MARGIN)));
		icon->setPos(QPointF(-64/2, -qreal(mHalfHeight) + qreal(MARGIN)));
	
	// position text
	if ((type == 0) || (type == 2))
		textitem->setPos(QPointF(-textitem->boundingRect().width()/2 - 5, qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));
	else textitem->setPos(QPointF(-textitem->boundingRect().width()/2 , qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));

	update(this->boundingRect());

	/*
	// debug-statements
	qDebug("node");
	qDebug("width: "+QString::number(this->boundingRect().width()).toLatin1());
	qDebug("height: "+QString::number(this->boundingRect().height()).toLatin1());
	qDebug("position: "+QString::number(this->x()).toLatin1()+ " / " + QString::number(this->y()).toLatin1());

	qDebug("text-item");
	qDebug("width: "+QString::number(textitem->boundingRect().width()).toLatin1());
	qDebug("height: "+QString::number(textitem->boundingRect().height()).toLatin1());
	qDebug("position: "+QString::number(textitem->pos().x()).toLatin1()+ " / " + QString::number(textitem->pos().y()).toLatin1());

	qDebug("icon");
	qDebug("width: "+QString::number(icon->boundingRect().width()).toLatin1());
	qDebug("height: "+QString::number(icon->boundingRect().height()).toLatin1());
	qDebug("position: "+QString::number(icon->pos().x()).toLatin1()+ " / " + QString::number(icon->pos().y()).toLatin1());
	*/


}
void FileMiniMapNodeGraphic::hideText()
{
	textitem->hide();
}
void FileMiniMapNodeGraphic::showText()
{
	textitem->show();
}
void FileMiniMapNodeGraphic::hideIcon()
{
	icon->hide();
}
void FileMiniMapNodeGraphic::showIcon()
{
	icon->show();
}
void FileMiniMapNodeGraphic::updateLOD()
{
	if (graph->mFileMiniMap->getLOD() == 0)	// fill node
	{
		hideIcon();
		hideText();
		return;
	}
	else if (graph->mFileMiniMap->getLOD() == 1)	// show icon, but no text
	{
		if ((type != 1) || (fragYStart.size() == 0))
			showIcon();
		hideText();
		return;
	}
	else if ((graph->mFileMiniMap->getLOD() == 2) || (this->greyed))	// show icon and text
	{
		if ((type != 1) || (fragYStart.size() == 0))
			showIcon();
		showText();
		return;
	}
	else if (graph->mFileMiniMap->getLOD() == 3)	// show icon and text and fragments, if file node
	{
		if ((type == 1) && (fragYStart.size() > 0))
			hideIcon();
		else showIcon();
		showText();
		return;
	}
}

void FileMiniMapNodeGraphic::createFragmentRects(
	unsigned int off, unsigned int len,
	qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
	std::vector<bool> activeStatus)
{
	/*
	qDebug("adding fragment rect for node");
	qDebug(label.toLatin1());
	qDebug("number of features:");
	qDebug(QString::number(colors.size()).toLatin1());
	qDebug(QString::number(y1).toLatin1());
	qDebug(QString::number(y2).toLatin1());
	*/
	offset.push_back(off);
	length.push_back(len);
	fragYStart.push_back((y1 * qreal(mHeight)) - qreal(mHalfHeight));
	fragYEnd.push_back((y2-y1)*qreal(mHeight));
	fragmentColors.push_back(colors);
	fragmentNames.push_back(names);
	isActive.push_back(activeStatus);

}

void FileMiniMapNodeGraphic::createHistogramRects(
	unsigned int off, unsigned int len,
	qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
	std::vector<bool> activeStatus)
{
	/*
	qDebug("adding fragment rect for node");
	qDebug(label.toLatin1());
	qDebug("number of features:");
	qDebug(QString::number(colors.size()).toLatin1());
	qDebug(QString::number(y1).toLatin1());
	qDebug(QString::number(y2).toLatin1());
	*/
	/*
	histogram_offset.push_back(off);
	histogram_length.push_back(len);
	histogram_yStart.push_back((y1 * qreal(mHeight)) - qreal(mHalfHeight));
	histogram_yEnd.push_back((y2-y1)*qreal(mHeight));
	histogram_Colors.push_back(colors);
	histogram_Names.push_back(names);
	histogram_isActive.push_back(activeStatus);
	*/
	histogram_offset.push_back(off);
	histogram_length.push_back(len);
	//histogram_yStart.push_back(-1.0*((y1 * qreal(mHeight)) - qreal(mHalfHeight))-(y2-y1)*qreal(mHeight));
	histogram_yStart.push_back(qreal(mHalfHeight)-(y1*qreal(mHeight))-(y2-y1)*qreal(mHeight));
	//histogram_yStart.push_back(qreal(mHalfHeight)-((1.0-y2)*qreal(mHeight)));
	histogram_yEnd.push_back((y2-y1)*qreal(mHeight));
	//histogram_yEnd.push_back((1.0-(y2-y1))*qreal(mHeight));
	histogram_Colors.push_back(colors);
	histogram_Names.push_back(names);
	histogram_isActive.push_back(activeStatus);

}

void FileMiniMapNodeGraphic::setToolTipContent(QString str)
{
	this->setToolTip(str);

	// update tool tip label
	QString strTool;
	strTool.append(graph->mFileGA.labelNode(mNode).cstr());
	if (this->type == 0)
		strTool.append(" (project root)");
	if (this->type == 2)
		strTool.append(" (folder)");
	if (this->type == 1)
	{	
		strTool.append(" (file)");
		strTool.append("\n\n");
		strTool.append("Features:\n");
		for (unsigned int i = 0; i < histogram_Names.size(); i++)
		{
			strTool.append("\n");
			strTool.append(histogram_Names[i][0]);
			if (histogram_isActive[i][0] == false)
			{
				strTool.append(" (not active)");
			}
			strTool.append(" : ");
			strTool += QString("%1 %").arg((int)(histogram_yEnd[i]/(qreal)mHeight*100.0));

		}
	}
	this->setToolTip(strTool);
}