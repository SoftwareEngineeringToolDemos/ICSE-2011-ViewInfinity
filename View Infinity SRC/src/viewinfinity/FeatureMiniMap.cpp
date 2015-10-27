

#include <QGraphicsScene>
#include "SMVMain.h"
#include "FeatureViewNodeGraphic.h"
#include "FeatureView.h"
#include "FeatureViewInteractiveRect.h"
#include "FeatureMiniMap.h"

// animation resolution
const qreal RESOLUTION = 0.1;

FeatureMiniMap::FeatureMiniMap(QWidget  *parent)
	: timerId(0)
{
	//mainWidget = graphWidget;
	mParent = parent;
	mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
	mScene->setSceneRect(-this->rect().width()/2,-this->rect().height()/2,this->rect().width(),this->rect().height());
	setScene(mScene);

	setRenderHint(QPainter::Antialiasing,true);
	setRenderHint(QPainter::TextAntialiasing, true);
	//setRenderHint(QPainter::SmoothPixmapTransform, true);
	//setOptimizationFlag(DontClipPainter, false);
	//setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	// initialize navigator rectangle
	m_intrect = new FeatureViewInteractiveRect(this);
	mScene->addItem(m_intrect);
	m_intrect->setPos(QPointF(0.0,0.0));
	zoomlevel = 1.0f;

	m_intrect->rect_height = scene()->sceneRect().height();
	m_intrect->rect_width = scene()->sceneRect().width();

	// initialize scale of file view
	initial_scale = viewportTransform().m11();
	wished_scale = initial_scale;
	current_scale = initial_scale;

	//showFullScene();

	// set initial value for panning
	isPanning = false;

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

FeatureMiniMap::~FeatureMiniMap(void)
{
}

// scale content of widget so that scene is fully visible
void FeatureMiniMap::showFullScene()
{
	/*
	qreal s_factor = this->geometry().width()/this->sceneRect().width();
	QTransform trans;
	trans.reset();
	trans.scale(s_factor,s_factor);
	this->setTransform(trans, false);
	initial_scale = viewportTransform().m11();
	wished_scale = initial_scale;
	current_scale = initial_scale;
	wished_zoomed_in = false;

	// set initial values for zooming and panning
	isPanning = false;
	*/
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
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
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


void FeatureMiniMap::setMainWidget(SMVMain *mWidget)
{
	mainWidget = mWidget;
}

// draw background of widget window
void FeatureMiniMap::drawBackground(QPainter *painter, const QRectF &rect)
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

	/*
	painter->setPen(QPen(QBrush(Qt::red,Qt::BrushStyle(Qt::SolidLine)),5,Qt::SolidLine,Qt::RoundCap,Qt::BevelJoin));
	painter->drawPoint(mFeatureView->mapToScene(mFeatureView->rect().topLeft()));
	painter->drawPoint(mFeatureView->mapToScene(mFeatureView->rect().topRight()));
	painter->drawPoint(mFeatureView->mapToScene(mFeatureView->rect().bottomLeft()));
	painter->drawPoint(mFeatureView->mapToScene(mFeatureView->rect().bottomRight()));
	painter->drawPoint(mFeatureView->mapToScene(QPoint(
		(mFeatureView->rect().bottomRight().x()-mFeatureView->rect().bottomLeft().x())/2,
		(mFeatureView->rect().bottomRight().y()-mFeatureView->rect().topLeft().y())/2)
		));
	*/
}

// Qt resize event
void FeatureMiniMap::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	updateContent();
}
// Qt timer event
void FeatureMiniMap::timerEvent(QTimerEvent *event)
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
void FeatureMiniMap::keyPressEvent(QKeyEvent *event)
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
void FeatureMiniMap::mousePressEvent(QMouseEvent *event)
{
	bool hovered_item_exists = false;
	
	//for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
	//if (!mainWidget->mFeatureGraph->empty())
	//{
		/* TODO: hier müssen dann die FeatureViewNodeGraphics abgefragt werden
		if (mainWidget->getFeatureViewNodeGraphic(v)->isUnderMouse() && (mainWidget->mFeatureGA->labelNode(v).length()>0))
		for(ogdf::node v = mainWidget->mFeatureGraph->firstNode(); v; v = v->succ())
		{
			hovered_item_exists = true;
			break;
			if (mainWidget->getFeatureViewNodeGraphic(v)->isUnderMouse() && (mainWidget->mFeatureGA->labelNode(v).length()>0))
			{
				hovered_item_exists = true;
				break;
			}
		}
		*/
	//}

	if ((!hovered_item_exists) && (m_intrect->rect_active == false)) // no item under mouse exists
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
		if (event->button() == Qt::LeftButton)
			QGraphicsView::mousePressEvent(event);
		//setCursor(Qt::SizeAllCursor);
	}
}


