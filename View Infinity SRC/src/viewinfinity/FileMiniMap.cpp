

#include <QGraphicsScene>
#include <QString>
#include "SMVMain.h"
#include "FileMiniMapNodeGraphic.h"
#include "FileView.h"
#include "FileViewInteractiveRect.h"
#include "FileMiniMap.h"

// animation resolution
const qreal RESOLUTION = 0.1;

FileMiniMap::FileMiniMap(QWidget  *parent)
	: timerId(0)
{
	//mainWidget = graphWidget;
	mParent = parent;
	mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

	mScene->setSceneRect(-this->rect().width()/2,-this->rect().height()/2,this->rect().width(),this->rect().height());
	setScene(mScene);

	LOD = 2;

	setRenderHint(QPainter::Antialiasing,true);
	setRenderHint(QPainter::TextAntialiasing, true);
	//setRenderHint(QPainter::SmoothPixmapTransform, true);
	//setOptimizationFlag(DontClipPainter, false);
	//setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	// initialize navigator rectangle
	m_intrect = new FileViewInteractiveRect(this);
	mScene->addItem(m_intrect);
	m_intrect->setPos(QPointF(0.0,0.0));
	zoomlevel = 1.0f;

	m_intrect->rect_height = scene()->sceneRect().height();
	m_intrect->rect_width = scene()->sceneRect().width();

	// initialize scale of file view
	initial_scale = viewportTransform().m11();
	wished_scale = initial_scale;
	current_scale = initial_scale;

	lower_scaling_limit = initial_scale;
	lower_scaling_zoomed_in = true;
	upper_scaling_limit = initial_scale;
	upper_scaling_zoomed_in = true;

	// set initial value for panning
	isPanning = false;

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

FileMiniMap::~FileMiniMap(void)
{
}


// scale content of widget so that scene is fully visible
void FileMiniMap::showFullScene()
{
	// initialize scale of file view
	/*
	qreal s_factor_w = qreal(this->geometry().width())/this->sceneRect().width();
	qreal s_factor_h = qreal(this->geometry().height())/this->sceneRect().height();
	qreal s_factor;
	if (s_factor_h < s_factor_w)
		s_factor = s_factor_h;
	else s_factor = s_factor_w;

	if (s_factor < 1.0)	// zoomed in
	{
		wished_scale = 1/s_factor;
		wished_zoomed_in = false;
	}
	else				// zoomed out
	{
		wished_scale = s_factor;
		wished_zoomed_in = true;
	}
	*/

	if (mainWidget->projectLoaded == false)
		return;

	calculateScalingLimits();

	wished_scale = lower_scaling_limit;
	wished_zoomed_in = lower_scaling_zoomed_in;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}

unsigned int FileMiniMap::getLOD()
{
	return LOD;
}

void FileMiniMap::setMainWidget(SMVMain *mWidget)
{
	mainWidget = mWidget;
}

// draw background of widget window
void FileMiniMap::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
	painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
	painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersect(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);
}
// Qt resize event
void FileMiniMap::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	updateContent();
}
// Qt timer event
void FileMiniMap::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

	// stop movement
	if (current_scale < 1.0)
	{
		qreal current = 1 / current_scale;
		if ((wished_scale * 0.99) < current)
			if ((wished_scale * 1.001) > current)
			{
				killTimer(timerId);
				timerId = 0;
				return;
			}
	}
	else
	{
		if ((wished_scale * 0.99) < current_scale)
			if ((wished_scale * 1.001) > current_scale)
			{
				killTimer(timerId);
				timerId = 0;
				return;
			}
	}
	if (current_scale < 1.0)
	{
		if (wished_zoomed_in)
		{
			
			// zoom in
			zoom_speed = (wished_scale-1.0)+(1/current_scale-1.0);
			if (zoom_speed > 1.0) zoom_speed = 1.0;
			scaleView(1.0 + zoom_speed * RESOLUTION);
			current_scale = viewportTransform().m11();
		}
		else
		{
			if (wished_scale > (1/current_scale))
			{
				// zoom out
				zoom_speed = (wished_scale-1.0) - (1/current_scale-1.0);
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 - zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
			}
			else
			{
				// zoom in
				zoom_speed = (1/current_scale-1.0)-(wished_scale-1.0);
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 + zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
			}
		}
	}
	else {
		if (wished_zoomed_in)
		{
			
			if (wished_scale < current_scale)
			{
				// zoom out
				zoom_speed = current_scale - wished_scale;
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 - zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
			}
			else
			{
				// zoom in
				zoom_speed = wished_scale - current_scale;
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 + zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
			}
		}
		else
		{
			// zoom out
			zoom_speed = (wished_scale-1.0) + (current_scale-1.0);
			if (zoom_speed > 1.0) zoom_speed = 1.0;
			scaleView(1.0 - zoom_speed * RESOLUTION);
			current_scale = viewportTransform().m11();
		}
	}
	updateContent();
}

