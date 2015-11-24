

#include <QGraphicsScene>
#include "SMVMain.h"
#include "FeatureViewNodeGraphic.h"
#include "FileView.h"
#include "FeatureView.h"
#include "FeatureViewInteractiveRect.h"
#include "FeatureMiniMap.h"

// animation resolution
const qreal RESOLUTION = 0.1;

FeatureView::FeatureView(SMVMain *graphWidget)
	: timerId(0)
{
	mainWidget = graphWidget;

    if (mainWidget)
        startColor = mainWidget->palette().window().color();
    else
        startColor = Qt::white;

	// general fading
	currentAlpha = 0;
	duration = 8000;
	//mTimer = new QTimer(this);
	mTimerID = 0;
	filezoom_TimerID = 0;
	timeinterval = 50;
	currentinterval = 0;

	fading_in_active = false;
	fading_out_active = false;
	nextWidgetIndexToFadeIn = -1;

	wishedZoomInFeatureGraphic = 0;

	// animation: zoom into feature node
	filezoom_steps = 20;

	this->setAcceptDrops(true);

	mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
	mScene->setSceneRect(-this->rect().width()/2,-this->rect().height()/2,this->rect().width(),this->rect().height());
	setScene(mScene);

	setRenderHint(QPainter::Antialiasing,true);
	setRenderHint(QPainter::TextAntialiasing, true);

	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	// initialize scale of file view
	initial_scale = viewportTransform().m11();
	wished_scale = initial_scale;
	current_scale = initial_scale;

	lower_scaling_limit = initial_scale;
	lower_scaling_zoomed_in = true;
	upper_scaling_limit = initial_scale;
	upper_scaling_zoomed_in = true;

	//showFullScene();

	//stripe0();

	// set initial value for panning
	isPanning = false;

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QSize areaSize = viewport()->size();
	QSize widgetSize = this->size();

	verticalScrollBar()->setPageStep(widgetSize.height());
	horizontalScrollBar()->setPageStep(widgetSize.width());
	verticalScrollBar()->setRange(0, widgetSize.height() - areaSize.height());
	horizontalScrollBar()->setRange(0, widgetSize.width() - areaSize.width());
}

FeatureView::~FeatureView(void)
{
}

// scale content of widget so that scene is fully visible
void FeatureView::showFullScene()
{
	if (mainWidget->projectLoaded == false)
		return;

	calculateScalingLimits();

	wished_scale = lower_scaling_limit;
	wished_zoomed_in = lower_scaling_zoomed_in;

	//stripe0();

	updateButtons();

	// start timer
	if (timerId == 0)
	{
        timerId = startTimer(1000/25); // timer started
	}
}

// draw background of widget window
void FeatureView::drawBackground(QPainter *painter, const QRectF &rect)
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

