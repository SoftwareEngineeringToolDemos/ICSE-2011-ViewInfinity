

#include <QGraphicsScene>
#include <QString>
#include "SMVMain.h"
#include "FileViewNodeGraphic.h"
#include "FeatureViewNodeGraphic.h"
#include "FeatureView.h"
#include "CodeView.h"
#include "FileViewInteractiveRect.h"
#include "FileMiniMap.h"
#include "FileView.h"


// animation resolution
const qreal RESOLUTION = 0.1;

FileView::FileView(SMVMain *graphWidget)
	: timerId(0)
{
	mainWidget = graphWidget;

	if (mainWidget)
        startColor = mainWidget->palette().window().color();
    else
        startColor = Qt::white;

	currentAlpha = 0;
	duration = 8000;
	mTimerID = 0;
	timeinterval = 50;
	currentinterval = 0;

	fading_in_active = false;
	fading_out_active = false;
	nextWidgetIndexToFadeIn = -1;

	mScene = new QGraphicsScene(this);
    mScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

	mScene->setSceneRect(-this->rect().width()/2,-this->rect().height()/2,this->rect().width(),this->rect().height());
	setScene(mScene);

	LOD = 2;

	setRenderHint(QPainter::Antialiasing,true);
	setRenderHint(QPainter::TextAntialiasing, true);
	//setRenderHint(QPainter::SmoothPixmapTransform, true);

	//setOptimizationFlag(DontClipPainter, false);

	//setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);


	// initialize scale of file view
	initial_scale = viewportTransform().m11();
	wished_scale = initial_scale;
	current_scale = initial_scale;
	showFullScene();

	stripe0();

	// set initial value for panning
	isPanning = false;

	wishedZoomInFile = 0;
	zoomIntoFileGraphic = 0;

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

FileView::~FileView(void)
{
}

unsigned int FileView::getLOD()
{
	return LOD;
}

// draw background of widget window
void FileView::drawBackground(QPainter *painter, const QRectF &rect)
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
	// app title
	if (mainWidget->projectLoaded == true)
	{
		if (mainWidget->featureInFileView.length() > 0)
		{
			// Text
			QRectF textRect(sceneRect.left() + 25, sceneRect.top() + 25, 500, 100);
			//QStringList list1 = mainWidget->getPathAnnotationsXML().split("\\");
			//QString message(list1[list1.size()-1].toLatin1());

		    QFont font = painter->font();
			font.setBold(true);
			font.setPointSize(30);
			painter->setFont(font);
			painter->setPen(Qt::lightGray);
			painter->drawText(textRect, mainWidget->featureInFileView);
			painter->setPen(Qt::black);
			painter->drawText(textRect.translated(2, 2), mainWidget->featureInFileView);
		}
	}
}

// scale content of widget so that scene is fully visible
void FileView::showFullScene()
{
	if (mainWidget-> mFileGraph.empty())
		return;

	calculateScalingLimits();

	wished_scale = lower_scaling_limit;
	wished_zoomed_in = lower_scaling_zoomed_in;

	updateButtons();

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
	
}
// scale content of widget so that scene is fully visible
void FileView::showFullSceneInstant()
{
	if (mainWidget-> mFileGraph.empty())
		return;

	killTimer(timerId);
	timerId = 0;

	calculateScalingLimits();

	wished_scale = lower_scaling_limit;
	wished_zoomed_in = lower_scaling_zoomed_in;

	//updateButtons();

	current_scale = viewportTransform().m11();

	if (wished_zoomed_in == true)
	{
		
		qreal scaleFactor = wished_scale;

		QTransform trans;
		trans.scale(scaleFactor, scaleFactor);
		setTransform(trans, false);
	}
	else
	{
		qreal scaleFactor = 1 / wished_scale;
		
		QTransform trans;
		trans.scale(scaleFactor, scaleFactor);
		setTransform(trans, false);
	}

	current_scale = viewportTransform().m11();

	this->calculateLevelOfDetail();

	mainWidget->mFileMiniMap->updateContent();
}