// Qt keyboard events
void FileMiniMap::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_Plus:
        scaleView(qreal(1.2));
        break;
    case Qt::Key_Minus:
        scaleView(1 / qreal(1.2));
        break;
    case Qt::Key_Space:
		break;
    case Qt::Key_Enter:
         break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

// Qt mouse press event
void FileMiniMap::mousePressEvent(QMouseEvent *event)
{
/*
	bool hovered_item_exists = false;
	if (mainWidget->projectLoaded == true)
	{
		if (!mainWidget->mFileGraph.empty())
		{
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				if (mainWidget->getFileMiniMapNodeGraphic(v)->isUnderMouse() && (mainWidget->mFileGA.labelNode(v).length()>0))
				{
					hovered_item_exists = true;
					break;
				}
			}
		}
	}
*/
	//if ((!hovered_item_exists) && (m_intrect->rect_active == false)) // no item under mouse exists
	if (m_intrect->rect_active == false )
	{
		if (event->button() == Qt::LeftButton)
		{
			// panning started
			isPanning = true;
			dragStartPosition = QPointF(event->pos().x(), event->pos().y());
			setCursor(Qt::ClosedHandCursor);
		}
	}
	else // item under mouse exists
	{
		// forward signal
		QGraphicsView::mousePressEvent(event);
		setCursor(Qt::SizeAllCursor);
	}
}


// Qt mouse movement event
void FileMiniMap::mouseMoveEvent(QMouseEvent *event)
{
	if (isPanning) // is panning running ?
	{
		/* TODO
		** Bug : Panning erfolgt mit falscher Geschwindigkeit, wenn gleichzeitig gezoomed wird
		*/

		QPointF p = event->pos();
		qreal deltaX ( ( p.x() - dragStartPosition.x() ));
		qreal deltaY ( ( p.y() - dragStartPosition.y() ));
		
		//debug
		//qDebug("deltaX: " + QString::number(deltaX).toLatin1());
		//qDebug("deltaY: " + QString::number(deltaY).toLatin1());

		horizontalScrollBar()->setValue( horizontalScrollBar()->value() - deltaX );
		verticalScrollBar()->setValue( verticalScrollBar()->value() - deltaY ); 
		dragStartPosition = QPointF(event->pos().x(), event->pos().y());

		/*
		// get path of visible area
		QPainterPath path;
		path.addRect(QRectF(this->rect()));
		QPainterPath scenePath;
		scenePath = this->mapToScene(path);
		

		// check each node graphics item, if it is visible
		for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
		{
			if (mainWidget->mFileViewGraphicData[v].graphicsMiniMap->collidesWithPath(
				mainWidget->mFileViewGraphicData[v].graphicsMiniMap->mapFromScene(scenePath),
				Qt::IntersectsItemShape))
			{
				//qDebug(mainWidget->mFileViewGraphicData[v].graphicsMiniMap->Label().toLatin1());
			}
		}
		*/

	}
	else // panning is not running
	{
		// forward signal
		QGraphicsView::mouseMoveEvent(event);
	}
}

// Qt mouse release event
void FileMiniMap::mouseReleaseEvent(QMouseEvent* event)
{
	if (isPanning) // is panning running ?
	{
		setCursor(Qt::ArrowCursor);
		isPanning = false;
	}
	else // panning is not running
	{
		// forward signal
		QGraphicsView::mouseReleaseEvent(event);
		//setCursor(Qt::ArrowCursor);
	}
}