// Qt timer event
void FeatureView::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);


	if (event->timerId() == filezoom_TimerID)
	{
		filezoom_counter++;
		if (fading_out_active == true)
		{
			//qDebug() << "fading out";
			if (filezoom_counter >= filezoom_steps)
			{
				mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = 255;
			}
			
			else
			{
				mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = 
					(int)( (qreal)mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha + filezoom_fade_interval );
			}
			
			mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->update(mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->boundingRect());
			//qDebug() << "counter = " << filezoom_counter;
			//qDebug() << "alpha = " << mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha;

			filezoom_current_scale += filezoom_scale_interval;

			

			if (filezoom_wished_zoomed_in == true)
			{
				QTransform trans;
				trans.scale(filezoom_current_scale, filezoom_current_scale);
				setTransform(trans, false);
			}
			else
			{
				qreal scaleFactor = 1 / filezoom_current_scale;
				
				QTransform trans;
				trans.scale(scaleFactor, scaleFactor);
				setTransform(trans, false);
			}

			current_scale = viewportTransform().m11();
			wished_zoomed_in = filezoom_wished_zoomed_in;
			wished_scale = current_scale;

			//qDebug() << "scale after transform = " << current_scale;
			//qDebug() << "filezoom_current_scale = " << filezoom_current_scale;

			centerOn(mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics);

			updateButtons();
			mainWidget->mFeatureMiniMap->updateContent();

			if (filezoom_counter >= filezoom_steps)
			{
				if (filezoom_TimerID)
				{
					killTimer(filezoom_TimerID);
					filezoom_TimerID = 0;

					//qDebug() << "switching to fileview...";

					mainWidget->showFileView();
				}
			}
			return;
		}
		if (fading_in_active == true)
		{
			//qDebug() << "fading in";
			if (filezoom_counter >= filezoom_steps)
			{
				mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = 0;
				mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->drawfilescenepreview = false;

			}
			else
			{
				mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = 
					(int)( (qreal)mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha - filezoom_fade_interval );

			}
			mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->update(mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->boundingRect());
			filezoom_current_scale += filezoom_scale_interval;

			if (filezoom_wished_zoomed_in == true)
			{
				QTransform trans;
				trans.scale(filezoom_current_scale, filezoom_current_scale);
				setTransform(trans, false);
			}
			else
			{
				qreal scaleFactor = 1 / filezoom_current_scale;
				
				QTransform trans;
				trans.scale(scaleFactor, scaleFactor);
				setTransform(trans, false);
			}

			current_scale = viewportTransform().m11();
			wished_zoomed_in = filezoom_wished_zoomed_in;
			wished_scale = current_scale;

			centerOn(mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics);

			updateButtons();
			mainWidget->mFeatureMiniMap->updateContent();

			if (filezoom_counter >= filezoom_steps)
			{
				if (filezoom_TimerID)
				{
					killTimer(filezoom_TimerID);
					filezoom_TimerID = 0;

					fading_in_active = false;
					setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
				}
			}

			return;
		}
	}

	if (event->timerId() == mTimerID)		// mTimerID, update foreground brush
	{
		currentinterval += timeinterval;
		updateForegroundBrush();
		if (fading_out_active == true)
		{
			/*
			scaleView(1.1);
			current_scale = viewportTransform().m11();
			wished_scale = current_scale;
			updateButtons();
			*/
			mainWidget->mFeatureMiniMap->updateContent();
			return;
		}
		if (fading_in_active == true)
		{
			//scaleView(0.9);
			//current_scale = viewportTransform().m11();
			//wished_scale = current_scale;
			//updateButtons();
			mainWidget->mFeatureMiniMap->updateContent();
			return;
		}
	}

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
			updateButtons();
			
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
				updateButtons();
			}

			else
			{
				// zoom in
				zoom_speed = (1/current_scale-1.0)-(wished_scale-1.0);
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 + zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
				updateButtons();
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
				updateButtons();
			}
			else
			{
				// zoom in
				zoom_speed = wished_scale - current_scale;
				if (zoom_speed > 1.0) zoom_speed = 1.0;
				scaleView(1.0 + zoom_speed * RESOLUTION);
				current_scale = viewportTransform().m11();
				updateButtons();
			}
		}
		else
		{
			// zoom out
			zoom_speed = (wished_scale-1.0) + (current_scale-1.0);
			if (zoom_speed > 1.0) zoom_speed = 1.0;
			scaleView(1.0 - zoom_speed * RESOLUTION);
			current_scale = viewportTransform().m11();
			updateButtons();
		}
	}

	/*
	qDebug() << "wished_scale" << wished_scale;
	qDebug() << "wished_zoomed_in" << wished_zoomed_in;
	qDebug() << "current_scale" << current_scale;
	qDebug() << "upper_scaling_limit" << upper_scaling_limit;
	qDebug() << "lower_scaling_limit" << lower_scaling_limit;
	*/

	mainWidget->mFeatureMiniMap->updateContent();
}

// Qt keyboard events
void FeatureView::keyPressEvent(QKeyEvent *event)
{
	QGraphicsView::keyPressEvent(event);
}