// Qt mouse movement event
void FeatureMiniMap::mouseMoveEvent(QMouseEvent *event)
{
	if (isPanning) // is panning running ?
	{
		/* TODO
		** Bug : Panning erfolgt mit falscher Geschwindigkeit, wenn gleichzeitig gezoomed wird
		*/

		QPointF p = event->pos();
		qreal deltaX ( ( p.x() - dragStartPosition.x() ));
		qreal deltaY ( ( p.y() - dragStartPosition.y() ));

		horizontalScrollBar()->setValue( horizontalScrollBar()->value() - deltaX );
		verticalScrollBar()->setValue( verticalScrollBar()->value() - deltaY ); 
		dragStartPosition = QPointF(event->pos().x(), event->pos().y());
	}
	else // panning is not running
	{
		// forward signal
		QGraphicsView::mouseMoveEvent(event);
	}
}

// Qt mouse release event
void FeatureMiniMap::mouseReleaseEvent(QMouseEvent* event)
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
		setCursor(Qt::ArrowCursor);
	}
}

// Qt wheel event
void FeatureMiniMap::wheelEvent(QWheelEvent *event)
{


	qDebug() << wished_scale;
	// zooming / set wished scale
	if (event->delta()>0)
	{
/*
		if (wished_zoomed_in)
		{
			if (wished_scale > 10.0) return;
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
void FeatureMiniMap::scaleView(qreal scaleFactor)
{
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    //scale(scaleFactor, scaleFactor);
	QTransform trans;
	trans.scale(scaleFactor, scaleFactor);
	setTransform(trans, true);
}
void FeatureMiniMap::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx,dy);
}
void FeatureMiniMap::setFeatureView(FeatureView *view)
{
	mFeatureView = view;
}
/*
void FeatureMiniMap::setScrollBarValue(qreal val)
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
void FeatureMiniMap::updateContent()
{
	//qDebug() << "update content";

	if (mainWidget->mViewMode == 0)
	{
		m_intrect->updateIntRect();		// update size and position
		mScene->update(mScene->sceneRect());
	}
}

void FeatureMiniMap::calculateScalingLimits()
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

	if (s_factor < 1.0)
	{
		lower_scaling_limit = 1/s_factor;
		lower_scaling_zoomed_in = false;
	}
	else
	{
		lower_scaling_limit = s_factor;
		lower_scaling_zoomed_in = true;
	}

	// calculate upper limit from smallest scene item
	qreal smallest_width = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphicsMiniMap->sceneBoundingRect().width();
	qreal smallest_height = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphicsMiniMap->sceneBoundingRect().height();

	qreal largest_width = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphicsMiniMap->sceneBoundingRect().width();
	qreal largest_height = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphicsMiniMap->sceneBoundingRect().height();

	for(ogdf::node v = mainWidget->mFeatureGraph->firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width() < smallest_width)
			smallest_width = mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width();
		if (mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height() < smallest_height)
			smallest_height = mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height();

		if (mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width() > largest_width)
			largest_width = mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().width();
		if (mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height() > largest_height)
			largest_height = mainWidget->mFeatureViewGraphicData[v].graphicsMiniMap->sceneBoundingRect().height();
	}

	qreal largest_item_factor_w = largest_width/qreal(this->geometry().width());
	qreal largest_item_factor_h = largest_height/qreal(this->geometry().height());
	qreal largest_item_factor;
	if (largest_item_factor_h > largest_item_factor_w)
		largest_item_factor = largest_item_factor_h;
	else largest_item_factor = largest_item_factor_w;
	
	scale_after_zoom_from_other_view = 1/largest_item_factor;

	// changed: 07.08. -> maximum zoom determined by largest item (previously smallest item, as can be seen above)
	upper_scaling_limit = 1/largest_item_factor;
	upper_scaling_zoomed_in = true;
}