

#pragma once

#ifndef CODEVIEW_H
#define CODEVIEW_H

#include <QTextEdit>
#include <QtGui>
//#include <QObject>
//#include <QGraphicsView>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
//class QGraphicsView;
class FileViewNodeGraphic;
class FeatureArea;
class Overlay;
class SMVMain;

class CodeView : public QTextEdit
{
	Q_OBJECT

public:
	CodeView(SMVMain *graphWidget);
	~CodeView(void);
	void createMiniMapPixMap();
	void setScrollBarValue(qreal val);
	void scrollText(int dx, int dy);

    void featureAreaPaintEvent(QPaintEvent *event);
	int featureAreaWidth();
	void setCurrentGraphic(FileViewNodeGraphic *graphic);
	void createFragmentRects();
	void getMousePressEventFromMainWindow(QPoint globpos);
	void getWheelEventFromMainWindow(QWheelEvent *event);

	void colorFeature(QString feature);
	void setColorTransparency(int value);
	void resetBackground();
	void fadeOutFragments();

	void updateColors();
	void drawFragments();

	void fadeOut();
	void fadeIn();
	void setForegroundTransparent();
	void setForegroundOpaque();
	void setNextWidgetToFadeIn(int widget_index);

	QPixmap getDocumentPixmap();

	// fragment data
	QVector<qreal>			feat_line_start;
	QVector<qreal>			feat_line_end;
	QVector<qreal>			feat_y_top;
	QVector<qreal>			feat_y_bottom;
	QVector<qreal>			feat_x_left;
	QVector<qreal>			feat_x_right;
	QVector<int>			feat_column;
	QVector<QString>		feat_name;
	QVector<QColor>			feat_color;
	QVector<bool>			feat_is_active;

	int						feat_number;
	QVector<QString>		feature_names;
	QVector<QColor>			feature_colors;
	QVector<qreal>			feature_x_left;
	QVector<qreal>			feature_x_right;
	QVector<bool>			feature_is_painted;


	qreal					interval;

	int						colorTransparency;
	FileViewNodeGraphic		*m_FileGraphic;

	Overlay					*overlay;

	int						mTimerID;
	int						timeinterval;
	int						currentinterval;
	QColor					startColor;
	int						currentAlpha;
	int						duration;

	bool					fading_in_active;
	bool					fading_out_active;

protected:
	void wheelEvent(QWheelEvent *event);
	void scrollContentsBy(int dx, int dy);
	void resizeEvent(QResizeEvent *event);
	void timerEvent(QTimerEvent *event);

 private slots:
     void updateFeatureAreaWidth(int newBlockCount);
     void updateFeatureArea(const QRect &, int);

private:
	void bubbleSortFragmentPositions();
	void switchFragmentPositions(int idx1, int idx2);
	void updateForegroundBrush();

	SMVMain					*mainWidget;
    QWidget					*featureArea;

	int						m_featureAreaWidth;
	int						m_lineNumberWidth;

	int						timerId;

	int					nextWidgetIndexToFadeIn;



};

 class FeatureArea : public QWidget
 {
 public:
     FeatureArea(CodeView *editor) : QWidget(editor) {
         codeEditor = editor;
     }

	 
     QSize sizeHint() const {
         return QSize(codeEditor->featureAreaWidth(), 0);
     }
protected:

    void paintEvent(QPaintEvent *event) {
		codeEditor->featureAreaPaintEvent(event);
     }

 private:
     CodeView *codeEditor;
 };

 class Overlay : public QWidget
 {
 public:
     //Overlay(CodeView *editor) : QWidget(editor) {
	 Overlay(QWidget *parent)  : QWidget(parent) {
         editorViewport = parent;
		 resize(parent->geometry().size());
     }

	 void setEditor(CodeView *editor)
	 {
		codeEditor = editor;
	 }
/*
     QSize sizeHint() const {
		 return codeEditor->viewport()->size();
     }
*/
 protected:
     void paintEvent(QPaintEvent *event) {
		
		QPainter painter(this);
		QColor semiTransparentColor = codeEditor->startColor;
		semiTransparentColor.setAlpha(codeEditor->currentAlpha);
		painter.fillRect(rect(), semiTransparentColor);
		
     }

 private:
	QWidget *editorViewport;
	CodeView *codeEditor;

 };


#endif