// Qt mouse press event
void FeatureView::mousePressEvent(QMouseEvent *event)
{
	if ((fading_in_active == true) || (fading_out_active == true))
		return;

#if 0
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

	if (!hovered_item_exists) // no item under mouse exists
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
		//QGraphicsView::mousePressEvent(event);
		setCursor(Qt::SizeAllCursor);
	}
#endif

	if (event->button() == Qt::LeftButton)
	{
		// panning started
		isPanning = true;
		dragStartPosition = QPointF(event->pos().x(), event->pos().y());
		setCursor(Qt::ClosedHandCursor);
	}

	QGraphicsView::mousePressEvent(event);
}


// Qt mouse movement event
void FeatureView::mouseMoveEvent(QMouseEvent *event)
{
	if (isPanning) // is panning running ?
	{
		/* TODO
		** Bug : panning runs with wrong velocity, if zooming runs at same time
		*/

		QPointF p = event->pos();
		qreal deltaX ( ( p.x() - dragStartPosition.x() ));
		qreal deltaY ( ( p.y() - dragStartPosition.y() ));

		horizontalScrollBar()->setValue( horizontalScrollBar()->value() - deltaX );
		verticalScrollBar()->setValue( verticalScrollBar()->value() - deltaY ); 
		dragStartPosition = QPointF(event->pos().x(), event->pos().y());

		mainWidget->mFeatureMiniMap->updateContent();
	}
	// forward signal
	QGraphicsView::mouseMoveEvent(event);
}

// Qt mouse release event
void FeatureView::mouseReleaseEvent(QMouseEvent* event)
{
	if (isPanning) // is panning running ?
	{
		setCursor(Qt::ArrowCursor);
		isPanning = false;
	}
	else // panning is not running
	{
		setCursor(Qt::ArrowCursor);
	}
	// forward signal
	QGraphicsView::mouseReleaseEvent(event);
}

// Qt wheel event
void FeatureView::wheelEvent(QWheelEvent *event)
{

	if ((fading_in_active == true) || (fading_out_active == true))
	{
		return;
	}

	// zooming / set wished scale
	if (event->delta() > 0)
	{
		if (transformationAnchor() != QGraphicsView::AnchorUnderMouse)
			setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

		if (wished_zoomed_in)
		{
			/*
			if ((wished_scale > 3.0*upper_scaling_limit) && (getVisibleNodeGraphics().size() == 0))
			{
				wished_scale =  3.0*upper_scaling_limit;
				return;
			}
			*/
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
	else if (event->delta() < 0)
	{
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

		if ((lower_scaling_zoomed_in == true) && (wished_scale < lower_scaling_limit) )
		{
			//qDebug() << "wheel wished_scale = " << wished_scale;
			//qDebug() <<  "wheel lower_scaling_limit = " << lower_scaling_limit;
			wished_scale = lower_scaling_limit;
			return;
		}
	}

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}

// scale widget window
void FeatureView::scaleView(qreal scaleFactor)
{
	//if (scaleFactor < 0) scaleFactor = 0.5;

    //qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	
	/*
    if (factor < 0.07 || factor > 100)
	{
		//qDebug("FeatureView (scaleView): factor extends limit!");
		//qDebug(QString::number(factor).toLatin1());
        return;
	}
	*/

	if (wishedZoomInFeatureGraphic != 0)
	{
		setTransformationAnchor(QGraphicsView::AnchorViewCenter);
		centerOn(wishedZoomInFeatureGraphic);
		QTransform trans;
		trans.scale(scaleFactor, scaleFactor);
		setTransform(trans, true);
		centerOn(wishedZoomInFeatureGraphic);
		setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	}
	else
	{
		QTransform trans;
		trans.scale(scaleFactor, scaleFactor);
		setTransform(trans, true);
	}

	if (mainWidget->projectLoaded == true)
	{
		if ((fading_in_active == true) || (fading_out_active == true))
			return;

		if ((wished_zoomed_in == upper_scaling_zoomed_in) && (current_scale > upper_scaling_limit))
		{
			if (this->wishedZoomInFeature.length() > 0)
			{
				mainWidget->zoomIntoFeature(wishedZoomInFeature);
				wishedZoomInFeature.clear();
				wishedZoomInFeatureGraphic = 0;

				return;
			}
		}
		if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > upper_scaling_limit))
		{
			// seek feature to zoom into
			QList<FeatureViewNodeGraphic*> visGraphics = getVisibleNodeGraphics();
			if (visGraphics.size() == 1)
			{
				// get feature name
				FeatureViewNodeGraphic *graphic = visGraphics.value(0);
				// zoom into feature
				if (graphic->isFeature()) // activatible feature
				{
					if ((graphic->getActivation() == true) && (wished_scale > current_scale))
					{
						mainWidget->zoomIntoFeature(graphic->Label());
					}
					else
					{
						if (wished_scale > current_scale)
						{
							QString statusmsg;
							statusmsg.append("Feature ''");
							statusmsg.append(graphic->Label());
							statusmsg.append("'' not active!");

							QMessageBox msgBox;
							msgBox.setText(statusmsg);
							msgBox.exec();
							stripe9();
						}
					}
				}
				else	// feature tree
				{
					mainWidget->zoomIntoFeature(graphic->Label());
				}
			}
		}
	}
}

