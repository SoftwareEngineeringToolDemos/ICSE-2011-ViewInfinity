

#include "SMVMain.h"
#include "FileView.h"
#include "CodeView.h"
#include "CodeViewInteractiveRect.h"
#include "FileViewNodeGraphic.h"
#include "CodeMiniMap.h"

const qreal MARGIN_BORDER_FRAGMENTS			= 5.0;
const qreal MARGIN_FRAGMENTS				= 3.0;
const qreal MIN_WIDTH_FRAGMENTS				= 3.0;
const qreal MAX_WIDTH_FRAGMENTS				= 5.0;
const qreal WISHED_WIDTH_FRAGMENTS_COLUMN	= 50.0;
const qreal MARGIN_FRAGMENTS_LINENUMBERS	= 5.0;
const qreal MARGIN_LINENUMBERS_CODE			= 10.0;

qreal real_width_fragments_column		= 0.0;

CodeView::CodeView(SMVMain *graphWidget)
	: timerId(0)
{
	mainWidget = graphWidget;

	if (mainWidget)
        startColor = mainWidget->palette().window().color();
    else
        startColor = Qt::white;

	currentAlpha = 0;
	duration = 1000;
	mTimerID = 0;
	timeinterval = 50;
	currentinterval = 0;

	fading_in_active = false;
	fading_out_active = false;
	nextWidgetIndexToFadeIn = -1;

	overlay = new Overlay(this);
	overlay->setEditor(this);
	overlay->setAutoFillBackground(false);
	overlay->show();
	overlay->raise();

	this->setWordWrapMode(QTextOption::NoWrap);
	
	featureArea = new FeatureArea(this);

    //connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateFeatureAreaWidth(int)));
	//connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateFeatureArea(const QRect &, int)));

	m_featureAreaWidth = 0;
	m_lineNumberWidth = 0;
	feat_number = 0;

	colorTransparency = 255;
}