// Qt wheel event
void FileMiniMap::wheelEvent(QWheelEvent *event)
{

	//qDebug() << "wished_scale: " << wished_scale;
	//qDebug() << "wished_zoomed_in: " << wished_zoomed_in;
	//qDebug() << "lower_scaling_limit: " << lower_scaling_limit;
	//qDebug() << "lower_scaling_zoomed_in: " << lower_scaling_zoomed_in;

	// zooming / set wished scale
	if (event->delta()>0)
	{
		/*
		if (wished_zoomed_in)
		{
			if (wished_scale > 20.0) return;
			wished_scale += wished_scale*0.1;
		}
		else
		{
			wished_scale -= wished_scale*0.1;
			if (wished_scale < 1)
			{
				wished_scale = 1.0 + (1.0-wished_scale);
				wished_zoomed_in = true;
			}
		}
		*/
		if (wished_zoomed_in)
		{
			if (wished_scale < 2.0*upper_scaling_limit)
				wished_scale += wished_scale*0.1;
		}
		else
		{
			wished_scale -= wished_scale*0.1;
			if (wished_scale < 1)
			{
				wished_scale = 1.0 + (1.0-wished_scale);
				wished_zoomed_in = true;
			}
		}
	}
	else if (event->delta()<0)
	{
		/*
		if (wished_zoomed_in)
		{
			wished_scale -= wished_scale*0.1;
			if (wished_scale < 1)
			{
				wished_scale = 1.0 + (1.0-wished_scale);
				wished_zoomed_in = false;
			}
		}
		else
		{
			if (wished_scale > 20.0) return;
			wished_scale += wished_scale*0.1;
		}
		*/
		if (wished_zoomed_in)
		{
			wished_scale -= wished_scale*0.1;
			if (wished_scale < 1)
			{
				wished_scale = 1.0 + (1.0-wished_scale);
				wished_zoomed_in = false;
			}
		}
		else
		{
			wished_scale += wished_scale*0.1;
		}

		if ((lower_scaling_zoomed_in == true) && (wished_scale < 0.9*lower_scaling_limit) )
		{
			wished_scale = 0.9*lower_scaling_limit;
			return;
		}

		if ((lower_scaling_zoomed_in == false) && (wished_scale > 1.1*lower_scaling_limit) )
		{
			wished_scale = 1.1*lower_scaling_limit;
			return;
		}
	}

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
	//updateContent();
}

// scale widget window
void FileMiniMap::scaleView(qreal scaleFactor)
{
/*
	if (scaleFactor < 0) scaleFactor = 0.5;
	//qDebug(QString::number(scaleFactor).toLatin1());
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	
	if (factor < 0.07 || factor > 100)
        return;
*/

    //scale(scaleFactor, scaleFactor);
	QTransform trans;
	trans.scale(scaleFactor, scaleFactor);
	setTransform(trans, true);

	calculateLevelOfDetail();
}

void FileMiniMap::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx,dy);
}

/*
void FileMiniMap::setScrollBarValue(qreal val)
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
*/
void FileMiniMap::updateContent()
{
	if (mainWidget->mViewMode == 1)
	{
		m_intrect->updateIntRect();		// update size and position
		mScene->update(mScene->sceneRect());
	}
}

void FileMiniMap::setFileView(FileView *view)
{
	mFileView = view;
}

// get list of all visible node graphics in file view
QList<FileMiniMapNodeGraphic*> FileMiniMap::getVisibleNodeGraphics()
{
	// get path of visible area
	QPainterPath path;
	path.addRect(QRectF(this->rect()));
	QPainterPath scenePath;
	scenePath = this->mapToScene(path);

	QList<FileMiniMapNodeGraphic*> visibleItems;

	// check each node graphics item, if it is visible
	for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFileViewGraphicData[v].graphicsMiniMap->collidesWithPath(
			mainWidget->mFileViewGraphicData[v].graphicsMiniMap->mapFromScene(scenePath),
			Qt::IntersectsItemShape))
		{
			visibleItems.push_back(mainWidget->mFileViewGraphicData[v].graphicsMiniMap);
			//qDebug(mainWidget->mFileViewGraphicData[v].graphicsMiniMap->Label().toLatin1());
		}
	}
	return visibleItems;
}

