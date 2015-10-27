

#pragma once

#ifndef CODEMINIMAP_H
#define CODEMINIMAP_H

#include <QApplication>
#include <QtGui>
#include <QDockWidget>

class CodeViewInteractiveRect;
class CodeView;

class CodeMiniMap : public QGraphicsView
{
	Q_OBJECT

public:
	CodeMiniMap(QWidget  * parent = 0);
	~CodeMiniMap(void);
	void setBackground(QPixmap *pixmap);
	void setCodeView(CodeView *view);
	void setScrollBarValue(qreal val);
	void updateContent();

	CodeViewInteractiveRect *m_intrect;
	CodeView				*mCodeView;

protected:
	void drawBackground(QPainter *painter, const QRectF &rect);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);
	void scrollContentsBy(int dx, int dy);

private:
	QWidget					*mParent;
	QGraphicsScene			*mScene;
	QPixmap					*codepixmap;
	float					zoomlevel;

};

#endif