CodeView::~CodeView(void)
{
}
int CodeView::featureAreaWidth()
{
	int digits = 1;
	int max = qMax(1, document()->blockCount());
	while (max >= 10) {
		 max /= 10;
		 ++digits;
	}
	m_lineNumberWidth = fontMetrics().width(QLatin1Char('9')) * digits;
	int space = MARGIN_BORDER_FRAGMENTS + real_width_fragments_column + MARGIN_FRAGMENTS_LINENUMBERS +
		m_lineNumberWidth + MARGIN_LINENUMBERS_CODE;
	return space;
}
void CodeView::updateFeatureAreaWidth(int /* newBlockCount */)
{
	m_featureAreaWidth = featureAreaWidth();
    setViewportMargins(m_featureAreaWidth, 0, 0, 0);
}
void CodeView::featureAreaPaintEvent(QPaintEvent *event)
{

    QPainter painter(featureArea);

	// get first visible block
	QPoint p(0,0);
	QTextCursor curs_pos = cursorForPosition(p);
	QTextBlock first_block = document()->findBlock(curs_pos.position());
	int first_loc = first_block.blockNumber();

	// get last visible block
	p.setY(rect().height());
	curs_pos = cursorForPosition(p);
    QTextBlock last_block = document()->findBlock(curs_pos.position());
	int last_loc = last_block.blockNumber();

	// draw feature fragments
	QPointF contentOffset(0.0, -this->verticalScrollBar()->value());
	for ( int i = 0 ; i < feat_y_top.size() ; i++ )
	{
		painter.fillRect(
			QRect(
			QPoint(feat_x_left[i], feat_y_top[i]),
			QPoint(feat_x_right[i], feat_y_bottom[i] )
			).translated(contentOffset.toPoint()),
			QBrush(feat_color.at(i)));
	}

	first_block = document()->findBlockByNumber(first_loc);
	QTextBlock block = first_block;

	// draw line numbers
	int top = (int) document()->documentLayout()->blockBoundingRect(block).translated(contentOffset).top();
	int bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();
	for ( block = first_block; block != last_block; block = block.next())
	{
		QString number = QString::number(block.blockNumber() + 1);
		painter.setPen(Qt::gray);
		painter.drawText(m_featureAreaWidth - m_lineNumberWidth - MARGIN_LINENUMBERS_CODE, top, m_lineNumberWidth, fontMetrics().height(),
                             Qt::AlignRight, number);
		top = bottom;
        bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();
	}
	// draw last line
	painter.setPen(Qt::gray);
	painter.drawText(m_featureAreaWidth - m_lineNumberWidth - MARGIN_LINENUMBERS_CODE, top, m_lineNumberWidth, fontMetrics().height(),
                             Qt::AlignRight, QString::number(block.blockNumber() + 1));

	// draw limiting line between line numbers and code
	painter.drawLine(m_featureAreaWidth - (MARGIN_LINENUMBERS_CODE / 2), (int) document()->documentLayout()->blockBoundingRect(first_block).translated(contentOffset).top(),
		m_featureAreaWidth - (MARGIN_LINENUMBERS_CODE / 2), (int) document()->documentLayout()->blockBoundingRect(last_block).translated(contentOffset).top()+
		(int) document()->documentLayout()->blockBoundingRect(last_block).height());

	// draw upper and lower white backgrounds
	QRectF white_rect(QPointF(0.0,0.0), QSizeF(m_featureAreaWidth - m_lineNumberWidth - MARGIN_LINENUMBERS_CODE, 20));
	painter.fillRect(white_rect, QBrush(Qt::white));
	white_rect.translate(0.0, geometry().height() - 20);
	painter.fillRect(white_rect, QBrush(Qt::white));

	qreal x = MARGIN_BORDER_FRAGMENTS;
	// draw feature shortcuts
	for ( int i = feature_names.size()-1; i >= 0; i-- )
	{
		painter.fillRect(x, 0.0, interval, 10.0, QBrush(feature_colors.at(i)));
		painter.fillRect(x, geometry().height() - 10, interval, 10.0,QBrush(feature_colors.at(i)));

		if (feature_is_painted.at(i) == true)
		{
			painter.fillRect(x, 12.0, interval, 5.0,QBrush(Qt::red));
			painter.fillRect(x, geometry().height() - 17, interval, 5.0,QBrush(Qt::red));
		}
		x += MARGIN_FRAGMENTS + interval;
	}

	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);
	painter.fillRect(rect(), semiTransparentColor);
}
void CodeView::updateFeatureArea(const QRect &rect, int dy)
{
    if (dy)
		featureArea->scroll(0, dy);
    else
        featureArea->update(0, rect.y(), m_featureAreaWidth, rect.height());
     if (rect.contains(viewport()->rect()))
        updateFeatureAreaWidth(0);
}
void CodeView::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() == Qt::NoModifier)
	{
		QTextEdit::wheelEvent(event);
	}
	else
	{
		if (event->delta()<0)
		{
			// zoom back to file view
			mainWidget->mFileView->stripe9();
			mainWidget->mFileView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
			nextWidgetIndexToFadeIn = 1;
			fadeOut();
		}
	}
}

void CodeView::createMiniMapPixMap()
{
	// Prepare a pixmap large enough to draw the item into
	QTextDocument *doc = this->document();
	QSizeF s = doc->size();
	QPixmap pm(s.toSize());
	QPainter painter(&pm);
	painter.fillRect(pm.rect(),QBrush(Qt::white));
	doc->drawContents(&painter);
	mainWidget->mCodeMiniMap->setBackground(&pm);
}