// get list of all visible node graphics in feature view
QList<FeatureViewNodeGraphic*> FeatureView::getVisibleNodeGraphics()
{
	// get path of visible area
	QPainterPath path;
	path.addRect(QRectF(this->rect()));
	QPainterPath scenePath;
	scenePath = this->mapToScene(path);

	QList<FeatureViewNodeGraphic*> visibleItems;

	// check each node graphics item, if it is visible
	for(ogdf::node v = mainWidget->mFeatureGraph->firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFeatureViewGraphicData[v].graphics->collidesWithPath(
			mainWidget->mFeatureViewGraphicData[v].graphics->mapFromScene(scenePath),
			Qt::IntersectsItemShape))
		{
			visibleItems.push_back(mainWidget->mFeatureViewGraphicData[v].graphics);
		}
	}
	return visibleItems;
}

void FeatureView::zmButton()
{
	switch (aktivButton)
	{
	case 1:
		stripe0();
		break;
	case 2:
		stripe1();
		break;
	case 3:
		stripe2();
		break;
	case 4:
		stripe3();
		break;
	case 5:
		stripe4();
		break;
	case 6:
		stripe5();
		break;
	case 7:
		stripe6();
		break;
	case 8:
		stripe7();
		break;
	case 9:
		stripe8();
		break;
	case 10:
		stripe9();
		break;
	}
}

void FeatureView::zpButton()
{
	switch (aktivButton)
	{
	case 0:
		stripe1();
		break;
	case 1:
		stripe2();
		break;
	case 2:
		stripe3();
		break;
	case 3:
		stripe4();
		break;
	case 4:
		stripe5();
		break;
	case 5:
		stripe6();
		break;
	case 6:
		stripe7();
		break;
	case 7:
		stripe8();
		break;
	case 8:
		stripe9();
		break;
	case 9:
		stripe10();
		break;
	case 10:
		this->stripe10();
		break;
	}
}
void FeatureView::stripe0()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe11.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button0_upper;

	aktivButton=0;
	
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}

void FeatureView::stripe1()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button0_upper+((interval_button1_upper-interval_button0_upper)/2);
	aktivButton=1;
	
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}

void FeatureView::stripe2()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button1_upper+((interval_button2_upper-interval_button1_upper)/2);
	aktivButton=2;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}
void FeatureView::stripe3()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button2_upper+((interval_button3_upper-interval_button2_upper)/2);
	aktivButton=3;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}

}

void FeatureView::stripe4()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button3_upper+((interval_button4_upper-interval_button3_upper)/2);
	aktivButton=4;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FeatureView::stripe5()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button4_upper+((interval_button5_upper-interval_button4_upper)/2);
	aktivButton=5;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FeatureView::stripe6()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = lower_scaling_zoomed_in;
	wished_scale = interval_button5_upper+((interval_button6_upper-interval_button5_upper)/2);
	aktivButton=6;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}

