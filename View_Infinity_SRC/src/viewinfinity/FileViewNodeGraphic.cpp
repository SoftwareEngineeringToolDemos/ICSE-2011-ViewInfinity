

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsItem>

#include "SMVMain.h"
#include "FileView.h"
#include "FileViewNodeGraphic.h"
#include "FileTreeModel.h"


const int MARGIN				= 5;
const int PREVIEW_MARGIN		= 2;

FileViewNodeGraphic::FileViewNodeGraphic(SMVMain *graphWidget)
    : graph(graphWidget)
{
	setFlags(ItemIsMovable);
	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

	icon = new QGraphicsPixmapItem(this);			// initialize node icon
	showPreviewPixmap = false;

	textitem = new QGraphicsSimpleTextItem(this);	// initialize node label

	greyed = false;
}	

QVariant FileViewNodeGraphic::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged)
	{
		graph->updateFileViewNodePosition(mNode);
	}
	return QGraphicsItem::itemChange(change, value);
}

void FileViewNodeGraphic::setFileGraphNode(ogdf::node n)
{
	mNode = n;
}

ogdf::node FileViewNodeGraphic::getFileGraphNode()
{
	return mNode;
}

int FileViewNodeGraphic::getObjectWidth()
{
	return mWidth;
}

bool FileViewNodeGraphic::setObjectWidth(int w)
{
	if (w > 0)
	{
		mWidth = w;
		mHalfWidth = mWidth / 2;
		return true;
	}
	return false;
}

int FileViewNodeGraphic::getObjectHeight()
{
	return mHeight;
}

bool FileViewNodeGraphic::setObjectHeight(int h)
{
	if (h > 0)
	{
		mHeight = h;
		mHalfHeight = mHeight / 2;
		return true;
	}
	return false;
}

bool FileViewNodeGraphic::setLabel(QString text)
{
	label = text;
	textitem->setText(label);
	return true;
}

QString	FileViewNodeGraphic::Label()
{
	return label;
}

void  FileViewNodeGraphic::setNodePixmap(QPixmap pm)
{
	codePreviewPixmap = pm;
	if (pm.size().width() > 0)
	{
		showPreviewPixmap = true;
		qreal pixmap_ratio = (double)codePreviewPixmap.width() / (double)codePreviewPixmap.height();
		//qDebug() << "fileviewnodegraphic, pixmap ratio = " << pixmap_ratio;

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

bool FileViewNodeGraphic::setType(int nodetype)
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

int FileViewNodeGraphic::Type()
{
	return type;
}

void FileViewNodeGraphic::setGreyed(bool is_gray)
{
	greyed = is_gray;
	setType(this->type);
}

bool FileViewNodeGraphic::isGreyed()
{
	return greyed;
}

void FileViewNodeGraphic::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{
	painter->setClipRect( option->exposedRect );

	if (graph->mFileView->getLOD() == 0)	// fill node
	{
		painter->setBrush(QBrush(mLineColor, Qt::SolidPattern));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		if ((type == 0) || (type == 2))		// draw folding symbol border
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
	else if (graph->mFileView->getLOD() == 1) // draw node border with icon only
	{
		// draw border
		painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		if ((type == 0) || (type == 2))		// draw folding symbol border
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
			if ((histogram_yStart.size() > 0) && (!greyed) )
			{
				// draw each fragment
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
			else		// draw representation for "file not found"
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
	else if ((graph->mFileView->getLOD() == 2) || (this->greyed == true)) // draw node border with icon and text
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
				// draw each fragment
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
			else		// draw representation for "file not found"
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
	else if (graph->mFileView->getLOD() == 3) // draw node border with code previews
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
				painter->fillRect(this->boundingRect(),QBrush(Qt::white));
				painter->drawPixmap(pixmap_x_start, pixmap_y_start, pixmap_width, pixmap_height, codePreviewPixmap);
	
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
		}

		// draw border
		painter->setBrush(Qt::NoBrush);
		if (greyed)
			painter->setPen(QPen(Qt::darkGray, 3, Qt::SolidLine));
		else painter->setPen(QPen(mLineColor, 3, Qt::SolidLine));
		painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);

		return;
	}
}

void FileViewNodeGraphic::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsItem::hoverEnterEvent(event);
}

void FileViewNodeGraphic::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	graph->mFileView->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}

void FileViewNodeGraphic::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if ((type == 0) || (type == 2))
	{
		QPointF p = event->pos();
		if ((p.x() >= float(mHalfWidth - 10)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(mHalfHeight - 10)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFileView->cursor().shape() != Qt::PointingHandCursor)
				{
					graph->mFileView->setCursor(Qt::PointingHandCursor);
				}
			}
			else
			{
				if (graph->mFileView->cursor().shape() != Qt::ArrowCursor)
				{
					graph->mFileView->setCursor(Qt::ArrowCursor);
				}

			}
		}
		else
		{
			if (graph->mFileView->cursor().shape() != Qt::ArrowCursor)
			{
				graph->mFileView->setCursor(Qt::ArrowCursor);
			}
		}
	}
	QGraphicsItem::hoverMoveEvent(event);
}

void FileViewNodeGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if ((type == 0) || (type == 2))
	{
		QPointF p = event->pos();
		if ((p.x() >= float(mHalfWidth - 10)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(mHalfHeight - 10)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFileView->cursor().shape() != Qt::ArrowCursor)
				{
					graph->mFileView->setCursor(Qt::ArrowCursor);
				}

				if (graph->mNodeCollapsed[mNode] == true)
				{
					graph->mNodeCollapsed[mNode] = false;
				}
				else
				{
					graph->mNodeCollapsed[mNode] = true;
				}

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

void FileViewNodeGraphic::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
	graph->mFileView->wished_zoomed_in = graph->mFileView->upper_scaling_zoomed_in;
	graph->mFileView->wished_scale = 1.5*graph->mFileView->upper_scaling_limit;
	graph->mFileView->wishedZoomInFile = this;
	if (graph->mFileView->transformationAnchor() != QGraphicsView::AnchorUnderMouse)
		graph->mFileView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	this->ungrabMouse();

	// start timer
	if (!graph->mFileView->timerId)
	{
		graph->mFileView->timerId = graph->mFileView->startTimer(1000/25); // timer started
	}
}

QRectF FileViewNodeGraphic::boundingRect() const
 {
	 // get bounding rectangle
     return QRectF(-mHalfWidth, -mHalfHeight, mWidth, mHeight);
 }
void FileViewNodeGraphic::calculateDimensions()
{
	// calculate width
	if ((type == 0) || (type == 2))
	{
		if (64 > (textitem->boundingRect().width() + 10))
			this->setObjectWidth(64 + 2 * MARGIN);
		else this->setObjectWidth(int(textitem->boundingRect().width()) + 10 + 2 * MARGIN);
	}
	else
	{
		if (64 > (textitem->boundingRect().width() ))
			this->setObjectWidth(64 + 2 * MARGIN);
		else this->setObjectWidth(int(textitem->boundingRect().width()) + 2 * MARGIN);
	}
	mHalfWidth = mWidth / 2;

	// calculate height
	this->setObjectHeight(64 + int(textitem->boundingRect().height())+3*MARGIN);
	mHalfHeight = mHeight / 2;

	// position icon
	if ((type == 0) || (type == 2))
		icon->setPos(QPointF(-64/2, -qreal(mHalfHeight) + qreal(MARGIN)));
	
	// position text
	if ((type == 0) || (type == 2))
		textitem->setPos(QPointF(-textitem->boundingRect().width()/2 - 5, qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));
	else textitem->setPos(QPointF(-textitem->boundingRect().width()/2 , qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));

	update(this->boundingRect());
}
void FileViewNodeGraphic::hideText()
{
	textitem->hide();
}
void FileViewNodeGraphic::showText()
{
	textitem->show();
}
void FileViewNodeGraphic::hideIcon()
{
	icon->hide();
}
void FileViewNodeGraphic::showIcon()
{
	icon->show();
}
void FileViewNodeGraphic::updateLOD()
{
	if (graph->mFileView->getLOD() == 0)	// fill node
	{
		hideIcon();
		hideText();
		return;
	}
	else if (graph->mFileView->getLOD() == 1)	// show icon, but no text
	{
		if ((type != 1) || (fragYStart.size() == 0))
			showIcon();
		hideText();
		return;
	}
	else if ((graph->mFileView->getLOD() == 2) || (this->greyed))	// show icon and text
	{
		if ((type != 1) || (fragYStart.size() == 0))
			showIcon();
		showText();
		return;
	}
	else if (graph->mFileView->getLOD() == 3)	// show icon and text and fragments, if file node
	{
		if ((type == 1) && (fragYStart.size() > 0))
			hideIcon();
		else showIcon();
		showText();
		return;
	}
}

void FileViewNodeGraphic::createFragmentRects(
	unsigned int off, unsigned int len,
	qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
	std::vector<bool> activeStatus)
{
	offset.push_back(off);
	length.push_back(len);
	fragYStart.push_back((y1 * qreal(mHeight)) - qreal(mHalfHeight));
	fragYEnd.push_back((y2-y1)*qreal(mHeight));
	fragmentColors.push_back(colors);
	fragmentNames.push_back(names);
	isActive.push_back(activeStatus);

}

void FileViewNodeGraphic::createHistogramRects(
	unsigned int off, unsigned int len,
	qreal y1, qreal y2, std::vector<QColor> colors, std::vector<QString> names,
	std::vector<bool> activeStatus)
{
	histogram_offset.push_back(off);
	histogram_length.push_back(len);
	histogram_yStart.push_back(qreal(mHalfHeight)-(y1*qreal(mHeight))-(y2-y1)*qreal(mHeight));
	histogram_yEnd.push_back((y2-y1)*qreal(mHeight));
	histogram_Colors.push_back(colors);
	histogram_Names.push_back(names);
	histogram_isActive.push_back(activeStatus);
}

void FileViewNodeGraphic::setToolTipContent(QString str)
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