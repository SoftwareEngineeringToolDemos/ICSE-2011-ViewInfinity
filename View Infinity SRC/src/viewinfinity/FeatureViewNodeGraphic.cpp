

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsItem>

#include "SMVMain.h"
#include "FeatureView.h"
#include "FeatureViewNodeGraphic.h"
#include "FeatureTreeModel.h"

const int MARGIN				= 5;

FeatureViewNodeGraphic::FeatureViewNodeGraphic(SMVMain *graphWidget)
    : graph(graphWidget)
{
	//setFlags(ItemIsMovable);
	this->setAcceptHoverEvents(true);
	this->setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
	this->setAcceptDrops(true);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
	featureColor = QColor(111, 245, 111, 180);
	mActive = true;
	drawfilescenepreview = false;

	// initialize node label
	//textitem = new QGraphicsSimpleTextItem(this);
	textitem = new QGraphicsSimpleTextItem(0);
}	

QVariant FeatureViewNodeGraphic::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionChange)
	{
		graph->updateFeatureViewNodePosition(mNode);
	}
	return QGraphicsItem::itemChange(change, value);
}

void FeatureViewNodeGraphic::setFeatureGraphNode(ogdf::node n)
{
	mNode = n;
}

ogdf::node FeatureViewNodeGraphic::getFeatureGraphNode()
{
	return mNode;
}

int FeatureViewNodeGraphic::getObjectWidth()
{
	return mWidth;
}

bool FeatureViewNodeGraphic::setObjectWidth(int w)
{
	if (w>0)
	{
		mWidth = w;
		mHalfWidth = mWidth / 2;
		return true;
	}
	return false;
}

int FeatureViewNodeGraphic::getObjectHeight()
{
	return mHeight;
}

bool FeatureViewNodeGraphic::setObjectHeight(int h)
{
	if (h>0)
	{
		mHeight = h;
		mHalfHeight = mHeight / 2;
		return true;
	}
	return false;
}

bool FeatureViewNodeGraphic::setLabel(QString text)
{
	label = text;
	textitem->setText(label);
	return true;
}

QString	FeatureViewNodeGraphic::Label()
{
	return label;
}

bool FeatureViewNodeGraphic::setType(int nodetype)
{
	if (nodetype >= 0)
	{
		type = nodetype;
		mLineColor = Qt::darkRed;
		
		return true;
	}
	return false;
}

void FeatureViewNodeGraphic::setFeatureColor(QColor col)
{
	featureColor = col;
	featureColor.setAlpha(180);
}

QColor FeatureViewNodeGraphic::getFeatureColor()
{
	return featureColor;
}

int FeatureViewNodeGraphic::Type()
{
	return type;
}
void FeatureViewNodeGraphic::setActivation(bool active)
{
	mActive = active;
	if (mActive)
	{
		QBrush brush = textitem->brush();
		brush.setColor(Qt::black);
	}
	else
	{
		QBrush brush = textitem->brush();
		brush.setColor(Qt::darkGray);
	}
}
bool FeatureViewNodeGraphic::getActivation()
{
	return mActive;
}
void FeatureViewNodeGraphic::setIsFeature(bool nodetype)
{
	mIsFeature = nodetype;
}
bool FeatureViewNodeGraphic::isFeature()
{
	return mIsFeature;
}