void CodeView::scrollContentsBy(int dx, int dy)
{
	QTextEdit::scrollContentsBy(dx,dy);
	featureArea->update();
	if (mainWidget->mCodeMiniMap != 0)
	{
		if (mainWidget->mCodeMiniMap->m_intrect->rect_active == false)
		{
			if (this->verticalScrollBar()->maximum() > this->verticalScrollBar()->minimum())
			{
				qreal correctedvalue = qreal(this->verticalScrollBar()->value())-qreal(this->verticalScrollBar()->minimum());
				qreal interval = qreal(this->verticalScrollBar()->maximum()) - qreal(this->verticalScrollBar()->minimum());
				qreal val =  correctedvalue / interval;
				if (val < 0.0)
					val = -val;
				mainWidget->mCodeMiniMap->setScrollBarValue(val);
			}
			else if (this->verticalScrollBar()->minimum() > this->verticalScrollBar()->maximum())
			{
				qreal correctedvalue = qreal(this->verticalScrollBar()->value())-qreal(this->verticalScrollBar()->maximum());
				qreal interval = qreal(this->verticalScrollBar()->minimum()) - qreal(this->verticalScrollBar()->maximum());
				qreal val =  correctedvalue / interval;
				if (val < 0.0)
					val = -val;
				mainWidget->mCodeMiniMap->setScrollBarValue(val);
			}
		}
		else //event comes from minimap rectangle
		{
			return;
		}
	}
	mainWidget->mCodeMiniMap->updateContent();
}
void CodeView::setScrollBarValue(qreal val)
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
void CodeView::resizeEvent(QResizeEvent *event)
{
	QTextEdit::resizeEvent(event);
	
	QRect cr = contentsRect();
    featureArea->setGeometry(QRect(cr.left(), cr.top(), featureAreaWidth(), cr.height()));

	overlay->resize(QSize(contentsRect().width(),contentsRect().height()));
	
	mainWidget->mCodeMiniMap->updateContent();
}

void CodeView::scrollText(int dx, int dy)
{
	scrollContentsBy(dx,dy);
}

void CodeView::setCurrentGraphic(FileViewNodeGraphic *graphic)
{
	m_FileGraphic = graphic;
}

