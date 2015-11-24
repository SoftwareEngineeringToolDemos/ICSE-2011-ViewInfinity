

#pragma once

#ifndef FILETREEVIEW_H
#define FILETREEVIEW_H

#include <QTreeView>

class FileTreeView : public QTreeView
{
	Q_OBJECT

public:
	FileTreeView(QWidget * parent = 0);
	~FileTreeView(void);

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