

#pragma once

#ifndef FEATURETREEVIEW_H
#define FEATURETREEVIEW_H

#include <QTreeView>

class FeatureTreeView : public QTreeView
{
	Q_OBJECT

public:
	FeatureTreeView(QWidget * parent = 0);
	~FeatureTreeView(void);

public slots:
	// Qt slots

private slots:

private:
	QWidget		*mParent;

protected:
	// protected declarations
    //void keyPressEvent(QKeyEvent *event);
};

#endif