void CodeView::createFragmentRects()
{
	// clear old data

	// fragment vectors
	feat_line_start.clear();
	feat_line_end.clear();
	feat_x_left.clear();
	feat_x_right.clear();
	feat_y_top.clear();
	feat_y_bottom.clear();
	feat_column.clear();
	feat_name.clear();
	feat_color.clear();
	feat_is_active.clear();

	// feature vectors
	feature_names.clear();
	feature_colors.clear();
	feature_x_left.clear();
	feature_x_right.clear();
	feature_is_painted.clear();
	feat_number = 0;

	// break if there are no fragments
	if (!(m_FileGraphic->offset.size() > 0))
		return;

	QTextCursor curs;

	// get fragments data
	QVector<int> temp_start;
	QVector<int> temp_end;
	QVector<QColor> temp_color;
	QVector<QString> temp_name;
	QVector<bool> temp_is_active;

	for(unsigned int i = 0; i < m_FileGraphic->offset.size(); i++)
	{
		for (unsigned int j = 0; j < m_FileGraphic->fragmentNames[i].size(); j++)
		{
			temp_start.push_back(m_FileGraphic->offset[i]);
			temp_end.push_back(m_FileGraphic->offset[i]+m_FileGraphic->length[i]);
			temp_name.push_back(m_FileGraphic->fragmentNames[i][j]);			// save name
			temp_is_active.push_back(m_FileGraphic->isActive[i][j]);			// save activation status
			if (m_FileGraphic->isActive[i][j])
			{
				temp_color.push_back(m_FileGraphic->fragmentColors[i][j]);		// save color if active
			}
			else
			{
				//qDebug() << m_FileGraphic->fragmentNames[i][j] << " is inactive";
				temp_color.push_back(QColor(Qt::gray));
			}
		}
	}

	// combine fragments
	bool combined;
	int start, end;
	QString name;
	while (temp_start.size() > 0)
	{
		combined = false;
		name = temp_name.at(0);
		start = temp_start.at(0);
		end = temp_end.at(0);

		int index = 1;
		while (index < temp_start.size()-1)
		{
						
			if (temp_start.at(index) > end)	// we can break here, because we moved over ending position
				break;

			if (QString::compare(name, temp_name.at(index)) == 0)
			{
				if (end == temp_start.at(index))
				{
					end = temp_end.at(index);
					combined = true;
					
					// delete this fragment
					temp_start.remove(index);
					temp_end.remove(index);
					temp_name.remove(index);
					temp_color.remove(index);
					temp_is_active.remove(index);

					index--;
				}
			}
			index++;
		}
		
		// add first fragment to list		

		// find y start
		curs = textCursor();
		curs.setPosition(start);
		feat_line_start.push_back(document()->findBlock(curs.position()).blockNumber());

		// find y end
		curs = textCursor();
		curs.setPosition(end,QTextCursor::MoveAnchor);
		feat_line_end.push_back(document()->findBlock(curs.position()).blockNumber());

		feat_name.push_back(name);								// save name
		feat_is_active.push_back(temp_is_active.at(0));			// save activation status

		feat_color.push_back(temp_color.at(0));
/*
		if (temp_is_active.at(0))
		{
			feat_color.push_back(temp_color.at(0));				// save color if active
		}
		else
		{
			feat_color.push_back(QColor(Qt::gray));
		}
*/
		feat_column.push_back(0);											// init with standard column
		feat_y_top.push_back(0.0);											// init with standard coordinate
		feat_y_bottom.push_back(0.0);										// init with standard coordinate
		feat_x_left.push_back(0.0);											// init with standard coordinate
		feat_x_right.push_back(0.0);										// init with standard coordinate

		// delete this fragment
		temp_start.remove(0);
		temp_end.remove(0);
		temp_name.remove(0);
		temp_color.remove(0);
		temp_is_active.remove(0);
	}

	// calculate fragment columns
	for ( int i = 0; i < feat_y_top.size(); i++ )
	{
		// only handle active features now
		if (feat_is_active.at(i) == false)									// feature not active?
			continue;														// then continue with next fragment

		if (feature_names.contains(feat_name.at(i)))						// feature name already existing
		{
			feat_column[i] = feature_names.indexOf(feat_name.at(i)) + 1;	// save the same column
			continue;														// continue with next fragment
		}

		// else: feature not yet in vector
		feature_names.push_back(feat_name.at(i));						// add feature name to vector
		feature_colors.push_back(feat_color.at(i));

		feat_column[i] = feature_names.size();							// save new column
	}

	// handle deactivated features now
	for ( int i = 0; i < feat_y_top.size(); i++ )
	{
		// only handle deactivated features now
		if (feat_is_active.at(i) == true)										// feature active?
			continue;															// then continue with next fragment

		if (feature_names.contains(feat_name.at(i)))						// feature name already existing
		{
			feat_column[i] = feature_names.indexOf(feat_name.at(i)) + 1;	// save the same column
			continue;															// continue with next fragment
		}

		// else: feature not yet in vector
		feature_names.push_back(feat_name.at(i));						// add feature name to vector

		// todo: check, if the color has to be added
		feature_colors.push_back(feat_color.at(i));

		feat_column[i] = feature_names.size();							// save new column
	}
	
	feat_number = feature_names.size();									// save number of different features to draw

	for ( int i = 0; i < feature_names.size(); i++ )					// init vector, no feature is painted
		feature_is_painted.push_back(false);

	// calculate coordinates for drawing
	interval = qMax( MIN_WIDTH_FRAGMENTS , ( ( WISHED_WIDTH_FRAGMENTS_COLUMN - (MARGIN_FRAGMENTS * feat_number ) ) / WISHED_WIDTH_FRAGMENTS_COLUMN ) );
	interval = qMax(MAX_WIDTH_FRAGMENTS, interval);
	real_width_fragments_column = (interval + MARGIN_FRAGMENTS ) * feat_number;
	for ( int i = 0; i < feat_y_top.size(); i++ )
	{
		// calculate x-coordinates
		feat_x_left[i] = ( MARGIN_BORDER_FRAGMENTS + (feat_number - feat_column.at(i)) * (interval + MARGIN_FRAGMENTS) );
		feat_x_right[i] = ( MARGIN_BORDER_FRAGMENTS + (feat_number - feat_column.at(i)) * (interval + MARGIN_FRAGMENTS) + interval );

		// calculate y-coordinates
		QTextBlock block = document()->findBlockByLineNumber(feat_line_start[i]);
		feat_y_top[i] = ( document()->documentLayout()->blockBoundingRect(block).top() );
		block = document()->findBlockByLineNumber(feat_line_end[i]);
		feat_y_bottom[i] = ( document()->documentLayout()->blockBoundingRect(block).bottom() );
	}

	updateFeatureAreaWidth(0);

	// sort fragments
	bubbleSortFragmentPositions();
}