// Qt timer event
void FileView::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

	if (event->timerId() == mTimerID)
	{
		currentinterval += timeinterval;
		updateForegroundBrush();
		
		if (fading_out_active == true)
		{

			if (nextWidgetIndexToFadeIn == 0)
			{
				/*
				scaleView(0.9);
				current_scale = viewportTransform().m11();
				wished_scale = current_scale;
				updateButtons();
				*/
				mainWidget->mFileMiniMap->updateContent();
				return;
			}
			
			
			if (nextWidgetIndexToFadeIn == 2)
			{
				/*
				scaleView(1.1);
				current_scale = viewportTransform().m11();
				wished_scale = current_scale;
				updateButtons();
				*/
				mainWidget->mFileMiniMap->updateContent();
				return;
			}
		}
	}

	/*
	if (wishedZoomInFile != 0)
	{
		QPoint p = mapToGlobal(mapFromScene(			
			wishedZoomInFile->scenePos() ) ) ;
		mainWidget->cursor().setPos(p);
	}
	*/

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

	mainWidget->mFileMiniMap->updateContent();

}

// Qt keyboard events
void FileView::keyPressEvent(QKeyEvent *event)
{   

	QGraphicsView::keyPressEvent(event);

}

// Qt mouse press event
void FileView::mousePressEvent(QMouseEvent *event)
{

	if ((fading_in_active == true) || (fading_out_active == true))
		return;

	bool hovered_item_exists = false;
	if (mainWidget->projectLoaded == true)
	{
		if (!mainWidget->mFileGraph.empty())
		{
			for(ogdf::node v = mainWidget->mFileGraph.firstNode(); v; v = v->succ())
			{
				if (mainWidget->getFileViewNodeGraphic(v)->isUnderMouse() && (mainWidget->mFileGA.labelNode(v).length()>0))
				{
					hovered_item_exists = true;
					break;
				}
			}
		}
	}

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
		QGraphicsView::mousePressEvent(event);
		setCursor(Qt::SizeAllCursor);
	}
}