void FeatureView::stripe7()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = upper_scaling_zoomed_in;
	wished_scale = interval_button6_upper+((interval_button7_upper-interval_button6_upper)/2);
	aktivButton=7;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FeatureView::stripe8()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = upper_scaling_zoomed_in;
	wished_scale = interval_button7_upper+((interval_button8_upper-interval_button7_upper)/2);
	aktivButton=8;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FeatureView::stripe9()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));

	wished_zoomed_in = upper_scaling_zoomed_in;
	wished_scale = interval_button8_upper+((interval_button9_upper-interval_button8_upper)/2);

	aktivButton=9;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FeatureView::stripe10()
{
	mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
	mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
	mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
	mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
	mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
	mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
	mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
	mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
	mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
	mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe10.png"));

	wished_zoomed_in = upper_scaling_zoomed_in;
	wished_scale = interval_button9_upper + ((upper_scaling_limit-interval_button9_upper)/2);

	aktivButton=10;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}
void FeatureView::setScrollBarValue(qreal x_val, qreal y_val)
{
	// set horizontal scrollbar
	if (this->horizontalScrollBar()->maximum() > this->horizontalScrollBar()->minimum())
	{
	this->horizontalScrollBar()->setValue(horizontalScrollBar()->minimum()+
		x_val*(horizontalScrollBar()->maximum()-horizontalScrollBar()->minimum()));
	}
	else if (this->horizontalScrollBar()->minimum() > this->horizontalScrollBar()->maximum())
	{
		this->horizontalScrollBar()->setValue(horizontalScrollBar()->maximum()+
			x_val*(horizontalScrollBar()->minimum()-horizontalScrollBar()->maximum()));
	}

	// set vertical scrollbar
	if (this->verticalScrollBar()->maximum() > this->verticalScrollBar()->minimum())
	{
	this->verticalScrollBar()->setValue(verticalScrollBar()->minimum()+
		y_val*(verticalScrollBar()->maximum()-verticalScrollBar()->minimum()));
	}
	else if (this->verticalScrollBar()->minimum() > this->verticalScrollBar()->maximum())
	{
		this->verticalScrollBar()->setValue(verticalScrollBar()->maximum()+
			y_val*(verticalScrollBar()->minimum()-verticalScrollBar()->maximum()));
	}
}
void FeatureView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	mainWidget->mFeatureMiniMap->updateContent();
	calculateScalingLimits();
	showFullScene();
}

void FeatureView::zoomIntoFileView(qreal zoom_wished_scale, bool zoom_wished_zoomed_in, ogdf::node feature_node)
{
	if (timerId)
	{
		killTimer(timerId);
		timerId = 0;
	}

	//qDebug() << "zoomIntoFileView...";

	filezoom_wished_zoomed_in = zoom_wished_zoomed_in;
	filezoom_feature_node = feature_node;

	fading_out_active = true;
	if (fading_in_active == true) fading_in_active = false;

	filezoom_current_scale = viewportTransform().m11();

	//qDebug() << "current scale " << filezoom_current_scale;
	//qDebug() << "zoom_wished_scale scale " << zoom_wished_scale;

	qreal filezoom_diff = zoom_wished_scale - filezoom_current_scale;
	//qDebug() << "diff scale = " << filezoom_diff;

	filezoom_scale_interval = filezoom_diff / filezoom_steps;
	//qDebug() << "filezoom_scale_interval = " << filezoom_scale_interval;
	filezoom_fade_interval = (255.0 / (qreal)filezoom_steps);
	//qDebug() << "filezoom_fade_interval = " << filezoom_fade_interval;

	mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = filezoom_fade_interval;

	setTransformationAnchor(QGraphicsView::AnchorViewCenter);


	filezoom_counter = 0;

	// start timer
	if (!filezoom_TimerID)
	{
		//qDebug() << "starting timer...";
        filezoom_TimerID = startTimer(timeinterval); // timer started
	}
}