void CodeView::getMousePressEventFromMainWindow(QPoint globpos)
{
	QPoint widget_coord = this->mapFromGlobal(globpos);

	// check if on upper or lower border clicked
	qreal x = MARGIN_BORDER_FRAGMENTS;
	for ( int i = feature_names.size()-1; i >= 0; i-- )
	{
		QRect upper_rect(x, 0.0, interval, 10.0);
		QRect lower_rect(x, geometry().height() - 10, interval, 10.0);
		if ( (upper_rect.contains(widget_coord)) || (lower_rect.contains(widget_coord)) )
		{
			if (feature_is_painted[i] == false)
			{
				feature_is_painted[i] = true;
			}
			else
			{
				feature_is_painted[i] = false;
				resetBackground();
			}
			
			updateColors();
			featureArea->update();
			return;
		}
		x += MARGIN_FRAGMENTS + interval;
	}

	// check if on fragment clicked
	QPointF contentOffset(0.0, -this->verticalScrollBar()->value());
	for ( int i = 0 ; i < feat_y_top.size() ; i++ )
	{
		QRect frag_rect(QRect(QPoint(feat_x_left[i], feat_y_top[i]),
			QPoint(feat_x_right[i], feat_y_bottom[i])
			).translated(contentOffset.toPoint()));
		if (frag_rect.contains(widget_coord))
		{
			int idx = feature_names.indexOf(feat_name[i]);
			if (feature_is_painted[idx] == false)
			{
				feature_is_painted[idx] = true;
			}
			else
			{
				feature_is_painted[idx] = false;
				resetBackground();
			}
			updateColors();
			featureArea->update();
			break;
		}
	}
}
void CodeView::getWheelEventFromMainWindow(QWheelEvent *event)
{
	wheelEvent(event);
}

void CodeView::resetBackground()
{
	// save old scroll pos
	int vert_v = verticalScrollBar()->value();
	int hori_v = horizontalScrollBar()->value();
	QTextCursor curs_start = textCursor();

	QTextCharFormat format;
	// clear old background color
	QTextBlock block = document()->findBlockByLineNumber(0);
	int first_char = block.position();
	block = document()->findBlockByLineNumber(document()->blockCount()-1);
	int last_char = block.position()+block.length()-1;
	QTextCursor curs = textCursor();
	curs.setPosition(first_char);
	curs.setPosition(last_char,QTextCursor::KeepAnchor);
	format.setBackground(QBrush());
	format.setToolTip("no selected feature");
	curs.setCharFormat(format);
	setTextCursor(curs);

	setTextCursor(curs_start);
	verticalScrollBar()->setValue(vert_v);
	horizontalScrollBar()->setValue(hori_v);
}

void CodeView::setColorTransparency(int value)
{
	colorTransparency = value;
	updateColors();
}

void CodeView::updateColors()
{
	drawFragments();
	createMiniMapPixMap();
}

void CodeView::drawFragments()
{
	// save old scroll pos
	int vert_v = verticalScrollBar()->value();
	int hori_v = horizontalScrollBar()->value();

	QTextCursor curs_start = textCursor();

	QTextCharFormat format;
	QTextBlock block;
	QTextCursor curs;
	int first_char, last_char;

	for ( int i = 0; i < feat_y_top.size(); i++ )
	{
		if (feature_is_painted.at(feature_names.indexOf(feat_name[i])) == true)
		{
			QString strToolTip;
			strToolTip.append("Feature:\n");
			QString str(feat_name[i]);
			strToolTip.append("\n");
			strToolTip.append(str);

			block = document()->findBlockByLineNumber(feat_line_start[i]);
			first_char = block.position();
			block = document()->findBlockByLineNumber(feat_line_end[i]);
			last_char = block.position()+block.length()-1;
			curs = textCursor();
			curs.setPosition(first_char);
			curs.setPosition(last_char,QTextCursor::KeepAnchor);
			QColor col(feat_color[i]);
			
			if (this->feat_is_active[i] == false)		// paint gray bar, if feature is not active
			{
				col = Qt::gray;
			}
			/*
			// debug
			else
			{
				qDebug() << feat_name[i] << " is not gray";
			}
			*/

			col.setAlpha(colorTransparency);
			format.setBackground(QBrush(col,Qt::SolidPattern));
			format.setToolTip(strToolTip);
			curs.setCharFormat(format);
			setTextCursor(curs);
		}
	}

	setTextCursor(curs_start);
	verticalScrollBar()->setValue(vert_v);
	horizontalScrollBar()->setValue(hori_v);
}