// Qt mouse movement event
void FileView::mouseMoveEvent(QMouseEvent *event)
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

		mainWidget->mFileMiniMap->updateContent();

		/*
		// get path of visible area
		QPainterPath path;
		path.addRect(QRectF(this->rect()));
		QPainterPath scenePath;
		scenePath = this->mapToScene(path);
		

		// check each node graphics item, if it is visible
		for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
		{
			if (mainWidget->mFileViewGraphicData[v].graphics->collidesWithPath(
				mainWidget->mFileViewGraphicData[v].graphics->mapFromScene(scenePath),
				Qt::IntersectsItemShape))
			{
				//qDebug(mainWidget->mFileViewGraphicData[v].graphics->Label().toLatin1());
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
void FileView::mouseReleaseEvent(QMouseEvent* event)
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
void FileView::wheelEvent(QWheelEvent *event)
{

	if ((fading_in_active == true) || (fading_out_active == true))
	{
		return;
	}

	// zooming / set wished scale
	if (event->delta()>0)
	{
		if (transformationAnchor() != QGraphicsView::AnchorUnderMouse)
			setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

		if (wished_zoomed_in)
		{
			//if (wished_scale > 1.5*upper_scaling_limit) return;
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
			//if (wished_scale > 1.5*lower_scaling_limit) return;
			wished_scale += wished_scale*0.1;
		}
	}

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}

// scale widget window
void FileView::scaleView(qreal scaleFactor)
{
	if (scaleFactor < 0) scaleFactor = 0.5;
   
	//qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();

	// zoom back into feature view, if zooming out above limit
	if (mainWidget->projectLoaded == true)
	{
		if ((wished_zoomed_in == lower_scaling_zoomed_in) && (wished_scale > lower_scaling_limit))
		{
			//mainWidget->mFeatureView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

			//nextWidgetIndexToFadeIn = 0;
			//fadeOut();

			killTimer(timerId);
			timerId = 0;

			// repaint file view preview via paint method of main window
			mainWidget->renderfileview = true;
			mainWidget->repaint();

			// get feature node for current file view
			ogdf::node v = mainWidget->mFeatureView->filezoom_feature_node;

			// calculate properties of lately written preview pixmap
			if (mainWidget->filescenepreview->size().width() > 0)
			{
				mainWidget->mFeatureViewGraphicData[v].graphics->drawfilescenepreview = true;

				qreal pixmap_ratio = (qreal)mainWidget->filescenepreview->width() / (qreal)mainWidget->filescenepreview->height();
				//qDebug() << "fileviewnodegraphic, pixmap ratio = " << pixmap_ratio;

				qreal item_ratio = mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().width() / mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().height();

				if (item_ratio > pixmap_ratio) // item is in proportions wider than pixmap, scale to height of item
				{
					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_y_start = (int)(mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().topLeft().y()) + 1;

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_height =  (int)(mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().height() - 2.0);

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_width = (int)((qreal)mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_height * pixmap_ratio + 0.5);

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_x_start = - mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_width / 2;
				}
				else	// item is in proportions higher than pixmap, scale to width of item
				{

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_x_start = (int)(mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().topLeft().x()) + 1;

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_width = (int)(mainWidget->mFeatureViewGraphicData[v].graphics->boundingRect().width() - 2.0);

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_height = (int)((qreal)mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_width / pixmap_ratio + 0.5 );

					mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_y_start = - mainWidget->mFeatureViewGraphicData[v].graphics->preview_pixmap_height / 2;
				}
			}

			// start zooming into feature view
			mainWidget->mFeatureView->zoomIntoFeatureView();

			// quit file view
			return;
		}
	}

	if (wishedZoomInFile != 0)
	{
		setTransformationAnchor(QGraphicsView::AnchorViewCenter);
		centerOn(wishedZoomInFile);
		QTransform trans;
		trans.scale(scaleFactor, scaleFactor);
		setTransform(trans, true);
		centerOn(wishedZoomInFile);
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

		calculateLevelOfDetail();

		if ((fading_in_active == true) || (fading_out_active == true))
			return;

		if ((wished_zoomed_in == upper_scaling_zoomed_in) && (current_scale > upper_scaling_limit))
		{
			if (wished_scale >= current_scale)
			{
				zoomToCodeView();
			}
		}
	}
}

// get list of all visible node graphics in file view
QList<FileViewNodeGraphic*> FileView::getVisibleNodeGraphics()
{
	// get path of visible area
	QPainterPath path;
	path.addRect(QRectF(this->rect()));
	QPainterPath scenePath;
	scenePath = this->mapToScene(path);

	QList<FileViewNodeGraphic*> visibleItems;

	// check each node graphics item, if it is visible
	for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFileViewGraphicData[v].graphics->collidesWithPath(
			mainWidget->mFileViewGraphicData[v].graphics->mapFromScene(scenePath),
			Qt::IntersectsItemShape))
		{
			visibleItems.push_back(mainWidget->mFileViewGraphicData[v].graphics);
			//qDebug(mainWidget->mFileViewGraphicData[v].graphics->Label().toLatin1());
		}
	}
	return visibleItems;
}

//calculate level of detail
void FileView::calculateLevelOfDetail()
{	
	ogdf::node v = mainWidget-> mFileGraph.firstNode();
	//calculate size of graphics item in screen coordinates
	QPointF itemCoordTopLeft = mainWidget->mFileViewGraphicData[v].graphics->mapToScene(
		mainWidget->mFileViewGraphicData[v].graphics->boundingRect().topLeft() );

	QPointF itemCoordBottomRight = mainWidget->mFileViewGraphicData[v].graphics->mapToScene(
		mainWidget->mFileViewGraphicData[v].graphics->boundingRect().bottomRight() );

	QPoint itemWidgetCoordTopLeft = mapFromScene(itemCoordTopLeft);
	QPoint itemWidgetCoordBottomRight = mapFromScene(itemCoordBottomRight);
		
	QPoint itemScreenCoordTopLeft = mapToGlobal(itemWidgetCoordTopLeft);
	QPoint itemScreenCoordBottomRight = mapToGlobal(itemWidgetCoordBottomRight);

	//debug
	/*
	qDebug("width");
	qDebug(QString::number(itemScreenCoordBottomRight.x()-itemScreenCoordTopLeft.x()).toLatin1());
	qDebug("height");
	qDebug(QString::number(itemScreenCoordBottomRight.y()-itemScreenCoordTopLeft.y()).toLatin1());
	*/

	int itemWidth = itemScreenCoordBottomRight.x() - itemScreenCoordTopLeft.x();

	if (itemWidth <= 20)
	{
		//LOD = 0
		if (LOD != 0)
		{
			LOD = 0;
			for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphics->updateLOD();
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
			for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphics->updateLOD();
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
			for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphics->updateLOD();
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
			for(ogdf::node v =mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
			{
				mainWidget->mFileViewGraphicData[v].graphics->updateLOD();
			}
		}
		//qDebug("LOD = 3");
		return;
	}
}