void FeatureView::zoomIntoFeatureView()
{
	mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->preview_pixmap_alpha = 255;
	mainWidget->mFeatureViewGraphicData[filezoom_feature_node].graphics->drawfilescenepreview = true;
	mainWidget->showFeatureView();

	calculateScalingLimits();
	updateButtons();

	setTransformationAnchor(QGraphicsView::AnchorViewCenter);

	filezoom_wished_zoomed_in = true;
	qreal zoom_wished_scale = scale_after_zoom_from_other_view;

	wished_scale		= scale_after_zoom_from_other_view;
	wished_zoomed_in	= true;

	fading_in_active = true;
	if (fading_out_active == true) fading_out_active = false;

	filezoom_current_scale = viewportTransform().m11();

	qreal filezoom_diff = zoom_wished_scale - filezoom_current_scale;

	filezoom_scale_interval = filezoom_diff / filezoom_steps;

	filezoom_fade_interval = (255.0 / (qreal)filezoom_steps);

	filezoom_counter = 0;

	// start timer
	if (!filezoom_TimerID)
	{
		//qDebug() << "starting timer...";
        filezoom_TimerID = startTimer(timeinterval); // timer started
	}
}

void FeatureView::fadeOut()
{
	if (timerId)
	{
		killTimer(timerId);
		timerId = 0;
	}

	currentAlpha = 0;
	fading_out_active = true;
	if (fading_in_active == true) fading_in_active = false;

	// start timer
	if (!mTimerID)
	{
        mTimerID = startTimer(timeinterval); // timer started
		currentinterval = 0;
	}
}

void FeatureView::fadeIn()
{
	/*
	if (timerId)
	{
		killTimer(timerId);
		timerId = 0;
	}
	*/

	calculateScalingLimits();
	updateButtons();


	wished_scale		= scale_after_zoom_from_other_view;
	wished_zoomed_in	= true;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}

	currentAlpha = 255;
	fading_in_active = true;
	if (fading_out_active == true) fading_out_active = false;

	if (!mTimerID)
	{
        mTimerID = startTimer(timeinterval); // timer started
		currentinterval = 0;
	}
}