// sorting in descending order of fragment lengths
void CodeView::bubbleSortFragmentPositions()
{
	bool switched;
	do
	{
		switched = false;
		for (int index = 0; index < feat_line_start.size()-1; index++)
		{
			if ((feat_line_end.at(index) - feat_line_start.at(index)) < (feat_line_end.at(index+1) - feat_line_start.at(index+1)))	// if fragment a is longer than b
			{
				switchFragmentPositions(index, index + 1);
				switched = true;
			}
		}
	}
	while (switched == true);
}

void CodeView::switchFragmentPositions(int idx1, int idx2)
{
	//qDebug() << "switch : " << idx1 << "/" << idx2;
	
	// switch upper y
	qreal temp_y_top(feat_y_top.at(idx1));
	feat_y_top[idx1] = feat_y_top.at(idx2);
	feat_y_top[idx2] = temp_y_top;

	// switch lower y
	qreal temp_y_bottom(feat_y_bottom.at(idx1));
	feat_y_bottom[idx1] = feat_y_bottom.at(idx2);
	feat_y_bottom[idx2] = temp_y_bottom;

	// switch line start
	qreal temp_line_start(feat_line_start.at(idx1));
	feat_line_start[idx1] = feat_line_start.at(idx2);
	feat_line_start[idx2] = temp_line_start;

	// switch line end
	qreal temp_line_end(feat_line_end.at(idx1));
	feat_line_end[idx1] = feat_line_end.at(idx2);
	feat_line_end[idx2] = temp_line_end;

	// switch x left
	qreal temp_x_left(feat_x_left.at(idx1));
	feat_x_left[idx1] = feat_x_left.at(idx2);
	feat_x_left[idx2] = temp_x_left;

	// switch x right
	qreal temp_x_right(feat_x_right.at(idx1));
	feat_x_right[idx1] = feat_x_right.at(idx2);
	feat_x_right[idx2] = temp_x_right;

	// switch column
	int temp_column(feat_column.at(idx1));
	feat_column[idx1] = feat_column.at(idx2);
	feat_column[idx2] = temp_column;

	// switch name
	QString temp_name(feat_name.at(idx1));
	feat_name[idx1] = feat_name.at(idx2);
	feat_name[idx2] = temp_name;

	// switch color
	QColor temp_color(feat_color.at(idx1));
	feat_color[idx1] = feat_color.at(idx2);
	feat_color[idx2] = temp_color;

	// switch is_active
	bool temp_is_active(feat_is_active.at(idx1));
	feat_is_active[idx1] = feat_is_active.at(idx2);
	feat_is_active[idx2] = temp_is_active;
}

void CodeView::fadeOutFragments()
{
	// activate all fragments
	for (int i = 0; i < feature_is_painted.size(); i++)
		feature_is_painted[i] =  true;

	colorTransparency = 255;
	drawFragments();

	// start timer
	if (!timerId)
	{
        timerId = startTimer(150); // timer started
	}
}
// Qt timer event
void CodeView::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

	if (event->timerId() == mTimerID)
	{
		currentinterval += timeinterval;
		updateForegroundBrush();
	}

	if (event->timerId() != timerId)
		return;

	colorTransparency -= 30;
	if (colorTransparency < 0)
		colorTransparency = 0;
	
	if (colorTransparency == 0)
	{
		killTimer(timerId);
		timerId = 0;
			
		drawFragments();
		colorTransparency = 128;

		// deactivate all fragments
		for (int i = 0; i < feature_is_painted.size(); i++)
		{
			feature_is_painted[i] =  false;
		}
		featureArea->update();
	}
	else
	{
		drawFragments();
	}
}
QPixmap CodeView::getDocumentPixmap()
{
	// Prepare a pixmap large enough to draw the item into
	QTextDocument *doc = this->document();
	QSizeF s = doc->size();
	QPixmap pm(s.toSize());
	QPainter painter(&pm);
	painter.fillRect(pm.rect(),QBrush(Qt::white));
	doc->drawContents(&painter);
	return pm;
}