void FileView::zmButton()
{
	switch (aktivButton)
	{
	case 0:
		mainWidget->fadeInFeatureView();
		break;
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

void FileView::zpButton()
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
		zoomToCodeView();
		break;
	}
}
void FileView::stripe0()
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
	wished_scale = lower_scaling_limit;
	aktivButton=0;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}

void FileView::stripe1()
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
	wished_scale = interval_button1_lower+((interval_button0_lower-interval_button1_lower)/2);
	aktivButton=1;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}

void FileView::stripe2()
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
	wished_scale = interval_button2_lower+((interval_button1_lower-interval_button2_lower)/2);
	aktivButton=2;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
}
void FileView::stripe3()
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
	wished_scale = interval_button3_lower+((interval_button2_lower-interval_button3_lower)/2);
	aktivButton=3;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}

}

void FileView::stripe4()
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
	wished_scale = interval_button4_lower+((interval_button3_lower-interval_button4_lower)/2);
	aktivButton=4;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::stripe5()
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
	wished_scale = interval_button5_lower+((interval_button4_lower-interval_button5_lower)/2);
	aktivButton=5;

	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::stripe6()
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
	wished_scale = interval_button6_lower+((interval_button5_lower-interval_button6_lower)/2);
	aktivButton=6;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}

void FileView::stripe7()
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
	wished_scale = interval_button7_upper-(upper_scaling_interval/2);
	aktivButton=7;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::stripe8()
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
	wished_scale = interval_button8_upper-(upper_scaling_interval/2);
	aktivButton=8;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::stripe9()
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
	wished_scale = interval_button9_upper-(upper_scaling_interval/2);
	aktivButton=9;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::stripe10()
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
	wished_scale = upper_scaling_limit-(upper_scaling_interval/2);
	aktivButton=10;
	// start timer
	if (!timerId)
	{
        timerId = startTimer(1000/25); // timer started
	}
		
}
void FileView::zoomToCodeView()
{
	if (wishedZoomInFile != 0)
	{
		zoomToCodeViewFile(wishedZoomInFile);
		wishedZoomInFile = 0;
		return;
	}

	QList<FileViewNodeGraphic*> visGraphics = getVisibleNodeGraphics();
	if (visGraphics.size() == 1)
	{
		zoomToCodeViewFile(visGraphics.value(0));
	}
}