void FeatureViewNodeGraphic::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget)
{

	//painter->setClipRect( option->exposedRect );
	if (mActive == false)
	{
		if(type == 3)
		{
			setActivation(false);
		}
	}

	// draw border
	if(mIsFeature)
	{
		if(mActive)
		{
			
			painter->setPen(QPen(QBrush(mLineColor,Qt::BrushStyle(Qt::SolidLine)),2,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
			//painter->setPen(QPen(QColor(Qt::black, 3, Qt::DotLine)));
			//painter->setCompositionMode(QPainter::CompositionMode_Source);
			painter->fillRect(-mHalfWidth+1, -mHalfHeight+1, mWidth-2, mHeight-2,QBrush(featureColor,Qt::SolidPattern));
			painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
			
		}
		else
		{
			painter->setPen(QPen(QBrush(Qt::darkGray,Qt::BrushStyle(Qt::SolidLine)),2,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
			//painter->setCompositionMode(QPainter::CompositionMode_Source);
			painter->fillRect(-mHalfWidth+1, -mHalfHeight+1, mWidth-2, mHeight-2,QBrush(Qt::lightGray,Qt::SolidPattern));
			painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
		}
	}
	else
	{
		//if(mActive)
		//{
			painter->setPen(QPen(QBrush(Qt::black,Qt::BrushStyle(Qt::SolidLine)),2,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
			painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
		//}
		/*
		else
		{
			painter->setPen(QPen(QBrush(Qt::darkGray,Qt::BrushStyle(Qt::SolidLine)),2,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
			//painter->setCompositionMode(QPainter::CompositionMode_Source);
			painter->drawRoundedRect(-mHalfWidth, -mHalfHeight, mWidth, mHeight, 5, 5);
		}
		*/
	}

	// draw dependencies
	switch (type)
	{
	case 0:
		break; 
	case 1:
		// für optional
		painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
		//painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);
		//painter->setBackgroundMode(Qt::OpaqueMode);
		painter->drawEllipse(-5, -mHalfHeight,8.0,8.0);
		break;
	case 2:
		// für Alternative
		painter->setPen(QPen(mLineColor, 2, Qt::SolidLine));
		//painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);
		//painter->setBackgroundMode(Qt::OpaqueMode);
		painter->drawEllipse(QPointF(0, mHalfHeight),8.0,8.0);
		break;
	case 3:
		// für obligatorisch
		painter->setPen(QPen(mLineColor, 5, Qt::SolidLine));
		//painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);
		//painter->setBackgroundMode(Qt::OpaqueMode);
		//painter->fillRect(-mHalfWidth+1, -mHalfHeight+1, mWidth-2, mHeight-2,QBrush(featureColor,Qt::SolidPattern));
		painter->drawEllipse(-3, -mHalfHeight+2,5.0,5.0);
		break;
	case 4:
		// für Or
		painter->setPen(QPen(mLineColor, 5, Qt::SolidLine));
		painter->drawEllipse(QPointF(0, mHalfHeight),4.0,4.0);
		//painter->setCompositionMode(QPainter::CompositionMode_Destination);
		//painter->drawArc(-5, mHalfHeight,5,2,-30*16,-120*16);
		break;	
	}

	//QTextItem item;
	//item.text() = this->label;
	//painter->drawTextItem(textitem->pos(), item);
	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
	painter->drawText(textpos, label);

	if (drawfilescenepreview)
	{
		//painter->drawPixmap( this->boundingRect().toRect(), graph->filescenepreview);

		//QPixmap pm = graph->filescenepreview.scaled(preview_pixmap_width,preview_pixmap_height,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
		//painter->drawPixmap(preview_pixmap_x_start, preview_pixmap_y_start, preview_pixmap_width, preview_pixmap_height, pm);

		//qDebug() << "bounding rect topleft" << this->boundingRect().topLeft();
		//qDebug() << "bounding rect bottom right" << this->boundingRect().bottomRight();

		//QRect target(preview_pixmap_x_start,preview_pixmap_y_start,preview_pixmap_width,preview_pixmap_height);
		//painter->drawPixmap(target, graph->filescenepreview, graph->filescenepreview.rect());

		QPixmap px(*graph->filescenepreview);
		QPixmap alphapix(px.size());

		QPainter p(&alphapix);
		p.fillRect(alphapix.rect(), QColor(preview_pixmap_alpha, preview_pixmap_alpha, preview_pixmap_alpha));
		p.end();

		//alphapix.fill(QColor(preview_pixmap_alpha,preview_pixmap_alpha,preview_pixmap_alpha));
		px.setAlphaChannel(alphapix);
		painter->drawPixmap(preview_pixmap_x_start,preview_pixmap_y_start,preview_pixmap_width,preview_pixmap_height,px);
		

		//painter->drawPixmap(preview_pixmap_x_start, preview_pixmap_y_start, graph->filescenepreview,100,100,0,0);
	}

	// Debug
	//painter->drawRect(int(icon->pos().x()),int(icon->pos().y()),icon->boundingRect().width(),icon->boundingRect().height());
	//painter->drawRect(int(textitem->pos().x()), int(textitem->pos().y()), textitem->boundingRect().width(), textitem->boundingRect().height());
}

/*
void FeatureViewNodeGraphic::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsItem::hoverEnterEvent(event);
}

void FeatureViewNodeGraphic::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	graph->mFeatureView->setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverLeaveEvent(event);
}
void FeatureViewNodeGraphic::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
	if ((mIsFeature) && (type != 3))
	{
		QPointF p = event->pos();
		//qDebug(QString::number(p.x()).toLatin1());
		//qDebug(QString::number(p.y()).toLatin1());
		//-mHalfWidth, -mHalfHeight, mWidth, mHeight
		if ((p.x() >= float(-mHalfWidth)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(-mHalfHeight)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFeatureView->cursor().shape() != Qt::PointingHandCursor)
				{
					graph->mFeatureView->setCursor(Qt::PointingHandCursor);
				}
			}
		}
		else
		{
			if (graph->mFeatureView->cursor().shape() != Qt::ArrowCursor)
			{
				graph->mFeatureView->setCursor(Qt::ArrowCursor);
			}

		}
	}
	QGraphicsItem::hoverMoveEvent(event);
}
*/
void FeatureViewNodeGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	//qDebug() << event->button();
	if (event->button() == Qt::LeftButton)
	{
		if (mActive == false)
		{
			//mActive = true;
			graph->updateFeatureStatus(label, mNode, true);
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		if ( (mActive == true) && (QString::compare(graph->mFeatureView->wishedZoomInFeature, label) != 0) )
		{
			//mActive = false;
			graph->updateFeatureStatus(label, mNode, false);
		}
	}
	
	//update();
	/*
	if ((mIsFeature) && (type != 3)) // 3= obligatory can not be changed!
	{
		QPointF p = event->pos();
		//qDebug(QString::number(p.x()).toLatin1());
		//qDebug(QString::number(p.y()).toLatin1());
		if ((p.x() >= float(-mHalfWidth)) && (p.x() <= float(mHalfWidth)))
		{
			if  ((p.y() >= float(-mHalfHeight)) && (p.y() <= float(mHalfHeight)))
			{
				if (graph->mFeatureView->cursor().shape() != Qt::PointingHandCursor)
				{
					graph->mFeatureView->setCursor(Qt::PointingHandCursor);
				}

				if (graph->mNodeActive[mNode] == true)
				{
					graph->mNodeActive[mNode] = false;
					setActivation(false);
					// dependencies
					if(type == 2) // alternative, also genau 1 festure auswählbar
					{
						graph->updateAlternativeFeatures(mNode, true);
						// elternknoten holen
						// davon alle kinderknoten, bei restlichen kinder einen default aktivieren
					}
				}
				else
				{			
					graph->mNodeActive[mNode] = true;
					setActivation(true);
					// dependencies
					if(type == 2) // alternative, also genau 1 festure auswählbar
					{
						graph->updateAlternativeFeatures(mNode, false);
						// elternknoten holen
						// davon alle kinderknoten, restlichen kinder deaktivieren
					}
				}
				update(this->boundingRect());
				// update feature states
				graph->updateFeatureStates();

				// save position of node in view
				// benötigt?
				graph->mFeatureViewEventNodePosition = scenePos();
				graph->mFeatureViewEventNodeLabel = ogdf::String(graph->mFeatureGA->labelNode(mNode));

				// update file view
				// todo: collape function for features
				//graph->collapseFileViewNode();

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
	*/
	QGraphicsItem::mousePressEvent(event);
}


void FeatureViewNodeGraphic::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
	if (graph->mViewMode != 0)
		return;

	if ( (mActive) || (!mIsFeature) )
	{
		if (m_parent != (QGraphicsView*)graph->mFeatureMiniMap)
		{
			graph->mFeatureView->wished_zoomed_in = graph->mFeatureView->upper_scaling_zoomed_in;
			graph->mFeatureView->wished_scale = 1.5*graph->mFeatureView->upper_scaling_limit;
			graph->mFeatureView->wishedZoomInFeature = label;
			graph->mFeatureView->wishedZoomInFeatureGraphic = this;
		if (graph->mFeatureView->transformationAnchor() != QGraphicsView::AnchorUnderMouse)
			graph->mFeatureView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
			// start timer
			if (!graph->mFeatureView->timerId)
			{
				graph->mFeatureView->timerId = graph->mFeatureView->startTimer(1000/25); // timer started
			}
		}
		else	// double click in minimap
		{
			// TODO: direct change of fileview for new feature/feature tree

			//graph->zoomIntoFeature(label);		// no working properly
		}
	}
	else
	{
		QString statusmsg;
		statusmsg.append("Feature ''");
		statusmsg.append(label);
		statusmsg.append("'' not active!");

		QMessageBox msgBox;
		msgBox.setText(statusmsg);
		msgBox.exec();
	}
}

QRectF FeatureViewNodeGraphic::boundingRect() const
 {
	 // get Bounding Rectangle
     return QRectF(-mHalfWidth, -mHalfHeight, mWidth, mHeight);
 }
void FeatureViewNodeGraphic::calculateDimensions()
{
	this->setObjectWidth(int(textitem->boundingRect().width()) + 10 + 2 * MARGIN);
	mHalfWidth = mWidth / 2;

	// calculate height
	this->setObjectHeight(int(textitem->boundingRect().height())+3*MARGIN);
	mHalfHeight = mHeight / 2;

	// position icon
	//icon->setPos(QPointF(-icon->boundingRect().width()/2, -qreal(mHalfHeight) + qreal(MARGIN)));
	
	// position text
	//if ((type == 0) || (type == 2))

	//	textitem->setPos(QPointF(-textitem->boundingRect().width()/2 + 5, qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));

	//else textitem->setPos(QPointF(-textitem->boundingRect().width()/2 , qreal(mHalfHeight) - qreal(MARGIN) - textitem->boundingRect().height()));

	textpos = QPointF(-textitem->boundingRect().width() / 2 - qreal(MARGIN) / 2 , mHalfHeight - qreal(MARGIN) - 2.5 );
	

	update(this->boundingRect());

}

void FeatureViewNodeGraphic::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
	//event->accept();
	//qDebug() << "dragEnterEvent";
	
	if( (mIsFeature) &&	(qvariant_cast<QColor>(event->mimeData()->colorData()).isValid()) )
        event->accept();
	
    else
        event->ignore();
	
}
void FeatureViewNodeGraphic::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
	//event->accept();
	//qDebug() << "dragMoveEvent";
	
    if( (mIsFeature) && (qvariant_cast<QColor>(event->mimeData()->colorData()).isValid()) )
	{
        event->accept();
    } else {
        event->ignore();
    }
	
}
void FeatureViewNodeGraphic::dropEvent(QGraphicsSceneDragDropEvent *event)
{
	//qDebug() << "drop event";
	QColor col = qvariant_cast<QColor>(event->mimeData()->colorData());
	//qDebug() << "color" << col.rgb();
    if (col.isValid()) {
		// update feature color for this node
		this->setFeatureColor(col);
		// update feature color all over
		this->graph->updateFeatureColor(this->label, this->featureColor);
        //int i = rowAt(e->pos().y()) + columnAt(e->pos().x()) * numRows();
        //values[i] = col.rgb();
        //update();
		//qDebug() << "acceped";
		//qDebug() << col.red();
		//qDebug() << col.green();
		//qDebug() << col.blue();
        event->accept();
    } else {
		//qDebug() << "ignored";
        event->ignore();
    }
}
QPixmap& FeatureViewNodeGraphic::setAlpha(QPixmap &px, int val)
{
	QPixmap alpha = px;
	QPainter p(&alpha);
	p.fillRect(alpha.rect(), QColor(val, val, val));
	p.end();
	px.setAlphaChannel(alpha);
	return px;
}