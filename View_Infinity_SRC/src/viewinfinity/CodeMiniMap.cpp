

#include <QGraphicsScene>
#include "SMVMain.h"
#include "CodeView.h"
#include "CodeViewInteractiveRect.h"
#include "CodeMiniMap.h"


CodeMiniMap::CodeMiniMap(QWidget  *parent)
{
	mParent = parent;
	mCodeView = 0;
	codepixmap = 0;

	mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
	mScene->setSceneRect(-this->geometry().width()/2,-this->geometry().height()/2,this->geometry().width(),this->geometry().height());
	setScene(mScene);

	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing,true);

	m_intrect = new CodeViewInteractiveRect(this);
	mScene->addItem(m_intrect);
	m_intrect->setPos(QPointF(0.0,0.0));

	m_intrect->rect_height = scene()->sceneRect().height();
	m_intrect->rect_width = scene()->sceneRect().width();
	zoomlevel = 1.0f;

	//this->horizontalScrollBar()->hide();
	//this->verticalScrollBar()->hide();
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

CodeMiniMap::~CodeMiniMap(void)
{
}

void CodeMiniMap::setBackground(QPixmap *pixmap)
{
	mScene->setSceneRect(-this->geometry().width()/2,-this->geometry().height()/2,this->geometry().width(),this->geometry().height());
	setScene(mScene);
	codepixmap = new QPixmap(*pixmap);
	m_intrect->rect_width = scene()->sceneRect().width();
	updateContent();
}

// draw background of widget window
void CodeMiniMap::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

	if (codepixmap != 0)
	{
		QPixmap pm = codepixmap->scaled(int(sceneRect().width()),int(sceneRect().height()*zoomlevel),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
		painter->drawPixmap(sceneRect(),pm,QRectF(pm.rect()));
		
	}
	
	// draw upper rect
	QPointF intrect_top = m_intrect->mapToScene(m_intrect->boundingRect().topRight());
	intrect_top.setY( intrect_top.y() + 2 );
	QRectF rec1(this->mapToScene(this->rect().topLeft()),intrect_top);
	painter->fillRect(rec1,QBrush(QColor::fromRgbF(0, 0, 0, 0.5)));

	// draw lower rect
	QPointF intrect_bottom = m_intrect->mapToScene(m_intrect->boundingRect().bottomLeft());
	intrect_bottom.setY( intrect_bottom.y() - 2 );
	QRectF rec2(intrect_bottom,	this->mapToScene(this->rect().bottomRight()));
	painter->fillRect(rec2,QBrush(QColor::fromRgbF(0, 0, 0, 0.5)));
}

void CodeMiniMap::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	mScene->setSceneRect(-this->geometry().width()/2,-this->geometry().height()*zoomlevel/2,this->geometry().width(),this->geometry().height()*zoomlevel);
	updateContent();
}
void CodeMiniMap::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() == Qt::NoModifier)
	{
		// zoom functionality
		/*
		if (event->delta()<0)
		{
			// zoom out
			zoomlevel -= zoomlevel/10.0;
			if (zoomlevel < 1.0f)
			{
				zoomlevel = 1.0f;
			}
		}		
		else if (event->delta()>0)
		{
			// zoom in
			zoomlevel += zoomlevel/10.0;
		}
		*/
	}
	else
	{
		QGraphicsView::wheelEvent(event);
	}

	mScene->setSceneRect(-this->geometry().width()/2,-this->geometry().height()*zoomlevel/2,this->geometry().width(),this->geometry().height()*zoomlevel);
	mCodeView->scrollText(0,0);
	updateContent();
}
void CodeMiniMap::scrollContentsBy(int dx, int dy)
{
	//qDebug() << "codeminimap scroll dx = " << dx << " y = " << dy;
	QGraphicsView::scrollContentsBy(dx,dy);

/*
	// update scroll in code view
	if (mCodeView != 0)
	{
		if (this->verticalScrollBar()->maximum() > this->verticalScrollBar()->minimum())
		{

			qDebug() << "min = " << verticalScrollBar()->minimum();
			qDebug() << "max = " << verticalScrollBar()->maximum();
			qreal correctedvalue = qreal(this->verticalScrollBar()->value())-qreal(this->verticalScrollBar()->minimum());
			qDebug() << "corrected value = " << correctedvalue;
			qreal interval = qreal(this->verticalScrollBar()->maximum()) - qreal(this->verticalScrollBar()->minimum());
			qDebug() << "interval = " << interval;
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << "try to set scroll value of code view to " << val;
			mCodeView->setScrollBarValue(val);
		}
		else if (this->verticalScrollBar()->minimum() > this->verticalScrollBar()->maximum())
		{
			qDebug() << "min = " << verticalScrollBar()->minimum();
			qDebug() << "max = " << verticalScrollBar()->maximum();
			qreal correctedvalue = qreal(this->verticalScrollBar()->value())-qreal(this->verticalScrollBar()->maximum());
			qDebug() << "corrected value = " << correctedvalue;
			qreal interval = qreal(this->verticalScrollBar()->minimum()) - qreal(this->verticalScrollBar()->maximum());
			qDebug() << "interval = " << interval;
			qreal val =  correctedvalue / interval;
			if (val < 0.0)
				val = -val;
			qDebug() << "try to set scroll value of code view to " << val;
			mCodeView->setScrollBarValue(val);
		}
		// else max-min = 0 !!!!
	}
*/
	/*
	qDebug() << "minimap scroll bar vertical value = " << this->verticalScrollBar()->value();
	qDebug() << "scrollbar min = " << this->verticalScrollBar()->minimum();
	qDebug() << "scrollbar max = " << this->verticalScrollBar()->maximum();
	qDebug() << "scrollbar page step = " << this->verticalScrollBar()->pageStep();
	qDebug() << "scene : " << this->scene()->height();
	qDebug() << "scene size = " << this->sceneRect().height();
	qDebug() << "widget rect size = " << this->rect().height();
	qDebug() << "widget geometry size = " << this->geometry().size().height();
	*/
}

void CodeMiniMap::setCodeView(CodeView *view)
{
	mCodeView = view;
}

void CodeMiniMap::setScrollBarValue(qreal val)
{
	if (this->verticalScrollBar()->maximum() > this->verticalScrollBar()->minimum())
	{
	this->verticalScrollBar()->setValue(verticalScrollBar()->minimum()+
		val*(verticalScrollBar()->maximum()-verticalScrollBar()->minimum()));
	}
	else if (this->verticalScrollBar()->minimum() > this->verticalScrollBar()->maximum())
	{
		this->verticalScrollBar()->setValue(verticalScrollBar()->maximum()+
			val*(verticalScrollBar()->minimum()-verticalScrollBar()->maximum()));
	}
}
void CodeMiniMap::updateContent()
{
	m_intrect->updateIntRectHeight();
	m_intrect->updateIntRectPosFromEditor();
	mScene->update(mScene->sceneRect());
}