void FileView::zoomToCodeViewFile(FileViewNodeGraphic *graphic)
{

	centerOn(graphic->scenePos());
	update(rect());
	mainWidget->mFileMiniMap->updateContent();

	// get file name
	ogdf::node v = graphic->getFileGraphNode();
	if (graphic->Type() == 1)
	{
		// get file path
		QString filePath;
		filePath.clear();
		filePath.append(mainWidget->mFileGA.labelNode(v).cstr());

		QString statusmsg;
		statusmsg.append("Zooming into file ");
		statusmsg.append(filePath);
		statusmsg.append("...");
		mainWidget->statusBar()->showMessage(tr(statusmsg.toLatin1()),10000);

		ogdf::node parent = ogdf::node(v);
		ogdf::node root = mainWidget->mFileGraph.firstNode();
		do
		{
			for(ogdf::adjEntry adj = parent->firstAdj();adj; adj = adj->succ())
			{
				ogdf::edge e = adj->theEdge();
				if (e->target() == parent) // target is this node
				{
					parent = e->source();
					filePath.insert(0,"//");
					if (parent != root)
						filePath.insert(0,QString(mainWidget->mFileGA.labelNode(parent).cstr()));
					break;
				}
			}
		} while (parent != root);
		
		// complete file path
		filePath.insert(0,mainWidget->getProjectPath());

		//qDebug("try to open file");
		//qDebug(filePath.toLatin1());

		// load file into code view
		QFile file(filePath);
		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			
			// remove old text
			QTextCursor curs = mainWidget->editor->textCursor();
			curs.setPosition(0,QTextCursor::MoveAnchor);
			curs.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
			curs.removeSelectedText();

			// load file content into editor
			mainWidget->editor->setPlainText(file.readAll());
			mainWidget->editor->resetBackground();

			curs = mainWidget->editor->textCursor();
			curs.setPosition(0,QTextCursor::MoveAnchor);
			mainWidget->editor->setTextCursor(curs);

			// set graphic and create fragment rectangles
			mainWidget->editor->setCurrentGraphic(graphic);
			
			// scale view back a little
			this->stripe9();

			// open code view
			mainWidget->statusBar()->showMessage(tr("Opening Code View..."));
			//mainWidget->codeView();
			nextWidgetIndexToFadeIn = 2;
			fadeOut();
/*
			// calculate fragment rectangles
			mainWidget->statusBar()->showMessage(tr("Calculating Fragment Rectangles..."));
			mainWidget->editor->createFragmentRects();
			mainWidget->editor->fadeOutFragments();
*/


		}
		else {
			// scale view back a little
			this->stripe9();

			statusmsg.clear();
			statusmsg.append("ERROR! File ");
			statusmsg.append(mainWidget->mFileGA.labelNode(v).cstr());
			statusmsg.append("'' not found!");
			mainWidget->statusBar()->showMessage(tr(statusmsg.toLatin1()));

			QMessageBox msgBox;
			msgBox.setText(statusmsg);
			msgBox.exec();
		}
	}
	else
	{
		this->stripe9();
		ogdf::node v = graphic->getFileGraphNode();

		QMessageBox msgBox;
		QString msgText;
		msgText.append(mainWidget->mFileGA.labelNode(v).cstr());
		msgText.append(" is no file. Only files can be opened!");
		msgBox.setText(msgText);
		msgBox.exec();
	}

	zoomIntoFileGraphic = graphic;
}