//calculate level of detail
void FileMiniMap::calculateLevelOfDetail()
{	
	if (mainWidget->projectLoaded == true)
	{
	ogdf::node v = mainWidget->mFileGraph.firstNode();
	//calculate size of graphics item in screen coordinates
	QPointF itemCoordTopLeft = mainWidget->mFileViewGraphicData[v].graphicsMiniMap->mapToScene(
		mainWidget->mFileViewGraphicData[v].graphicsMiniMap->boundingRect().topLeft() );

	QPointF itemCoordBottomRight = mainWidget->mFileViewGraphicData[v].graphicsMiniMap->mapToScene(
		mainWidget->mFileViewGraphicData[v].graphicsMiniMap->boundingRect().bottomRight() );

	QPoint itemWidgetCoordTopLeft = mapFromScene(itemCoordTopLeft);
	QPoint itemWidgetCoordBottomRight = mapFromScene(itemCoordBottomRight);
		
	QPoint itemScreenCoordTopLeft = mapToGlobal(itemWidgetCoordTopLeft);
	QPoint itemScreenCoordBottomRight = mapToGlobal(itemWidgetCoordBottomRight);

	int itemWidth = itemScreenCoordBottomRight.x() - itemScreenCoordTopLeft.x();

	if (itemWidth <= 20)
	{
		//LOD = 0
		if (LOD != 0)
		{
			LOD = 0;
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphicsMiniMap->updateLOD();
			}
		}
		//qDebug("LOD = 0");
		return;
	}
	else if ((itemWidth > 20) && (itemWidth <= 40))
	{
		//LOD = 1
		if (LOD != 1)
		{
			LOD = 1;
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphicsMiniMap->updateLOD();
			}
		}
		//qDebug("LOD = 1");
		return;
	}
	else if ((itemWidth > 40) && (itemWidth <= 70))
	{
		//LOD = 2
		if (LOD != 2)
		{
			LOD = 2;
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphicsMiniMap->updateLOD();
			}
		}
		//qDebug("LOD = 2");
		return;
	}
	else if (itemWidth > 70)
	{
		//LOD = 3
		if (LOD != 3)
		{
			LOD = 3;
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphicsMiniMap->updateLOD();
			}
		}
		//qDebug("LOD = 3");
		return;
	}
	}
}

void FileMiniMap::calculateScalingLimits()
{

	if (mainWidget->projectLoaded == false)
		return;

	// calculate lower limit from scene width
	qreal s_factor_w = qreal(this->geometry().width())/this->sceneRect().width();
	qreal s_factor_h = qreal(this->geometry().height())/this->sceneRect().height();
	qreal s_factor;
	if (s_factor_h < s_factor_w)
		s_factor = s_factor_h;
	else s_factor = s_factor_w;

	lower_scaling_limit = 1/s_factor;
	lower_scaling_zoomed_in = false;

	// calculate upper limit from smallest scene item
	qreal smallest_width = mainWidget->mFileViewGraphicData[mainWidget-> mFileGraph.firstNode()].graphicsMiniMap->sceneBoundingRect().width();
	qreal smallest_height = mainWidget->mFileViewGraphicData[mainWidget-> mFileGraph.firstNode()].graphicsMiniMap->sceneBoundingRect().height();

	for(ogdf::node v = mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFileViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width() < smallest_width)
			smallest_width = mainWidget->mFileViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width();
		if (mainWidget->mFileViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height() < smallest_height)
			smallest_height = mainWidget->mFileViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height();
	}

	qreal item_factor_w = smallest_width/qreal(this->geometry().width());
	qreal item_factor_h = smallest_height/qreal(this->geometry().height());
	qreal item_factor;
	if (item_factor_h < item_factor_w)
		item_factor = item_factor_h;
	else item_factor = item_factor_w;

	upper_scaling_limit = 1/item_factor;
	upper_scaling_zoomed_in = true;
}