void FeatureView::setForegroundTransparent()
{
	currentAlpha = 0;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	QBrush b(semiTransparentColor);
	setForegroundBrush(b);
	repaint();
}
void FeatureView::setForegroundOpaque()
{
	currentAlpha = 255;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	QBrush b(semiTransparentColor);
	setForegroundBrush(b);
	repaint();
}
void FeatureView::updateForegroundBrush()
{
	if (fading_in_active == true)
	{
		currentAlpha -= 255 * currentinterval / duration;		// fading in

		if (currentAlpha <= 0) {

			fading_in_active = false;

			currentAlpha = 0;
			killTimer(mTimerID);
			mTimerID = 0;
		}
	}
	else if (fading_out_active == true)
	{
		currentAlpha += 255 * currentinterval / duration;		// fading out
		
		if (currentAlpha >= 255) {

			fading_out_active = false;

			currentAlpha = 255;
			killTimer(mTimerID);
			mTimerID = 0;
			
			wishedZoomInFeatureGraphic = 0;


			switch(nextWidgetIndexToFadeIn)
			{
			case 1:												// fading in file view
				nextWidgetIndexToFadeIn = -1;
				mainWidget->mFileView->fading_in_from_featureview = true;
				mainWidget->fadeInFileView();
				break;
			case 2:												// fading in code view
				nextWidgetIndexToFadeIn = -1;
				mainWidget->fadeInCodeView();
				break;
			default:
				nextWidgetIndexToFadeIn = -1;
				fadeIn();
				break;
			}
		}
	}

    QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	QBrush b(semiTransparentColor);
	setForegroundBrush(b);
	
	repaint();
}
void FeatureView::setNextWidgetToFadeIn(int widget_index)
{
	nextWidgetIndexToFadeIn = widget_index;
}
void FeatureView::updateButtons()
{
	if ((lower_scaling_zoomed_in == true) && (upper_scaling_zoomed_in == true))
	{
		if((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale <= interval_button0_upper) && (aktivButton != 0))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe11.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=0;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button0_upper) && (wished_scale <= interval_button1_upper) && (aktivButton != 1))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=1;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button1_upper) && (wished_scale <= interval_button2_upper) && (aktivButton != 2))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=2;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button2_upper) && (wished_scale <= interval_button3_upper) && (aktivButton != 3))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=3;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button3_upper) && (wished_scale <= interval_button4_upper) && (aktivButton != 4))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=4;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button4_upper) && (wished_scale <= interval_button5_upper) && (aktivButton != 5))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=5;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > interval_button5_upper) && (wished_scale <= interval_button6_upper) && (aktivButton != 6))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=6;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button6_upper) && (wished_scale <= interval_button7_upper) && (aktivButton != 7))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=7;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button7_upper) && (wished_scale <= interval_button8_upper) && (aktivButton != 8))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=8;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button8_upper) && (wished_scale <= interval_button9_upper) && (aktivButton != 9))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=9;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button9_upper) && (aktivButton != 10))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe10.png"));
			aktivButton=10;
			return;
		}
	}
	else if ((lower_scaling_zoomed_in == false) && (upper_scaling_zoomed_in == true))
	{
		if((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button0_lower) && (aktivButton != 0))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe11.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=0;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button1_lower) && (wished_scale < interval_button0_lower) && (aktivButton != 1))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe12.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=1;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button2_lower) && (wished_scale < interval_button1_lower) && (aktivButton != 2))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe13.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=2;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button3_lower) && (wished_scale < interval_button2_lower) && (aktivButton != 3))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe14.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=3;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button4_lower) && (wished_scale < interval_button3_lower) && (aktivButton != 4))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe15.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=4;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= interval_button5_lower) && (wished_scale < interval_button4_lower) && (aktivButton != 5))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe16.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=5;
			return;
		}
		else if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale >= 1) && (wished_scale < interval_button5_lower) && (aktivButton != 6))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe17.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=6;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale >= 1) && (wished_scale < interval_button7_upper) && (aktivButton != 7))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe18.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=7;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button7_upper) && (wished_scale <= interval_button8_upper) && (aktivButton != 8))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe19.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=8;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale >interval_button8_upper) && (wished_scale <= interval_button9_upper) && (aktivButton != 9))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe.png"));
			aktivButton=9;
			return;
		}
		else if ((wished_zoomed_in == upper_scaling_zoomed_in) && (wished_scale > interval_button9_upper) && (aktivButton != 10))
		{
			mainWidget->Stripe1->setIcon(QIcon(".\\graphics\\Stripe1.png"));
			mainWidget->Stripe2->setIcon(QIcon(".\\graphics\\Stripe2.png"));
			mainWidget->Stripe3->setIcon(QIcon(".\\graphics\\Stripe3.png"));
			mainWidget->Stripe4->setIcon(QIcon(".\\graphics\\Stripe4.png"));
			mainWidget->Stripe5->setIcon(QIcon(".\\graphics\\Stripe5.png"));
			mainWidget->Stripe6->setIcon(QIcon(".\\graphics\\Stripe6.png"));
			mainWidget->Stripe7->setIcon(QIcon(".\\graphics\\Stripe7.png"));
			mainWidget->Stripe8->setIcon(QIcon(".\\graphics\\Stripe8.png"));
			mainWidget->Stripe9->setIcon(QIcon(".\\graphics\\Stripe9.png"));
			mainWidget->Stripe10->setIcon(QIcon(".\\graphics\\Stripe10.png"));
			aktivButton=10;
			return;
		}
	}
	else
	{
		qDebug() << "ERROR: no configuration for button labels!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
	}
}
void FeatureView::calculateScalingLimits()
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

	//qDebug() << "lower_scaling_limit = " << lower_scaling_limit;
	//qDebug() << "lower_scaling_zoomed_in = " << lower_scaling_zoomed_in;

	// calculate upper limit from smallest scene item
	qreal smallest_width = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphics->sceneBoundingRect().width();
	qreal smallest_height = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphics->sceneBoundingRect().height();

	qreal largest_width = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphics->sceneBoundingRect().width();
	qreal largest_height = mainWidget->mFeatureViewGraphicData[mainWidget->mFeatureGraph->firstNode()].graphics->sceneBoundingRect().height();

	for(ogdf::node v = mainWidget->mFeatureGraph->firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().width() < smallest_width)
			smallest_width = mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().width();
		if (mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().height() < smallest_height)
			smallest_height = mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().height();

		if (mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().width() > largest_width)
			largest_width = mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().width();
		if (mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().height() > largest_height)
			largest_height = mainWidget->mFeatureViewGraphicData[v].graphics->sceneBoundingRect().height();
	}