void CodeView::fadeOut()
{
	//overlay->resize(geometry().size());

	currentAlpha = 0;
	fading_out_active = true;
	if (fading_in_active == true) fading_in_active = false;
	//qDebug() << "start fading out";
    //mTimer->start(33);
	//mTimerID = startTimer(33);

	overlay->setEnabled(true);
	overlay->show();

	featureArea->repaint();
	overlay->repaint();

	// start timer
	if (!mTimerID)
	{
        mTimerID = startTimer(timeinterval); // timer started
		currentinterval = 0;
		//qDebug() << "mTimerID created = " << mTimerID;
	}
}

void CodeView::fadeIn()
{
/*
	qDebug() << "size" << this->size();
	qDebug() << "rect" << this->rect();
	qDebug() << "geometry size" << this->geometry().size();
	qDebug() << "viewport rect" << this->viewport()->rect();
	qDebug() << "viewport size" << this->viewport()->size();
	qDebug() << "document size" << document()->size().toSize();
	qDebug() << "contentsrect" << this->contentsRect();
*/

	overlay->resize(geometry().size());

	currentAlpha = 255;
	fading_in_active = true;
	if (fading_out_active == true) fading_out_active = false;
	//qDebug() << "start fading out";

	overlay->setEnabled(true);
	overlay->show();

	featureArea->repaint();
	overlay->repaint();

	if (!mTimerID)
	{
        mTimerID = startTimer(timeinterval); // timer started
		currentinterval = 0;
		//qDebug() << "mTimerID created = " << mTimerID;
	}
}

void CodeView::setForegroundTransparent()
{
	overlay->resize(geometry().size());

	currentAlpha = 0;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);

	overlay->setEnabled(true);
	overlay->show();

	featureArea->repaint();
	overlay->repaint();
}
void CodeView::setForegroundOpaque()
{
	overlay->resize(geometry().size());

	currentAlpha = 255;
	QColor semiTransparentColor = startColor;
	semiTransparentColor.setAlpha(currentAlpha);

	overlay->setEnabled(true);
	overlay->show();

	featureArea->repaint();
	overlay->repaint();
}
void CodeView::updateForegroundBrush()
{
	//overlay->resize(document()->size().toSize());

	if (fading_in_active == true)
	{
		currentAlpha -= 255 * currentinterval / duration;
		if (currentAlpha <= 0) {
			currentAlpha = 0;
			killTimer(mTimerID);
			mTimerID = 0;

			fading_in_active = false;

			overlay->setDisabled(true);
			overlay->hide();

			return;
		}
	}
	else if (fading_out_active == true)
	{
		currentAlpha += 255 * currentinterval / duration;
		if (currentAlpha >= 255) {
			currentAlpha = 255;
			killTimer(mTimerID);
			mTimerID = 0;

			fading_out_active = false;

			switch(nextWidgetIndexToFadeIn)
			{
			case 0:
				nextWidgetIndexToFadeIn = -1;
				mainWidget->fadeInFeatureView();
				break;
			case 1:
				nextWidgetIndexToFadeIn = -1;
				mainWidget->mFileView->fading_in_from_codeview = true;
				mainWidget->fadeInFileView();
				break;
			default:
				nextWidgetIndexToFadeIn = -1;
				fadeIn();
				break;
			}

			return;
		}
	}
	featureArea->repaint();
	overlay->repaint();
	//repaint();
}
void CodeView::setNextWidgetToFadeIn(int widget_index)
{
	nextWidgetIndexToFadeIn = widget_index;
}