void FileView::setScrollBarValue(qreal x_val, qreal y_val)
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
void FileView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);
	mainWidget->mFileMiniMap->updateContent();
	calculateScalingLimits();
	stripe0();
}
void FileView::fadeOut()
{
	killTimer(timerId);
	timerId = 0;

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

void FileView::fadeIn()
{
	killTimer(timerId);
	timerId = 0;

	calculateScalingLimits();
	updateButtons();

	// scale views
	if (fading_in_from_featureview == true)
	{
		/*
		QTransform trans;
		trans.scale(lower_scaling_limit/10.0, lower_scaling_limit/10.0);
		setTransform(trans, true);
		*/

		//current_scale =  viewportTransform().m11();
		//wished_scale = current_scale;
		showFullScene();
		//updateButtons();
		//stripe0();
	}

	if (fading_in_from_codeview == true)
	{
		/*
		wished_scale = upper_scaling_limit / 2;
		wished_zoomed_in = upper_scaling_zoomed_in;
		

		// start timer
		if (!timerId)
		{
			timerId = startTimer(1000/25); // timer started
		}
		*/
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

void FileView::setForegroundTransparent()
{
	currentAlpha = 0;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	QBrush b(semiTransparentColor);
	setForegroundBrush(b);
	repaint();
}
void FileView::setForegroundOpaque()
{
	currentAlpha = 255;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	QBrush b(semiTransparentColor);
	setForegroundBrush(b);
	repaint();
}
void FileView::updateForegroundBrush()
{
	//qDebug() << "event timer " << event->timerId();
	//qDebug() << "(updateForegroundBrush) timer " << mTimer->timerId();
	//qDebug() << "update foreground brush";
	//qDebug() << "interval = " << currentinterval;
	
	//qDebug() << "repaint at " << mTimer->interval();
	//painter->fillRect(mScene->sceneRect(), QBrush(semiTransparentColor));
	if (fading_in_active == true)
	{
		currentAlpha -= 255 * currentinterval / duration;
		//qDebug() << "fading in " << currentAlpha;
		if (currentAlpha <= 0) {
			
			fading_in_active = false;
			fading_in_from_codeview = false;
			fading_in_from_featureview = false;

			currentAlpha = 0;
			killTimer(mTimerID);
			mTimerID = 0;
		}
	}
	else if (fading_out_active == true)
	{
		currentAlpha += 255 * currentinterval / duration;
		//qDebug() << "fading out " << currentAlpha;
		if (currentAlpha >= 255) {

			fading_out_active = false;

			currentAlpha = 255;
			killTimer(mTimerID);
			mTimerID = 0;


			switch(nextWidgetIndexToFadeIn)
			{
			case 0:
				//qDebug() << "file view, current scale = " << current_scale;
				nextWidgetIndexToFadeIn = -1;
				mainWidget->fadeInFeatureView();
				break;
			case 2:
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
	//mScene->update(mScene->sceneRect());
	//update();
	repaint();
}
void FileView::setNextWidgetToFadeIn(int widget_index)
{
	nextWidgetIndexToFadeIn = widget_index;
}

void FileView::updateButtons()
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
void FileView::calculateScalingLimits()
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
	qreal smallest_width = mainWidget->mFileViewGraphicData[mainWidget-> mFileGraph.firstNode()].graphics->sceneBoundingRect().width();
	qreal smallest_height = mainWidget->mFileViewGraphicData[mainWidget-> mFileGraph.firstNode()].graphics->sceneBoundingRect().height();

	for(ogdf::node v = mainWidget-> mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mainWidget->mFileViewGraphicData[v].graphics->sceneBoundingRect().width() < smallest_width)
			smallest_width = mainWidget->mFileViewGraphicData[v].graphics->sceneBoundingRect().width();
		if (mainWidget->mFileViewGraphicData[v].graphics->sceneBoundingRect().height() < smallest_height)
			smallest_height = mainWidget->mFileViewGraphicData[v].graphics->sceneBoundingRect().height();
	}

	qreal item_factor_w = smallest_width/qreal(this->geometry().width());
	qreal item_factor_h = smallest_height/qreal(this->geometry().height());
	qreal item_factor;
	if (item_factor_h < item_factor_w)
		item_factor = item_factor_h;
	else item_factor = item_factor_w;

	upper_scaling_limit = 1/item_factor;
	upper_scaling_zoomed_in = true;

	lower_scaling_interval = (lower_scaling_limit - 1.0) / 6.0;	// 6 steps for zooming out
	upper_scaling_interval = (upper_scaling_limit - 1.0) / 4.0;	// 4 steps for zooming in

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