/*
	qreal item_factor_w = smallest_width/qreal(this->geometry().width());
	qreal item_factor_h = smallest_height/qreal(this->geometry().height());
	qreal item_factor;
	if (item_factor_h > item_factor_w)
		item_factor = item_factor_h;
	else item_factor = item_factor_w;

	upper_scaling_limit = 1/item_factor;
	upper_scaling_zoomed_in = true;


	qDebug() << "upper_scaling_limit = " << upper_scaling_limit;
	qDebug() << "upper_scaling_zoomed_in = " << upper_scaling_zoomed_in;
*/
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


	//qDebug() << "upper_scaling_limit = " << upper_scaling_limit;
	//qDebug() << "upper_scaling_zoomed_in = " << upper_scaling_zoomed_in;

	// calculat button intervals
	if ((lower_scaling_zoomed_in == true) && (upper_scaling_zoomed_in == true))
	{
		qreal scaling_interval = (upper_scaling_limit - lower_scaling_limit) / 8.0;
		
		interval_button0_upper = 0.5*lower_scaling_limit;
		interval_button1_upper = lower_scaling_limit;
		interval_button2_upper = interval_button1_upper + scaling_interval;
		interval_button3_upper = interval_button2_upper + scaling_interval;
		interval_button4_upper = interval_button3_upper + scaling_interval;
		interval_button5_upper = interval_button4_upper + scaling_interval;
		interval_button6_upper = interval_button5_upper + scaling_interval;
		interval_button7_upper = interval_button6_upper + scaling_interval;
		interval_button8_upper = interval_button7_upper + scaling_interval;
		interval_button9_upper = interval_button8_upper + scaling_interval;
	}
	else if ((lower_scaling_zoomed_in == false) && (upper_scaling_zoomed_in == true))
	{
		qreal lower_scaling_interval = (lower_scaling_limit - 1.0) / 6.0;
		qreal upper_scaling_interval = (upper_scaling_limit - 1.0) / 4.0;

		interval_button0_lower = lower_scaling_limit;
		interval_button1_lower = interval_button0_lower - lower_scaling_interval;
		interval_button2_lower = interval_button1_lower - lower_scaling_interval;
		interval_button3_lower = interval_button2_lower - lower_scaling_interval;
		interval_button4_lower = interval_button3_lower - lower_scaling_interval;
		interval_button5_lower = interval_button4_lower - lower_scaling_interval;
		interval_button6_lower = interval_button5_lower - lower_scaling_interval;
		interval_button7_upper = 1 + upper_scaling_interval;
		interval_button8_upper = interval_button7_upper + upper_scaling_interval;
		interval_button9_upper = interval_button8_upper + upper_scaling_interval;
	}
	else
	{
		qDebug() << "ERROR: no configuration for button labels!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
		qDebug() << "lower_scaling_zoomed_in = " << lower_scaling_zoomed_in;
		qDebug() << "upper_scaling_zoomed_in = " << upper_scaling_zoomed_in;
	}
}