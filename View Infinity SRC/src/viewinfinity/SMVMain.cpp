

#include <QDebug>
#include <QtGui>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QPushButton>
#include <QTextEdit>
#include <QString>
#include <QMessageBox>
#include <QFile>
#include <QToolTip>

#include <iostream>
#include <fstream>
#include <sstream>

#include <ogdf/misclayout/CircularLayout.h>					// circular layout
#include <ogdf/layered/FastHierarchyLayout.h>				// fast hierarchical layout
#include <ogdf/misclayout/BalloonLayout.h>					// balloon layout
#include <ogdf/tree/TreeLayout.h>							// tree layout
#include <ogdf/layered/SugiyamaLayout.h>					// hierarchical layout
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/energybased/FMMMLayout.h>					// energy-based layout
#include <ogdf/planarity/PlanarizationLayout.h>				// orthogonal layout
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/planarity/FastPlanarSubgraph.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>

#include "DomModel.h"
#include "LinearEdgeGraphic.h"
#include "FeatureEdgeGraphic.h"

// file view
#include "FileView.h"
#include "FileMiniMap.h"
#include "FileViewInteractiveRect.h"
#include "FileViewNodeGraphic.h"
#include "FileMiniMapNodeGraphic.h"
#include "FileTreeView.h"
#include "FileTreeModel.h"
#include "FileTreeItem.h"

// feature view
#include "FeatureView.h"
#include "FeatureMiniMap.h"
#include "FeatureViewInteractiveRect.h"
#include "FeatureViewNodeGraphic.h"
#include "FeatureTreeView.h"
#include "FeatureTreeModel.h"
#include "FeatureTreeItem.h"
#include "readmodel.h"

// code view
#include "CodeView.h"
#include "CodeMiniMap.h"
#include "SyntaxHighlighter.h"

// main
#include "SMVMain.h"

using namespace smv;

// const variables
const QString PROJECT_ICON = "graphics\\project.png";
const QString FOLDER_ICON = "graphics\\folder.png";

// margin in file graph view between graph and widget border
const int GRAPH_MARGIN = 50;
// constructor
SMVMain::SMVMain(void)
{

	projectLoaded = false;
	feature_mini_map_first_time = true;
	filenodesneedupdate = false;
	filepreviewscreated = false;

	// init
	mFeatureView = 0;
	mFeatureMiniMap = 0;
	mFileView = 0;
	mFileMiniMap = 0;
	editor = 0;
	mCodeMiniMap = 0;

	filescenepreview = 0;
	
	// set up toolbox actions
	createActions();

	// set up menu
	createMenu();

	// set up tool bar
	createToolBars();

	// set up status bar
	createStatusBar();

	// create dock widgets
	createDockWindows();

	// set up file view
	mFileView = new FileView(this);
	mFileMiniMap->setFileView(mFileView);
	mFileMiniMap->setMainWidget(this);

	// set up feature view
	mFeatureView = new FeatureView(this);
	mFeatureMiniMap->setFeatureView(mFeatureView);
	mFeatureMiniMap->setMainWidget(this);

	// set up feature color dialog
	mColorDialog = new QColorDialog(this);
	
	// set up code view
	QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    editor = new CodeView(this);
    editor->setFont(font);
	editor->setReadOnly(true);
	mCodeMiniMap->setCodeView(editor);
    highlighter = new SyntaxHighlighter(editor->document());

	mViewMode = FEATURE_VIEW;
	mFeatureViewLayout = HIERARCHICAL;
	mFileViewLayout = HIERARCHICAL;
	mCurrentView = (QGraphicsView*)mFeatureView;
	
    pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(mFeatureView);
	pagesWidget->addWidget(mFileView);
	pagesWidget->addWidget(editor);
	setCentralWidget(pagesWidget);

	// load icons
	// project icon
	QPixmap *project_pix = new QPixmap();
	project_pix->load(PROJECT_ICON, "PNG");
	NodeIcon project_icon = {project_pix};
	icons.push_back(project_icon);

	// folder icon
	QPixmap *folder_pix = new QPixmap();
	folder_pix->load(FOLDER_ICON, "PNG");
	NodeIcon folder_icon = {folder_pix};
	icons.push_back(folder_icon);

	this->showMaximized();

	// scale views
	mFeatureMiniMap->showFullScene();
	mFileView->showFullScene();
	mFileMiniMap->showFullScene();

	openFile();

	mFileView->resize(mFeatureView->frameSize());
	editor->resize(mFeatureView->frameSize());

	qDebug() << mFeatureView->geometry();
	qDebug() << mFileView->geometry();
	qDebug() << editor->geometry();

}
// destructor
SMVMain::~SMVMain(void)
{
	/* TODO
	** alle mit new() erzeugten Instanzen müssen mit delete() gelöscht werden
	...
	*/

	// delete data from file view
	for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
	{
		mFileView->scene()->removeItem(mFileViewEdgeGraphicData[e].graphics);
	}

	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileView->scene()->removeItem(mFileViewGraphicData[n].graphics);
	}

	QList<QGraphicsItem*> list = mFileView->scene()->items();
	for (int i = 0; i < list.size(); ++i) {
		mFileView->scene()->removeItem(list.at(i));
	}
	mFileView->scene()->clear();

	// delete data from feature view
	for(ogdf::edge e = mFeatureGraph->firstEdge(); e; e = e->succ())
	{
		mFeatureView->scene()->removeItem(mFeatureViewEdgeGraphicData[e].graphics);
	}

	for(ogdf::node n = mFeatureGraph->firstNode(); n; n = n->succ())
	{
		mFeatureView->scene()->removeItem(mFeatureViewGraphicData[n].graphics);
	}

	// delete data from feature view
	list = mFeatureView->scene()->items();
	for (int i = 0; i < list.size(); ++i) {
		mFeatureView->scene()->removeItem(list.at(i));
	}
	mFeatureView->scene()->clear();

	// delete data from feature minimap
	list = mFeatureMiniMap->scene()->items();
	for (int i = 0; i < list.size(); ++i) {
		mFeatureMiniMap->scene()->removeItem(list.at(i));
	}

	// delete data from file tree view
	mFileTreeView->setModel(0);
	delete mFileTreeViewModel;

	// TODO: delete DOM-model ??

	// delete data from feature tree view
	mFeatureTreeView->setModel(0);
	delete mFeatureTreeViewModel;

	delete model;

	// delete feature graph
	mAnnotationsGraph->clear();
	mAnnotationsGA->init(*mAnnotationsGraph,
		ogdf::GraphAttributes::nodeType | 
		ogdf::GraphAttributes::nodeLabel |
		ogdf::GraphAttributes::nodeGraphics |
		ogdf::GraphAttributes::edgeGraphics |
		ogdf::GraphAttributes::nodeColor |
		ogdf::GraphAttributes::edgeColor |
		ogdf::GraphAttributes::nodeLevel | 
		ogdf::GraphAttributes::edgeStyle | 
		ogdf::GraphAttributes::nodeStyle |
		ogdf::GraphAttributes::nodeTemplate);
	mNodeType.init();
	mNodeCollapsed.init();
	mFragmentData.init();
	mFeatureData.init();
	mFeatureList.clear();
	mFileLength.init();
	mFragmentRects.init();
	mHistogramRects.init();
	mFileViewHistogramTotalLengths.init();
	mFileViewHistogramAbsoluteLengths.init();
	mNodeAmbiguityPositions.init();
	nodeLabelVector.clear();
	nodeLabelList.clear();
	nodeLabelCounts.clear();

	// delete file graph
	mFileGraph.clear();

	projectLoaded = false;

	for (unsigned int i = 0; i < icons.size(); i++)
		icons.pop_back();

	delete highlighter;
	delete mColorDialog;

	delete mCodeMiniMap;
	delete mCodeMiniMapDock;
	delete editor;
	
	delete mFeatureMiniMap;
	delete mFeatureMiniMapDock;
	delete mFeatureView;

	delete mFileMiniMap;
	delete mFileMiniMapDock;
	delete mFileView;

	delete pagesWidget;

	delete mFileTreeView;
	delete mFileTreeViewDock;

	delete mFeatureTreeView;
	delete mFeatureTreeViewDock;

	delete aboutAct;
	delete OpacityButton;
	delete TransparencyButton;
	delete ColorButton;
	delete ZPButton;
	delete Stripe10;
	delete Stripe9;
	delete Stripe8;
	delete Stripe7;
	delete Stripe6;
	delete Stripe5;
	delete Stripe4;
	delete Stripe3;
	delete Stripe2;
	delete Stripe1;
	delete ZMButton;
	delete CodeButton;
	delete FileButton;
	delete NodeButton;
	delete mTransSlider;

	delete showFullFileGraphAction;

	delete mAnnotationsGraph;
	delete mAnnotationsGA;
	delete mFeatureGraph;
	delete mFeatureGA;
	
	delete fileMenu;
	delete ansichtMenu;
	delete windowMenu;
	delete helpMenu;
	delete viewToolBar;
	delete zoomToolBar;
	delete extrasToolBar;
	delete opacityToolBar;

}

void SMVMain::about()
{
   QMessageBox::about(this, tr("View Infinity"),
				tr("View Infinity (Version: 08-2010)\n\n"
				"Otto-von-Guericke-University of Magdeburg\n"
               "Department of Simulation and Graphics\n"
			   "Faculty of Computer Science\n"
               "Software & Model Visualization\n\n"
			   "Coding: Michael Stengel, Marco Winter\n"
			   "eMail: virtuellerealitaet@googlemail.com"));
}

void SMVMain::createActions()
{
	// TODO: icon size ? maximum size?
	NodeButton = new QAction(QIcon(".\\graphics\\Node.png"), tr("&Feature View"), this);
	NodeButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    NodeButton->setStatusTip(tr("Show Feature View"));
    connect(NodeButton, SIGNAL(triggered()), this, SLOT(directFadeInFeatureView()));

	FileButton = new QAction(QIcon(".\\graphics\\documents.png"), tr("&File View"), this);
    FileButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    FileButton->setStatusTip(tr("Show File View"));
    connect(FileButton, SIGNAL(triggered()), this, SLOT(directFadeInFileView()));

	CodeButton = new QAction(QIcon(".\\graphics\\code.png"), tr("&Code View"), this);
    CodeButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    CodeButton->setStatusTip(tr("Show Code View"));
    connect(CodeButton, SIGNAL(triggered()), this, SLOT(directFadeInCodeView()));

	ZMButton = new QAction(QIcon(".\\graphics\\Zoom-minus.png"), tr("&Zoom Minus"), this);
	ZMButton->setShortcut(QKeySequence::ZoomOut);
    ZMButton->setStatusTip(tr("Zoom Out"));
    connect(ZMButton, SIGNAL(triggered()), this, SLOT(zmButton()));

	Stripe1 = new QAction(QIcon(".\\graphics\\Stripe11.png"), tr("&Zoom Level 1"), this);
    Stripe1->setStatusTip(tr("Zoom to level 1"));
    connect(Stripe1, SIGNAL(triggered()), this, SLOT(stripe1()));

	Stripe2 = new QAction(QIcon(".\\graphics\\Stripe12.png"), tr("&Zoom Level 2"), this);
    Stripe2->setStatusTip(tr("Zoom to level 2"));
    connect(Stripe2, SIGNAL(triggered()), this, SLOT(stripe2()));

	Stripe3 = new QAction(QIcon(".\\graphics\\Stripe13.png"), tr("&Zoom Level 3"), this);
    Stripe3->setStatusTip(tr("Zoom to level 3"));
    connect(Stripe3, SIGNAL(triggered()), this, SLOT(stripe3()));

	Stripe4 = new QAction(QIcon(".\\graphics\\Stripe14.png"), tr("&Zoom Level 4"), this);
    Stripe4->setStatusTip(tr("Zoom to level 4"));
    connect(Stripe4, SIGNAL(triggered()), this, SLOT(stripe4()));

	Stripe5 = new QAction(QIcon(".\\graphics\\Stripe15.png"), tr("&Zoom Level 5"), this);
    Stripe5->setStatusTip(tr("Zoom to level 5"));
    connect(Stripe5, SIGNAL(triggered()), this, SLOT(stripe5()));

	Stripe6 = new QAction(QIcon(".\\graphics\\Stripe16.png"), tr("&Zoom Level 6"), this);
    Stripe6->setStatusTip(tr("Zoom to level 6"));
    connect(Stripe6, SIGNAL(triggered()), this, SLOT(stripe6()));

	Stripe7 = new QAction(QIcon(".\\graphics\\Stripe17.png"), tr("&Zoom Level 7"), this);
    Stripe7->setStatusTip(tr("Zoom to level 7"));
    connect(Stripe7, SIGNAL(triggered()), this, SLOT(stripe7()));

	Stripe8 = new QAction(QIcon(".\\graphics\\Stripe18.png"), tr("&Zoom Level 8"), this);
    Stripe8->setStatusTip(tr("Zoom to level 8"));
    connect(Stripe8, SIGNAL(triggered()), this, SLOT(stripe8()));

	Stripe9 = new QAction(QIcon(".\\graphics\\Stripe19.png"), tr("&Zoom Level 9"), this);
    Stripe9->setStatusTip(tr("Zoom to level 9"));
    connect(Stripe9, SIGNAL(triggered()), this, SLOT(stripe9()));

	Stripe10 = new QAction(QIcon(".\\graphics\\Stripe.png"), tr("&Zoom Level 10"), this);
    Stripe10->setStatusTip(tr("Zoom to level 10"));
    connect(Stripe10, SIGNAL(triggered()), this, SLOT(stripe10()));

	ZPButton = new QAction(QIcon(".\\graphics\\Zoom-plus.png"), tr("&Zoom Plus"), this);
	ZPButton->setShortcut(QKeySequence::ZoomIn);
    ZPButton->setStatusTip(tr("Zoom In"));
    connect(ZPButton, SIGNAL(triggered()), this, SLOT(zpButton()));

	ColorButton = new QAction(QIcon(".\\graphics\\Color.png"), tr("&Pick Color"), this);
	ColorButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    ColorButton->setStatusTip(tr("Pick Feature Color"));
    connect(ColorButton, SIGNAL(triggered()), this, SLOT(openColorDialog()));

	TransparencyButton = new QAction(QIcon(".\\graphics\\transparency_icon.png"), tr("&Set features fully transparent"), this);
	TransparencyButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    TransparencyButton->setStatusTip(tr("Set features fully transparent"));
    connect(TransparencyButton, SIGNAL(triggered()), this, SLOT(setFeaturesTransparent()));

	OpacityButton = new QAction(QIcon(".\\graphics\\opacity_icon.png"), tr("&Set features fully opaque"), this);
	OpacityButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    OpacityButton->setStatusTip(tr("Set features fully opaque"));
    connect(OpacityButton, SIGNAL(triggered()), this, SLOT(setFeaturesOpaque()));

	aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void SMVMain::createMenu()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("&Open Project"), this, SLOT(openFile()));
	fileMenu->addAction(tr("&Export file graph into GML"),this, SLOT(writeGML()));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("&Close"), this, SLOT(close()));
		
	ansichtMenu = menuBar()->addMenu(tr("&View"));
	ansichtMenu->addAction(NodeButton);
	ansichtMenu->addAction(FileButton);
	ansichtMenu->addAction(CodeButton);
	ansichtMenu->addAction(ZMButton);
	ansichtMenu->addAction(ZPButton);
	ansichtMenu->addSeparator();
	showFullFileGraphAction = ansichtMenu->addAction(tr("Show full filegraph"));
	connect(showFullFileGraphAction, SIGNAL(triggered()), this, SLOT(setFullFileGraph()));
	showFullFileGraphAction->setCheckable(true);
	showFullFileGraphAction->setChecked(true);
	ansichtMenu->addSeparator();
	ansichtMenu->addAction(tr("Balloon Layout"),this, SLOT(setBallonLayout()));
	ansichtMenu->addAction(tr("circular Layout"),this, SLOT(setCircularLayout()));
	ansichtMenu->addAction(tr("Energy-based Layout"),this, SLOT(setEnergybasedLayout()));
	ansichtMenu->addAction(tr("Fast hierarchical Layout"),this, SLOT(setFastHierarchicalLayout()));
	ansichtMenu->addAction(tr("Hierarchical Layout"),this, SLOT(setHierarchicalLayout()));
	//ansichtMenu->addAction(tr("Orthogonal Layout"),this, SLOT(setOrthogonalLayout()));
    ansichtMenu->addAction(tr("Tree Layout"),this, SLOT(setTreeLayout()));

	windowMenu = menuBar()->addMenu(tr("&Window"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void SMVMain::createToolBars()
{
    viewToolBar = addToolBar(tr("View"));
    viewToolBar->addAction(NodeButton);
    viewToolBar->addAction(FileButton);
    viewToolBar->addAction(CodeButton);

    zoomToolBar = addToolBar(tr("Zooming"));
    zoomToolBar->addAction(ZMButton);
	zoomToolBar->addAction(Stripe1);
	zoomToolBar->addAction(Stripe2);
	zoomToolBar->addAction(Stripe3);
	zoomToolBar->addAction(Stripe4);
	zoomToolBar->addAction(Stripe5);
	zoomToolBar->addAction(Stripe6);
	zoomToolBar->addAction(Stripe7);
	zoomToolBar->addAction(Stripe8);
	zoomToolBar->addAction(Stripe9);
	zoomToolBar->addAction(Stripe10);
	zoomToolBar->addAction(ZPButton);

	extrasToolBar = addToolBar(tr("Additional Functions"));
	extrasToolBar->addAction(ColorButton);

	opacityToolBar = addToolBar(tr("Feature Transparency"));
	// add color transparency slider
	mTransSlider = new QSlider(Qt::Horizontal, opacityToolBar);
	mTransSlider->setMaximum ( 255 );
	mTransSlider->setMinimum ( 0 );
	mTransSlider->setTracking(false);
	//mFeatureMiniMap->setMainWidget(this);
	
	mTransSlider->setValue(128);
	mTransSlider->setFixedWidth(100);
	
	qApp->setStyleSheet(
		"QSlider::groove:horizontal {border: 1px solid #bbb;background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 1,"
			"stop: 0.1 #fff, stop: 1.0 #222);height: 10px;border-radius: 4px;}"
		"QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #eee, stop:1 #ccc);"
		"border: 1px solid #777;width: 13px;margin-top: -2px;margin-bottom: -2px;border-radius: 4px;}"
		"QSlider::handle:horizontal:hover {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #fff, stop:1 #ddd);"
		"border: 1px solid #444;border-radius: 4px;}"
		"QSlider::sub-page:horizontal:disabled {background: #bbb;border-color: #999;}"
		"QSlider::add-page:horizontal:disabled {background: #eee;border-color: #999;}"
		"QSlider::handle:horizontal:disabled {background: #eee;border: 1px solid #aaa;border-radius: 4px;}"
	);
	
	opacityToolBar->addAction(TransparencyButton);
	opacityToolBar->addWidget(mTransSlider);
	connect(mTransSlider, SIGNAL(valueChanged(int)), this, SLOT(CodeColorTransparency(int)));
	opacityToolBar->addAction(OpacityButton);
	opacityToolBar->hide();
}

void SMVMain::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void SMVMain::createDockWindows()
{
	// add feature mini map
	mFeatureMiniMapDock = new QDockWidget(tr("Feature Mini Map"), this);
    mFeatureMiniMapDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mFeatureMiniMap = new FeatureMiniMap(mFeatureMiniMapDock);
	mFeatureMiniMap->setMainWidget(this);
	mFeatureMiniMapDock->setWidget(mFeatureMiniMap);
	addDockWidget(Qt::RightDockWidgetArea, mFeatureMiniMapDock);
    windowMenu->addAction(mFeatureMiniMapDock->toggleViewAction());
	//mFeatureMiniMapDock->setMaximumSize(this->geometry().width()/2,16777215);
	mFeatureMiniMapDock->setFixedWidth(300);
	mFeatureMiniMapDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	mFeatureMiniMapDock->setFeatures(QDockWidget::DockWidgetClosable);

	// add file mini map
	mFileMiniMapDock = new QDockWidget(tr("File Mini Map"), this);
    mFileMiniMapDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mFileMiniMap = new FileMiniMap(mFileMiniMapDock);
	mFileMiniMap->setMainWidget(this);
    mFileMiniMapDock->setWidget(mFileMiniMap);
	addDockWidget(Qt::RightDockWidgetArea, mFileMiniMapDock);
    windowMenu->addAction(mFileMiniMapDock->toggleViewAction());
	mFileMiniMapDock->hide();
	//mFileMiniMapDock->setMaximumSize(this->geometry().width()/2,16777215);
	mFileMiniMapDock->setFixedWidth(300);
	mFileMiniMapDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	mFileMiniMapDock->setFeatures(QDockWidget::DockWidgetClosable);

	// add code mini map
	mCodeMiniMapDock = new QDockWidget(tr("Code Mini Map"), this);
    mCodeMiniMapDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mCodeMiniMap = new CodeMiniMap(mCodeMiniMapDock);
    mCodeMiniMapDock->setWidget(mCodeMiniMap);
	addDockWidget(Qt::RightDockWidgetArea, mCodeMiniMapDock);
	windowMenu->addAction(mCodeMiniMapDock->toggleViewAction());
	mCodeMiniMapDock->hide();
	//mCodeMiniMapDock->setMaximumSize(this->geometry().width()/2,16777215);
	mCodeMiniMapDock->setFixedWidth(300);
	mCodeMiniMapDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	mCodeMiniMapDock->setFeatures(QDockWidget::DockWidgetClosable);

	// add file tree indentation view
	mFileTreeViewDock = new QDockWidget(tr("File Tree View"), this);
    mFileTreeViewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	mFileTreeView = new FileTreeView(mFileTreeViewDock);
	mFileTreeView->setAnimated(true);
    mFileTreeViewDock->setWidget(mFileTreeView);
    addDockWidget(Qt::LeftDockWidgetArea, mFileTreeViewDock);
    windowMenu->addAction(mFileTreeViewDock->toggleViewAction());

	// add feature tree indentation view
	mFeatureTreeViewDock = new QDockWidget(tr("Feature Tree View"), this);
    mFeatureTreeViewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	mFeatureTreeView = new FeatureTreeView(mFeatureTreeViewDock);
	mFeatureTreeView->setAnimated(false);
	mFeatureTreeView->setItemsExpandable(false);
    mFeatureTreeViewDock->setWidget(mFeatureTreeView);
    addDockWidget(Qt::LeftDockWidgetArea, mFeatureTreeViewDock);
    windowMenu->addAction(mFeatureTreeViewDock->toggleViewAction());

	// order feature tree view and file tree view into tabs
	this->tabifyDockWidget(mFeatureTreeViewDock, mFileTreeViewDock);

	connect(mFileTreeView,SIGNAL(collapsed(QModelIndex)),this,SLOT(treeViewCollapsSlot(QModelIndex)));
	connect(mFileTreeView,SIGNAL(expanded(QModelIndex)),this,SLOT(treeViewExpandSlot(QModelIndex)));

	connect(mFeatureTreeView,SIGNAL(collapsed(QModelIndex)),this,SLOT(featureTreeViewCollapsSlot(QModelIndex)));
	connect(mFeatureTreeView,SIGNAL(expanded(QModelIndex)),this,SLOT(featureTreeViewExpandSlot(QModelIndex)));
}

void SMVMain::openFile()
{

	QString filePath;
	// open file
	/*

	// automatic loading at start-up
	if (feature_mini_map_first_time == true)
	{
		filePath = "Examples\\ExpressionP";
		//filePath = "C:\\smv_subversion\\smvifm\\XMLProjects\\berkeley4typechecks";
		//filePath = "C:\\smv_subversion\\smvifm\\XMLProjects\\prevayler";
	}
	else
	*/
	{
		filePath = QFileDialog::getExistingDirectory(this,tr("Open Project Directory"),
		"",QFileDialog::ShowDirsOnly);
	}
	QString statusmsg;

	if (filePath.length() > 0)
	{
		if (projectLoaded == true)
		{
			statusBar()->showMessage(tr("Unloading previous project..."));
			// delete data from file view
			for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
			{
				mFileView->scene()->removeItem(mFileViewEdgeGraphicData[e].graphics);
			}

			for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
			{
				mFileView->scene()->removeItem(mFileViewGraphicData[n].graphics);
			}

			QList<QGraphicsItem*> list = mFileView->scene()->items();
			for (int i = 0; i < list.size(); ++i) {
				mFileView->scene()->removeItem(list.at(i));
			}
			mFileView->scene()->clear();

			// delete data from feature view
			for(ogdf::edge e = mFeatureGraph->firstEdge(); e; e = e->succ())
			{
				mFeatureView->scene()->removeItem(mFeatureViewEdgeGraphicData[e].graphics);
			}

			for(ogdf::node n = mFeatureGraph->firstNode(); n; n = n->succ())
			{
				mFeatureView->scene()->removeItem(mFeatureViewGraphicData[n].graphics);
			}

			// delete data from feature view
			list = mFeatureView->scene()->items();
			for (int i = 0; i < list.size(); ++i) {
				mFeatureView->scene()->removeItem(list.at(i));
			}
			mFeatureView->scene()->clear();

			// delete data from feature minimap
			list = mFeatureMiniMap->scene()->items();
			for (int i = 0; i < list.size(); ++i) {
				// dont delete rect!!!
				if (list.at(i) != (QGraphicsItem*)mFeatureMiniMap->m_intrect)
					mFeatureMiniMap->scene()->removeItem(list.at(i));
			}

			// delete data from file tree view
			mFileTreeView->setModel(0);
			delete mFileTreeViewModel;

			// TODO: delete DOM-model ??

			// delete data from feature tree view
			mFeatureTreeView->setModel(0);
			delete mFeatureTreeViewModel;

			// delete feature graph
			mAnnotationsGraph->clear();
			mAnnotationsGA->init(*mAnnotationsGraph,
				ogdf::GraphAttributes::nodeType | 
				ogdf::GraphAttributes::nodeLabel |
				ogdf::GraphAttributes::nodeGraphics |
				ogdf::GraphAttributes::edgeGraphics |
				ogdf::GraphAttributes::nodeColor |
				ogdf::GraphAttributes::edgeColor |
				ogdf::GraphAttributes::nodeLevel | 
				ogdf::GraphAttributes::edgeStyle | 
				ogdf::GraphAttributes::nodeStyle |
				ogdf::GraphAttributes::nodeTemplate);
			mNodeType.init();
			mNodeCollapsed.init();
			mFragmentData.init();
			mFeatureData.init();
			mFeatureList.clear();
			mFileLength.init();
			mFragmentRects.init();
			mHistogramRects.init();
			mFileViewHistogramTotalLengths.init();
			mFileViewHistogramAbsoluteLengths.init();
			mNodeAmbiguityPositions.init();
			nodeLabelVector.clear();
			nodeLabelList.clear();
			nodeLabelCounts.clear();

			// delete file graph
			mFileGraph.clear();
			//mFileGA->clearAllBends();
			mFileGA.init(mFileGraph,
				ogdf::GraphAttributes::nodeType | 
				ogdf::GraphAttributes::nodeLabel |
				ogdf::GraphAttributes::nodeGraphics |
				ogdf::GraphAttributes::edgeGraphics |
				ogdf::GraphAttributes::nodeColor |
				ogdf::GraphAttributes::edgeColor |
				ogdf::GraphAttributes::nodeLevel | 
				ogdf::GraphAttributes::edgeStyle | 
				ogdf::GraphAttributes::nodeStyle |
				ogdf::GraphAttributes::nodeTemplate);
			mFileNodeType.init();
			mFileNodeCollapsed.init();
			mFileViewFragmentData.init();
			mFileViewFeatureData.init();
			mFileViewFileLength.init();
			mFileViewFragmentRects.init();
			mFileViewHistogramRects.init();
			mFileViewGraphicData.init();
			mFileViewEdgeGraphicData.init();
			mFileNodeToBeDeleted.init();
			mFileNodeAmbiguityPositions.init();
			mFileToolTipLabels.init();
			mFileViewPreviewPixmaps.init();

			mFileViewEventNodePosition = QPointF();
			mFileViewEventNodeLabel = ogdf::String();
			mFileViewEventNodeAmbigPosition = 0;

			strFileTree = QString();
			strSearchedNode = QString();
			idxSearchedNodes.clear();
			positionSearchedNode = 0;
			idxSearchedNode = QModelIndex();
			searchedNode = ogdf::node();

			path_project.clear();
			path_annotations_xml.clear();
			path_model_m.clear();

			filepreviewscreated = false;

			projectLoaded = false;
			statusBar()->showMessage(tr("Unloading successful!"));
		}
	
		// set paths
		path_project = filePath;
		path_annotations_xml.append(path_project);
		path_annotations_xml.append("\\annotations.xml");
		path_model_m.append(path_project);
		path_model_m.append("\\model.m");

		QFile annotxml(path_annotations_xml);
		if (!(annotxml.open(QIODevice::ReadOnly))) {
			QMessageBox msgBox;
			statusBar()->showMessage(tr("ERROR: Illegal project directory! No annotations.xml available!"));
			msgBox.setText("Error: Illegal project directory!\nNo annotations.xml available!");
			msgBox.exec();
			return;
		}
		QFile modelm(path_model_m);
		if (!(modelm.open(QIODevice::ReadOnly))) {
			QMessageBox msgBox;
			statusBar()->showMessage(tr("ERROR: Illegal project directory! No model.m available!"));
			msgBox.setText("Error: Illegal project directory!\nNo model.m available!");
			msgBox.exec();
			return;
		}
		
		// app title
		QStringList list1 = path_annotations_xml.split("\\");
		setWindowTitle(tr("SMV: Interactive Feature Model (")+list1[list1.size()-2].toLatin1()+")");

		// load annotations.xml for creation of feature graph
		loadAnnotationsXML();
		
		// create annotations graph
		createAnnotationsGraph();

		projectLoaded = true;
		feature_mini_map_first_time = true;

		// create feature graph
		createFeatureGraph();
		updateFeatureStates();

		// create file graph
		createFileGraph();

		// update file graph
		collapseFileViewNode();
		
		// scale views
		mFileView->calculateScalingLimits();		// calculate scaling limits for file view
		mFeatureView->showFullScene();
		mFeatureMiniMap->showFullScene();
		mFileView->showFullScene();
		mFileMiniMap->showFullScene();

		statusmsg.append(path_project);
		statusmsg.append(" loaded. Ready.");
		statusBar()->showMessage(tr(statusmsg.toLatin1()));

		// TODO: show main view with file view
	}
}
void SMVMain::setCircularLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = CIRCULAR;

			// calculate circular layout
			ogdf::CircularLayout *cl = new ogdf::CircularLayout();
			cl->minDistCC(20.0); // standard 20.0
			cl->minDistCircle(20.0); // standard 20.0
			cl->minDistLevel(20.0); // standard 20.0
			cl->minDistSibling(10.0); // standard 10.0
			cl->pageRatio(1.0); // standard 1.0
			cl->call(mFileGA);

			// update BoundingBox
			mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();

		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = CIRCULAR;

			// calculate circular layout
			ogdf::CircularLayout *cl = new ogdf::CircularLayout();
			cl->minDistCC(20.0); // standard 20.0
			cl->minDistCircle(20.0); // standard 20.0
			cl->minDistLevel(20.0); // standard 20.0
			cl->minDistSibling(10.0); // standard 10.0
			cl->pageRatio(1.0); // standard 1.0
			cl->call(*mFeatureGA);

			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);


			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();

		}
	}
}

void SMVMain::setFastHierarchicalLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = FASTHIERARCHICAL;

			ogdf::SugiyamaLayout SL;
			SL.setRanking(new ogdf::OptimalRanking);
			SL.setCrossMin(new ogdf::MedianHeuristic);

			// calculate fast hierarchical layout
			ogdf::FastHierarchyLayout *FH = new ogdf::FastHierarchyLayout();
			ogdf::Hierarchy hier;
			FH->call(hier,mFileGA);
			SL.setLayout(FH);
			SL.call(mFileGA);

			// update BoundingBox
			mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}
		
			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = FASTHIERARCHICAL;

			ogdf::SugiyamaLayout SL;
			SL.setRanking(new ogdf::OptimalRanking);
			SL.setCrossMin(new ogdf::MedianHeuristic);

			// calculate fast hierarchical layout
			ogdf::FastHierarchyLayout *FH = new ogdf::FastHierarchyLayout();
			ogdf::Hierarchy hier;
			FH->call(hier,mFileGA);
			SL.setLayout(FH);
			SL.call(*mFeatureGA);

			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();

		}
	}
}

void SMVMain::setBallonLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = BALLON;

			// calculate balloon layout
			ogdf::BalloonLayout *ba = new ogdf::BalloonLayout();
			ba->setEvenAngles(false);
			ba->call(mFileGA);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}
			
			// update bounding box
			qreal mostTop = mFileGA.boundingBox().p1().m_y;
			qreal mostBottom = mFileGA.boundingBox().p2().m_y;
			qreal mostLeft = mFileGA.boundingBox().p1().m_x;
			qreal mostRight = mFileGA.boundingBox().p2().m_x;
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				if (mFileGA.x(v) < mostLeft)
					mostLeft = mFileGA.x(v);
				if (mFileGA.x(v) > mostRight)
					mostRight = mFileGA.x(v);
				if (mFileGA.y(v) > mostBottom)
					mostLeft = mFileGA.x(v);
				if (mFileGA.y(v) < mostTop)
					mostRight = mFileGA.x(v);
			}
			// update BoundingBox
			mFileView->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = BALLON;

			// calculate balloon layout
			ogdf::BalloonLayout *ba = new ogdf::BalloonLayout();
			ba->setEvenAngles(false);
			ba->call(*mFeatureGA);

			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}
			// update bounding box
			qreal mostTop = mFeatureGA->boundingBox().p1().m_y;
			qreal mostBottom = mFeatureGA->boundingBox().p2().m_y;
			qreal mostLeft = mFeatureGA->boundingBox().p1().m_x;
			qreal mostRight = mFeatureGA->boundingBox().p2().m_x;
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				if (mFeatureGA->x(v) < mostLeft)
					mostLeft = mFileGA.x(v);
				if (mFeatureGA->x(v) > mostRight)
					mostRight = mFileGA.x(v);
				if (mFeatureGA->y(v) > mostBottom)
					mostLeft = mFileGA.x(v);
				if (mFeatureGA->y(v) < mostTop)
					mostRight = mFileGA.x(v);
			}
			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();
		}
	}
}

void SMVMain::setTreeLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = TREE;

			// calculate tree layout
			ogdf::TreeLayout tree;

			// options
			tree.siblingDistance(20.0);
			tree.subtreeDistance(60.0);
			tree.levelDistance(60.0);
			tree.treeDistance(60.0);
			tree.orthogonalLayout(false);
			tree.orientation(ogdf::bottomToTop);
			tree.rootSelection(ogdf::TreeLayout::rootIsSource);
			tree.call(mFileGA);
			
			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}

			// update bounding box
			qreal mostTop = mFileGA.boundingBox().p1().m_y;
			qreal mostBottom = mFileGA.boundingBox().p2().m_y;
			qreal mostLeft = mFileGA.boundingBox().p1().m_x;
			qreal mostRight = mFileGA.boundingBox().p2().m_x;
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				if (mFileGA.x(v) < mostLeft)
					mostLeft = mFileGA.x(v);
				if (mFileGA.x(v) > mostRight)
					mostRight = mFileGA.x(v);
				if (mFileGA.y(v) > mostBottom)
					mostLeft = mFileGA.x(v);
				if (mFileGA.y(v) < mostTop)
					mostRight = mFileGA.x(v);
			}
			// update BoundingBox
			mFileView->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = TREE;

			// calculate tree layout
			ogdf::TreeLayout tree;

			// options
			tree.siblingDistance(20.0);
			tree.subtreeDistance(60.0);
			tree.levelDistance(60.0);
			tree.treeDistance(60.0);
			tree.orthogonalLayout(false);
			tree.orientation(ogdf::bottomToTop);
			tree.rootSelection(ogdf::TreeLayout::rootIsSource);
			tree.call(*mFeatureGA);

			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}
			// update bounding box
			qreal mostTop = mFeatureGA->boundingBox().p1().m_y;
			qreal mostBottom = mFeatureGA->boundingBox().p2().m_y;
			qreal mostLeft = mFeatureGA->boundingBox().p1().m_x;
			qreal mostRight = mFeatureGA->boundingBox().p2().m_x;
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				if (mFeatureGA->x(v) < mostLeft)
					mostLeft = mFileGA.x(v);
				if (mFeatureGA->x(v) > mostRight)
					mostRight = mFileGA.x(v);
				if (mFeatureGA->y(v) > mostBottom)
					mostLeft = mFileGA.x(v);
				if (mFeatureGA->y(v) < mostTop)
					mostRight = mFileGA.x(v);
			}
			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mostLeft - GRAPH_MARGIN,
				mostTop - GRAPH_MARGIN,
				(mostRight-mostLeft) + 2 * GRAPH_MARGIN,
				(mostBottom-mostTop) + 2 * GRAPH_MARGIN);

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();
		}
	}
}

void SMVMain::setHierarchicalLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = HIERARCHICAL;

			// calculate hierarchical layout
			ogdf::SugiyamaLayout SL;
			SL.setRanking(new ogdf::OptimalRanking);
			SL.setCrossMin(new ogdf::MedianHeuristic);
			ogdf::OptimalHierarchyLayout *ohl = new ogdf::OptimalHierarchyLayout;
			ohl->layerDistance(10.0);
			ohl->nodeDistance(10.0);
			ohl->weightBalancing(0.8);
			SL.setLayout(ohl);
			SL.call(mFileGA);

			// update BoundingBox
			mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = HIERARCHICAL;

			// calculate hierarchical layout
			ogdf::SugiyamaLayout SL;
			SL.setRanking(new ogdf::OptimalRanking);
			SL.setCrossMin(new ogdf::MedianHeuristic);
			ogdf::OptimalHierarchyLayout *ohl = new ogdf::OptimalHierarchyLayout;
			ohl->layerDistance(10.0);
			ohl->nodeDistance(10.0);
			ohl->weightBalancing(0.8);
			SL.setLayout(ohl);
			SL.call(*mFeatureGA);

			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();
		}
	}
}

void SMVMain::setEnergybasedLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = ENERGYBASED;

			// calculate energy based layout
			ogdf::FMMMLayout fmmm;

			fmmm.useHighLevelOptions(true);
			fmmm.unitEdgeLength(200.0); 
			fmmm.newInitialPlacement(true);
			//fmmm.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);
			fmmm.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);
		 
			fmmm.call(mFileGA);

			// update BoundingBox
			mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
		else if (view == FEATURE_VIEW)
		{
			mFeatureViewLayout = ENERGYBASED;

			// calculate energy based layout
			ogdf::FMMMLayout fmmm;
			fmmm.useHighLevelOptions(true);
			fmmm.unitEdgeLength(200.0); 
			fmmm.newInitialPlacement(true);
			fmmm.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);
		 	fmmm.call(*mFeatureGA);
		 
			// update BoundingBox
			mFeatureView->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFeatureMiniMap->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update positions
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

				mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
				mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFeatureViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFeatureView->showFullScene();
			mFeatureMiniMap->showFullScene();
		}
	}
}

void SMVMain::setOrthogonalLayout(ViewMode view)
{
	if (projectLoaded == true)
	{
		if (view == UNKNOWN)
			view = mViewMode;

		if (view == FILE_VIEW)
		{
			mFileViewLayout = ORTHOGONAL;

			// calculate orthogonal layout
			ogdf::PlanarizationLayout pl;

			ogdf::FastPlanarSubgraph *ps = new ogdf::FastPlanarSubgraph;
			ps->runs(100);
			ogdf::VariableEmbeddingInserter *ves = new ogdf::VariableEmbeddingInserter;
			ves->removeReinsert(ogdf::EdgeInsertionModule::rrAll);
			pl.setSubgraph(ps);
			pl.setInserter(ves);

			ogdf::EmbedderMinDepthMaxFaceLayers *emb = new ogdf::EmbedderMinDepthMaxFaceLayers;
			pl.setEmbedder(emb);

			ogdf::OrthoLayout *ol = new ogdf::OrthoLayout;
			ol->separation(40.0);
			ol->cOverhang(0.4);
			ol->setOptions(0);
			pl.setPlanarLayouter(ol);

			pl.call(mFileGA);

			// update BoundingBox
			mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
				mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
				mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

			//update node and edge positions
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

				mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
				mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
				mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
			}
			for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
			{
				for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->source() == v) // target is this node
					{
						updateFileViewEdgePosition(e);
					}
				}
			}

			// scale views
			mFileView->showFullScene();
			mFileMiniMap->showFullScene();
		}
	}
}

void SMVMain::writeGML()
{
	if (projectLoaded == true)
	{
		QString str;
		str.append(mFileGA.labelNode(mFileGraph.firstNode()).cstr());
		str.append("_filegraph_output.gml");
		mFileGA.writeGML(ogdf::String(str.toLatin1()));
		QMessageBox msgBox;
		msgBox.setText("File Graph written to GML-File!");
		msgBox.exec();
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("Writing not possible! No Project has been loaded!");
		msgBox.exec();
	}
}

void SMVMain::setFullFileGraph()
{
	createFileGraph();
}

void SMVMain::directFadeInFeatureView()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->setNextWidgetToFadeIn(0);
		mFileView->fadeOut();
	}
	else if (mViewMode == CODE_VIEW)
	{
		editor->setNextWidgetToFadeIn(0);
		editor->fadeOut();
	}
}

void SMVMain::directFadeInFileView()
{
	if (featureInFileView.isEmpty())
	{
		QString msg("Please zoom into a feature first!");
		QMessageBox msgBox;
		msgBox.setText(msg);
		msgBox.exec();
		return;
	}
	else
	{
		if (mViewMode == FEATURE_VIEW)
		{
			mFeatureView->setNextWidgetToFadeIn(1);
			mFeatureView->fadeOut();
		}
		else if (mViewMode == CODE_VIEW)
		{
			editor->setNextWidgetToFadeIn(1);
			editor->fadeOut();
		}
	}
}

void SMVMain::directFadeInCodeView()
{
	
	if (mFileView->zoomIntoFileGraphic == 0)
	{
		QString msg("Please zoom into a file first!");
		QMessageBox msgBox;
		msgBox.setText(msg);
		msgBox.exec();
		return;
	}
	else
	{
		if (mViewMode == FEATURE_VIEW)
		{
			mFeatureView->setNextWidgetToFadeIn(2);
			mFeatureView->fadeOut();
		}
		else if (mViewMode == FILE_VIEW)
		{
			mFileView->setNextWidgetToFadeIn(2);
			mFileView->fadeOut();
		}
	}
}

void SMVMain::fadeInFeatureView()
{
	//qDebug() << "fading into feature view";

	statusBar()->showMessage(tr("Changing into feature view..."));

	mViewMode = FEATURE_VIEW;
	mCurrentView = (QWidget*)mFeatureView;

	int main_height_parts = this->mFeatureView->frameGeometry().height();

	pagesWidget->setCurrentIndex(0);

	mFeatureView->setForegroundOpaque();
	mFeatureView->setEnabled(true);
	mFeatureView->show();
	mFeatureView->fadeIn();

	Stripe1->setEnabled(true);
	Stripe2->setEnabled(true);
	Stripe3->setEnabled(true);
	Stripe4->setEnabled(true);
	Stripe5->setEnabled(true);
	Stripe6->setEnabled(true);
	Stripe7->setEnabled(true);
	Stripe8->setEnabled(true);
	Stripe9->setEnabled(true);
	Stripe10->setEnabled(true);
	ZPButton->setEnabled(true);

	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	// scale views
	//mFeatureView->showFullScene();

	// set navigator position to selected feature node
	/*
	for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
	{
		if (QString::compare(mFeatureGA->labelNode(v).cstr(), featureInFileView) == 0)
		{
			mFeatureView->centerOn(mFeatureViewGraphicData[v].graphics->scenePos());
			mFeatureView->update(mFeatureView->rect());
			mFeatureMiniMap->updateContent();
		}
	}
	*/
	mFeatureMiniMap->showFullScene();

	extrasToolBar->show();
	opacityToolBar->hide();

	mFileView->hide();
	mFileView->setDisabled(true);
	mFileMiniMapDock->hide();
	mFileMiniMap->hide();
	mFileMiniMap->setDisabled(true);

	editor->hide();
	editor->setDisabled(true);
	mCodeMiniMapDock->hide();
	mCodeMiniMap->hide();
	mCodeMiniMap->setDisabled(true);

	mFeatureView->setFocus();
	//mFeatureView->grabMouse();
	statusBar()->showMessage(tr("Feature view ready."));
}
void SMVMain::fadeInFileView()
{
	//qDebug() << "fadeInFileView()";

	statusBar()->showMessage(tr("Changing into file view..."));

	mViewMode = FILE_VIEW;
	mCurrentView = (QWidget*)mFileView;

	int main_height_parts = this->mFeatureView->frameGeometry().height() / 2;

	extrasToolBar->hide();
	opacityToolBar->hide();

	pagesWidget->setCurrentIndex(1);

	mFileView->setForegroundOpaque();
	mFileView->setEnabled(true);
	mFileView->show();

	mFileView->fadeIn();

	Stripe1->setEnabled(true);
	Stripe2->setEnabled(true);
	Stripe3->setEnabled(true);
	Stripe4->setEnabled(true);
	Stripe5->setEnabled(true);
	Stripe6->setEnabled(true);
	Stripe7->setEnabled(true);
	Stripe8->setEnabled(true);
	Stripe9->setEnabled(true);
	Stripe10->setEnabled(true);
	ZPButton->setEnabled(true);

	mFileMiniMap->setEnabled(true);
	mFileMiniMap->show();
	mFileMiniMapDock->setFixedHeight(main_height_parts);
	mFileMiniMapDock->show();

	mFeatureView->hide();
	mFeatureView->setDisabled(true);

	//mFeatureView->setEnabled(true);
	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	editor->hide();
	editor->setDisabled(true);
	mCodeMiniMapDock->hide();
	mCodeMiniMap->hide();
	mCodeMiniMap->setDisabled(true);
	
	mFileView->setFocus();
	
	if (feature_mini_map_first_time)
	{
		//mFileView->showFullScene();
		mFileMiniMap->showFullScene();
		feature_mini_map_first_time = false;
	}

	// set navigator position on zoomed node

	mFeatureMiniMap->centerOn(mFeatureView->zoomIntoFeatureGraphic);
	//update(mFeatureView->rect());
	//mFeatureMiniMap->updateContent();

	qreal item_width = mFeatureView->zoomIntoFeatureGraphic->boundingRect().width();
	qreal item_height = mFeatureView->zoomIntoFeatureGraphic->boundingRect().height();

	QPointF rect_topleft, rect_bottomright;
	rect_topleft.setX(mFeatureView->zoomIntoFeatureGraphic->scenePos().x() - item_width);
	rect_topleft.setY(mFeatureView->zoomIntoFeatureGraphic->scenePos().y() - item_height);
	rect_bottomright.setX(mFeatureView->zoomIntoFeatureGraphic->scenePos().x() + item_width);
	rect_bottomright.setY(mFeatureView->zoomIntoFeatureGraphic->scenePos().y() + item_height);
	mFeatureMiniMap->m_intrect->setRectInScene(rect_topleft, rect_bottomright);

	statusBar()->showMessage(tr("File view ready."));


}
void SMVMain::fadeInCodeView()
{
	//qDebug() << "fading into code view";

	statusBar()->showMessage(tr("Changing into code view..."));

	mViewMode = CODE_VIEW;
	mCurrentView = (QWidget*)editor;

	int main_height_parts = this->mFeatureView->frameGeometry().height() / 4;

	extrasToolBar->hide();
	opacityToolBar->show();

	editor->setForegroundOpaque();
	editor->setEnabled(true);
	editor->show();
	editor->fadeIn();

	Stripe1->setDisabled(true);
	Stripe2->setDisabled(true);
	Stripe3->setDisabled(true);
	Stripe4->setDisabled(true);
	Stripe5->setDisabled(true);
	Stripe6->setDisabled(true);
	Stripe7->setDisabled(true);
	Stripe8->setDisabled(true);
	Stripe9->setDisabled(true);
	Stripe10->setDisabled(true);
	ZPButton->setDisabled(true);

	mCodeMiniMap->setEnabled(true);
	mCodeMiniMap->show();
	mCodeMiniMapDock->setFixedHeight(main_height_parts * 2);
	mCodeMiniMapDock->show();
		
	pagesWidget->setCurrentIndex(2);

	mFeatureView->hide();
	mFeatureView->setDisabled(true);
	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	mFileView->hide();
	mFileView->setDisabled(true);
	mFileMiniMap->setEnabled(true);
	mFileMiniMap->show();
	mFileMiniMapDock->setFixedHeight(main_height_parts);
	mFileMiniMapDock->show();

	editor->setFocus();
	editor->createMiniMapPixMap();

	// calculate fragment rectangles
	statusBar()->showMessage(tr("Calculating Fragment Rectangles..."));
	editor->createFragmentRects();
	editor->fadeOutFragments();

	// set navigator position on zoomed node
	mFileMiniMap->centerOn(mFileView->zoomIntoFileGraphic);
	//update(mFileView->rect());
	//mFileMiniMap->updateContent();

	qreal item_width = mFileView->zoomIntoFileGraphic->boundingRect().width();
	qreal item_height = mFileView->zoomIntoFileGraphic->boundingRect().height();

	QPointF rect_topleft, rect_bottomright;
	rect_topleft.setX(mFileView->zoomIntoFileGraphic->scenePos().x() - item_width);
	rect_topleft.setY(mFileView->zoomIntoFileGraphic->scenePos().y() - item_height);
	rect_bottomright.setX(mFileView->zoomIntoFileGraphic->scenePos().x() + item_width);
	rect_bottomright.setY(mFileView->zoomIntoFileGraphic->scenePos().y() + item_height);
	mFileMiniMap->m_intrect->setRectInScene(rect_topleft, rect_bottomright);

	// show filename
	QString statusmsg;
	statusmsg.clear();
	statusmsg.append("Code view ready. File : ");
	statusmsg.append(editor->m_FileGraphic->Label());
	statusBar()->showMessage(tr(statusmsg.toLatin1()));

}

void SMVMain::showFeatureView()
{
	statusBar()->showMessage(tr("Changing into feature view..."));

	mViewMode = FEATURE_VIEW;
	mCurrentView = (QWidget*)mFeatureView;

	int main_height_parts = this->mFeatureView->frameGeometry().height();

	pagesWidget->setCurrentIndex(0);

	mFeatureView->setForegroundTransparent();
	mFeatureView->setEnabled(true);
	mFeatureView->show();

	Stripe1->setEnabled(true);
	Stripe2->setEnabled(true);
	Stripe3->setEnabled(true);
	Stripe4->setEnabled(true);
	Stripe5->setEnabled(true);
	Stripe6->setEnabled(true);
	Stripe7->setEnabled(true);
	Stripe8->setEnabled(true);
	Stripe9->setEnabled(true);
	Stripe10->setEnabled(true);
	ZPButton->setEnabled(true);

	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	mFeatureMiniMap->showFullScene();

	extrasToolBar->show();
	opacityToolBar->hide();

	mFileView->hide();
	mFileView->setDisabled(true);
	mFileMiniMapDock->hide();
	mFileMiniMap->hide();
	mFileMiniMap->setDisabled(true);

	editor->hide();
	editor->setDisabled(true);
	mCodeMiniMapDock->hide();
	mCodeMiniMap->hide();
	mCodeMiniMap->setDisabled(true);

	mFeatureView->setFocus();
	statusBar()->showMessage(tr("Feature view ready."));
}

void SMVMain::showFileView()
{
	statusBar()->showMessage(tr("Changing into file view..."));

	mViewMode = FILE_VIEW;
	mCurrentView = (QWidget*)mFileView;

	int main_height_parts = this->mFeatureView->frameGeometry().height() / 2;

	extrasToolBar->hide();
	opacityToolBar->hide();

	mFileView->setEnabled(true);
	//mFileView->setForegroundTransparent();
	pagesWidget->setCurrentIndex(1);
	mFileView->showFullSceneInstant();


	mFileView->show();

	Stripe1->setEnabled(true);
	Stripe2->setEnabled(true);
	Stripe3->setEnabled(true);
	Stripe4->setEnabled(true);
	Stripe5->setEnabled(true);
	Stripe6->setEnabled(true);
	Stripe7->setEnabled(true);
	Stripe8->setEnabled(true);
	Stripe9->setEnabled(true);
	Stripe10->setEnabled(true);
	ZPButton->setEnabled(true);

	mFileMiniMap->setEnabled(true);
	mFileMiniMap->show();
	mFileMiniMapDock->setFixedHeight(main_height_parts);
	mFileMiniMapDock->show();

	mFeatureView->hide();
	mFeatureView->setDisabled(true);

	mFeatureViewGraphicData[mFeatureView->filezoom_feature_node].graphics->preview_pixmap_alpha = 0;
	mFeatureViewGraphicData[mFeatureView->filezoom_feature_node].graphics->drawfilescenepreview = false;
	

	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	editor->hide();
	editor->setDisabled(true);
	mCodeMiniMapDock->hide();
	mCodeMiniMap->hide();
	mCodeMiniMap->setDisabled(true);
	
	mFileView->setFocus();
	
	if (feature_mini_map_first_time)
	{
		mFileMiniMap->showFullScene();
		feature_mini_map_first_time = false;
	}

	// set navigator position on zoomed node
	mFeatureMiniMap->centerOn(mFeatureView->zoomIntoFeatureGraphic);

	qreal item_width = mFeatureView->zoomIntoFeatureGraphic->boundingRect().width();
	qreal item_height = mFeatureView->zoomIntoFeatureGraphic->boundingRect().height();

	QPointF rect_topleft, rect_bottomright;
	rect_topleft.setX(mFeatureView->zoomIntoFeatureGraphic->scenePos().x() - item_width);
	rect_topleft.setY(mFeatureView->zoomIntoFeatureGraphic->scenePos().y() - item_height);
	rect_bottomright.setX(mFeatureView->zoomIntoFeatureGraphic->scenePos().x() + item_width);
	rect_bottomright.setY(mFeatureView->zoomIntoFeatureGraphic->scenePos().y() + item_height);
	mFeatureMiniMap->m_intrect->setRectInScene(rect_topleft, rect_bottomright);

	statusBar()->showMessage(tr("File view ready."));

}

void SMVMain::showCodeView()
{
	statusBar()->showMessage(tr("Changing into code view..."));

	mViewMode = CODE_VIEW;
	mCurrentView = (QWidget*)editor;

	int main_height_parts = this->mFeatureView->frameGeometry().height() / 4;

	extrasToolBar->hide();
	opacityToolBar->show();

	editor->setForegroundTransparent();
	editor->setEnabled(true);
	editor->show();

	Stripe1->setDisabled(true);
	Stripe2->setDisabled(true);
	Stripe3->setDisabled(true);
	Stripe4->setDisabled(true);
	Stripe5->setDisabled(true);
	Stripe6->setDisabled(true);
	Stripe7->setDisabled(true);
	Stripe8->setDisabled(true);
	Stripe9->setDisabled(true);
	Stripe10->setDisabled(true);
	ZPButton->setDisabled(true);

	mCodeMiniMap->setEnabled(true);
	mCodeMiniMap->show();
	mCodeMiniMapDock->setFixedHeight(main_height_parts * 2);
	mCodeMiniMapDock->show();
		
	pagesWidget->setCurrentIndex(2);

	mFeatureView->hide();
	mFeatureView->setDisabled(true);
	mFeatureMiniMap->setEnabled(true);
	mFeatureMiniMap->show();
	mFeatureMiniMapDock->setFixedHeight(main_height_parts);
	mFeatureMiniMapDock->show();

	mFileView->hide();
	mFileView->setDisabled(true);
	mFileMiniMap->setEnabled(true);
	mFileMiniMap->show();
	mFileMiniMapDock->setFixedHeight(main_height_parts);
	mFileMiniMapDock->show();

	editor->setFocus();
	editor->createMiniMapPixMap();

	// calculate fragment rectangles
	statusBar()->showMessage(tr("Calculating Fragment Rectangles..."));
	editor->createFragmentRects();
	editor->fadeOutFragments();

	mFileMiniMap->centerOn(mFileView->zoomIntoFileGraphic);

	qreal item_width = mFileView->zoomIntoFileGraphic->boundingRect().width();
	qreal item_height = mFileView->zoomIntoFileGraphic->boundingRect().height();

	QPointF rect_topleft, rect_bottomright;
	rect_topleft.setX(mFileView->zoomIntoFileGraphic->scenePos().x() - item_width);
	rect_topleft.setY(mFileView->zoomIntoFileGraphic->scenePos().y() - item_height);
	rect_bottomright.setX(mFileView->zoomIntoFileGraphic->scenePos().x() + item_width);
	rect_bottomright.setY(mFileView->zoomIntoFileGraphic->scenePos().y() + item_height);
	mFileMiniMap->m_intrect->setRectInScene(rect_topleft, rect_bottomright);

	// show filename
	QString statusmsg;
	statusmsg.clear();
	statusmsg.append("Code view ready. File : ");
	statusmsg.append(editor->m_FileGraphic->Label());
	statusBar()->showMessage(tr(statusmsg.toLatin1()));
}

void SMVMain::stripe0()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe0();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe0();
	}
}
void SMVMain::stripe1()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe1();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe1();
	}
}
void SMVMain::stripe2()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe2();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe2();
	}
}
void SMVMain::stripe3()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe3();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe3();
	}
}
void SMVMain::stripe4()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe4();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe4();
	}
}
void SMVMain::stripe5()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe5();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe5();
	}
}
void SMVMain::stripe6()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe6();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe6();
	}
}
void SMVMain::stripe7()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe7();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe7();
	}
}
void SMVMain::stripe8()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe8();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe8();
	}
}
void SMVMain::stripe9()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe9();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe9();
	}
}
void SMVMain::stripe10()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->stripe10();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->stripe10();
	}
}
void SMVMain::zmButton()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->zmButton();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->zmButton();
	}
	else if (mViewMode == CODE_VIEW)
	{
		editor->setNextWidgetToFadeIn(1);
		editor->fadeOut();
	}
}
void SMVMain::zpButton()
{
	if (mViewMode == FILE_VIEW)
	{
		mFileView->zpButton();
	}
	else if (mViewMode == FEATURE_VIEW)
	{
		mFeatureView->zpButton();
	}
}

// get path of annotations.xml
QString SMVMain::getPathAnnotationsXML()
{
	return path_annotations_xml;
}
// get path of model.m
QString SMVMain::getPathModelM()
{
	return path_model_m;
}
// get path to project folder
QString SMVMain::getProjectPath()
{
	return path_project;
}
// get graphics item of given file view node
FileViewNodeGraphic* SMVMain::getFileViewNodeGraphic(ogdf::node v)
{
	return mFileViewGraphicData[v].graphics;
}
// get graphics item of given file minimap node
FileMiniMapNodeGraphic* SMVMain::getFileMiniMapNodeGraphic(ogdf::node v)
{
	return mFileViewGraphicData[v].graphicsMiniMap;
}
// get graphics item of given feature view node
FeatureViewNodeGraphic* SMVMain::getFeatureViewNodeGraphic(ogdf::node v)
{
	return mFeatureViewGraphicData[v].graphics;
}
// activate given view mode
void SMVMain::setViewMode(ViewMode mode)
{
	mViewMode = mode;
	//TODO: load view ?
}
// get view mode (FEATURE_VIEW, FILE_VIEW, FILE_DETAIL_VIEW, CODE_VIEW)
SMVMain::ViewMode SMVMain::getViewMode()
{
	return mViewMode;
}
// load annotations xml file and create DOM
void SMVMain::loadAnnotationsXML()
{
	if (!path_annotations_xml.isEmpty()) {
		statusBar()->showMessage(tr("Loading annotations..."));
		QFile file(path_annotations_xml);
        if (file.open(QIODevice::ReadOnly)) {
			// load document
            QDomDocument document;
            if (document.setContent(&file)) {
				// create Qt Direct-Object-Model from XML-File
				model = new DomModel(document, this);

				// get the outermost element
				docElem = document.documentElement();
			}
			file.close();
		}
	}
}

// create OGDF-Graph from DOM
// INFO: first element has to be <project name="...">
void SMVMain::createAnnotationsGraph()
{
	statusBar()->showMessage(tr("Creating annotations graph..."));

	// create instance of Graph and GraphAttributes
	mAnnotationsGraph = new ogdf::Graph();
	mAnnotationsGA = new ogdf::GraphAttributes(*mAnnotationsGraph,
				ogdf::GraphAttributes::nodeType | 
				ogdf::GraphAttributes::nodeLabel |
				ogdf::GraphAttributes::nodeGraphics |
				ogdf::GraphAttributes::edgeGraphics |
				ogdf::GraphAttributes::nodeColor |
				ogdf::GraphAttributes::edgeColor |
				ogdf::GraphAttributes::nodeLevel | 
				ogdf::GraphAttributes::edgeStyle | 
				ogdf::GraphAttributes::nodeStyle |
				ogdf::GraphAttributes::nodeTemplate);

	// create arrays for node data
	mNodeType = ogdf::NodeArray<GraphNodeType>(*mAnnotationsGraph, NODETYPE_UNKNOWN);
	mFragmentData = ogdf::NodeArray<FragmentDataItem>(*mAnnotationsGraph);
	mFeatureData = ogdf::NodeArray<unsigned int>(*mAnnotationsGraph);
	mNodeCollapsed = ogdf::NodeArray<bool>(*mAnnotationsGraph);
	mNodeAmbiguityPositions = ogdf::NodeArray<unsigned int>(*mAnnotationsGraph);
	mFileLength = ogdf::NodeArray<int>(*mAnnotationsGraph);
	mFragmentRects = ogdf::NodeArray<std::vector<FragmentRect>>(*mAnnotationsGraph);
	mHistogramRects = ogdf::NodeArray<std::vector<FragmentRect>>(*mAnnotationsGraph);
	mFileViewHistogramTotalLengths = ogdf::NodeArray<int>(*mAnnotationsGraph);
	mFileViewHistogramAbsoluteLengths = ogdf::NodeArray<int>(*mAnnotationsGraph);
	mToolTipLabels = ogdf::NodeArray<QString>(*mAnnotationsGraph);
	mPreviewPixmaps = ogdf::NodeArray<QPixmap>(*mAnnotationsGraph);

	createFeatureList(docElem);
	//printFeatureList();

	if( !docElem.isNull() ) {
		// TODO: Feature-Graphen auch erstellen, wenn das erste Element kein project-Tag ist
		if(docElem.tagName().compare("project", Qt::CaseInsensitive) == 0)
		{
			//create new node
			ogdf::node v = mAnnotationsGraph->newNode();
			// save type of node
			mNodeType[v] = NODETYPE_PROJECT;
			// save name of project
			if (docElem.hasAttributes())
			{
			   QStringList attributes;
			   QString  attribute_string;
			   QDomNamedNodeMap attributeMap = docElem.attributes();

			   // search "name"-tag and save value
			   for (int i = 0; i < attributeMap.count(); ++i) {
				   QDomNode attribute = attributeMap.item(i);
				   if (attribute.nodeName().compare("name",Qt::CaseInsensitive) == 0)
				   {
					   mAnnotationsGA->labelNode(v) = attribute.nodeValue().toLatin1();
				   }
			   }
			}
			// continue creation with next level
			createAnnotationsGraphRecursive(docElem,v);
		}
	}

	// calculate fragment rectangles for each file node
	ogdf::node root = mAnnotationsGraph->firstNode();
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		if (mNodeType[v] == NODETYPE_FILE)
		{
			// get file path
			QString filePath;
			filePath.clear();
			filePath.append(mAnnotationsGA->labelNode(v).cstr());
			ogdf::node parent = ogdf::node(v);
			do
			{
				for(ogdf::adjEntry adj = parent->firstAdj();adj; adj = adj->succ())
				{
					ogdf::edge e = adj->theEdge();
					if (e->target() == parent) // target is this node
					{
						parent = e->source();
						filePath.insert(0,"\\");
						if (parent != root)
							filePath.insert(0,QString(mAnnotationsGA->labelNode(parent).cstr()));
						break;
					}
				}
			} while (parent != root);
			
			// complete file path
			filePath.insert(0,path_project);
			
			// load file and get file length
			QFile file(filePath);
			if (file.open(QFile::ReadOnly | QFile::Text)) {
				QTextStream in(&file);
				QString str = in.readAll();
				mFileLength[v] = str.length();
				file.close();
			}
			else	// error on opening file
			{
				mFileLength[v] = 0;
			}
			int max = 0;
			int sum = 0;
			// try to get length out of fragments
			for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
			{
				ogdf::edge e = adj->theEdge();
				if (e->source() == v)
				{
					if (mNodeType[e->target()] == NODETYPE_FRAGMENT)
					{
						sum = mFragmentData[e->target()].offset + mFragmentData[e->target()].length;
						if (sum > max)
							max = sum;
					}
				}
			}

			// save max length of file
			if (max > mFileLength[v])
				mFileLength[v] = max;

			// calculate fragment rect
			std::vector<FragmentRect> vecFragRect;
			vecFragRect.clear();
						
			// calculate fragments
			for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
			{
				ogdf::edge e = adj->theEdge();
				
				if (e->source() == v) // for each fragment
				{
					ogdf::node t = ogdf::node(e->target());
	
					if (mNodeType[t] == NODETYPE_FRAGMENT)
					{
						std::vector<QColor> vecQColor;
						std::vector<QString> vecNames;
						std::vector<bool>	vecIsActive;
						vecQColor.clear();
						vecNames.clear();

						qreal y_start = qreal(mFragmentData[t].offset) / qreal(mFileLength[v]);
						qreal y_end = qreal(mFragmentData[t].offset
							+ mFragmentData[t].length) / qreal(mFileLength[v]);

						// get colors of features in this fragment
						for(ogdf::adjEntry adjac = t->firstAdj();adjac; adjac = adjac->succ())
						{
							ogdf::edge edg = adjac->theEdge();
							if (edg->source() == t) // fragment is source
							{
								ogdf::node nFeat = edg->target();
								if (mNodeType[nFeat] == NODETYPE_FEATURE)
								{
									vecQColor.push_back(mFeatureList[mFeatureData[nFeat]].color);
									vecNames.push_back(mFeatureList[mFeatureData[nFeat]].name);
									vecIsActive.push_back(true);
								}
							}
						}
						// create fragment rectangle
						FragmentRect fragRect = {mFragmentData[t].offset, mFragmentData[t].length,
							y_start,y_end,vecQColor, vecNames, vecIsActive};
						vecFragRect.push_back(fragRect);
					}
				}
			}
			mFragmentRects[v] = vecFragRect;

			// calculate histograms

			QVector<QString> hist_names;
			QVector<int> hist_lengths;
			QVector<QColor> hist_colors;
			int total_length = 0;
			int absolute_length_fragments = 0;

			// calculate fragments
			for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
			{
				ogdf::edge e = adj->theEdge();
				
				if (e->source() == v) // for each fragment
				{
					bool first_fragment = true;

					ogdf::node t = ogdf::node(e->target());
	
					if (mNodeType[t] == NODETYPE_FRAGMENT)
					{
						// get colors of features in this fragment
						for(ogdf::adjEntry adjac = t->firstAdj();adjac; adjac = adjac->succ())
						{
							ogdf::edge edg = adjac->theEdge();
							if (edg->source() == t) // fragment is source
							{
								ogdf::node nFeat = edg->target();
								if (mNodeType[nFeat] == NODETYPE_FEATURE)
								{
									if (first_fragment == true)		// calculation of relative used space of fragments in file
									{
										first_fragment = false;
										absolute_length_fragments += mFragmentData[t].length;
									}

									if (hist_names.contains(mFeatureList[mFeatureData[nFeat]].name))
									{
										int hist_idx = hist_names.indexOf(mFeatureList[mFeatureData[nFeat]].name);
										hist_lengths[hist_idx] += mFragmentData[t].length;
										total_length += mFragmentData[t].length;
									}
									else
									{
										hist_names.push_back(mFeatureList[mFeatureData[nFeat]].name);
										hist_colors.push_back(mFeatureList[mFeatureData[nFeat]].color);
										hist_lengths.push_back(mFragmentData[t].length);
										total_length += mFragmentData[t].length;
									}
								}
							}
						}
					}
				}
			}
	
			// sort histogram vector
			bool switched;
			do
			{
				switched = false;
				for (int index = 0; index < hist_lengths.size()-1; index++)
				{
					if ( hist_lengths.at(index) < hist_lengths.at(index+1) )	// if fragment a is shorter than b
					{
						// switch position a and b
						QColor temp_color = hist_colors.at(index);
						int temp_length = hist_lengths.at(index);
						QString temp_name = hist_names.at(index);
						
						hist_colors[index] = hist_colors[index+1];
						hist_lengths[index] = hist_lengths[index+1];
						hist_names[index] = hist_names[index+1];

						hist_colors[index+1] = temp_color;
						hist_lengths[index+1] = temp_length;
						hist_names[index+1] = temp_name;

						switched = true;
					}
				}
			}
			while (switched == true);

			std::vector<FragmentRect> histRect;
			histRect.clear();

			qreal hist_length = 0;
			if (total_length > 0)
				hist_length = ( (qreal)mFileLength[v] / (qreal)absolute_length_fragments ) * (qreal)total_length; 

			mFileViewHistogramTotalLengths[v] = total_length;
			mFileViewHistogramAbsoluteLengths[v] = absolute_length_fragments;

			//qDebug() << "used length = " << absolute_length_fragments;
			//qDebug() << "file length = " << mFileLength[v];
			//qDebug() << "total fragments length = " << total_length;
			//qDebug() << "histogram length = " << hist_length;

			// calculate y-coordinates for histogram
			int cumulative_pos = 0;
			qreal y = 0.0;
			if (hist_length > 0.0)
			{
				for (int index = 0; index < hist_lengths.size(); index++)
				{
					int start_pos = cumulative_pos;
					//qreal y_start = qreal(cumulative_pos) / qreal(mFileLength[v]);
					qreal y_start = 0.8*y;	// start a little lower to avoid visual holes
					cumulative_pos += hist_lengths.at(index);
					//qreal y_end = qreal(cumulative_pos) / qreal(mFileLength[v]);
					y = qreal(cumulative_pos) / qreal(hist_length);
					qreal y_end = y;

					std::vector<QColor> vec_hist_color;
					vec_hist_color.push_back(hist_colors.at(index));
					std::vector<QString> vec_hist_name;
					vec_hist_name.push_back(hist_names.at(index));
					std::vector<bool> vec_hist_active;
					vec_hist_active.push_back(true);
					
					// create fragment rectangle
					FragmentRect fragRect = {start_pos, hist_lengths[index],
						y_start,y_end,vec_hist_color, vec_hist_name, vec_hist_active};
					histRect.push_back(fragRect);
				}
			}
			// save histogram fragment
			mHistogramRects[v] = histRect;
		}
	}

	// calculate ambiguity positions

	// initialize
	nodeLabelVector.clear();
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		mNodeAmbiguityPositions[v] = false;
	}
	// get every label in list
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mAnnotationsGA->labelNode(v);
		if (nodeLabel.length() > 0)
			nodeLabelVector.push_back(nodeLabel);
		
	}
	// make every item unique in list
	nodeLabelList.clear();
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelList.push_back(nodeLabelVector[i]);
	}
	nodeLabelList.unique();
	nodeLabelVector.clear();
	std::list<ogdf::String>::iterator it;
	for (it = nodeLabelList.begin(); it != nodeLabelList.end(); it++)
	{
		nodeLabelVector.push_back(*it);
	}
	
	// initialize counts of labels
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelCounts.push_back(0);
	}
	
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mAnnotationsGA->labelNode(v);
		
		// get position
		unsigned int  i;
		for (i = 0; i < nodeLabelVector.size(); i++)
		{
			if (ogdf::String::compare(nodeLabelVector[i],nodeLabel) == 0)
			{
				nodeLabelCounts[i] = nodeLabelCounts[i]+1;
				mNodeAmbiguityPositions[v] = nodeLabelCounts[i];
				break;
			}
		}
		
	}

	// initialize "collapsed"-status
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		mNodeCollapsed[v] = false;
	}

	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		mNodeCollapsed[v] = false;
	
		// set tooltip labels
		if (mNodeType[v] == NODETYPE_PROJECT)
		{
			QString str;
			str.append("Project: ");
			str.append(mAnnotationsGA->labelNode(v).cstr());
			mToolTipLabels[v] = str;
		}
		if (mNodeType[v] == NODETYPE_FILE)
		{
			QString str;
			str.append("File: ");
			str.append(mAnnotationsGA->labelNode(v).cstr());
			QVector<QString> strVector;
			strVector.clear();
			// add features
			ogdf::edge e;
			for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
			{
				e = adj->theEdge();

				if (v == e->source()) // n is parent of fragments
				{
					ogdf::node t = e->target();
					ogdf::edge edg;
					for(ogdf::adjEntry adjac = t->firstAdj();adjac; adjac = adjac->succ())
					{
						edg = adjac->theEdge();
						if (edg->source() == t)  // fragment is source
						{
							if (mNodeType[edg->target()] == NODETYPE_FEATURE)
							{
								QString tempstr(mFeatureList[mFeatureData[edg->target()]].name.toLatin1());
								if (!strVector.contains(tempstr))
									strVector.push_back(mFeatureList[mFeatureData[edg->target()]].name );
							}
						}
					}
				}
			}
			if (strVector.size() > 0)
			{
				str.append("\n\n");
				str.append("Features:");
				for (int i = 0; i < strVector.size(); i++)
				{
					str.append("\n");
					str.append(strVector[i]);
				}
			}
			else
			{
				str.append("\n\n");
				str.append("(contains no features)");
			}
			
			mToolTipLabels[v] = str;
		}
		if (mNodeType[v] == NODETYPE_FOLDER)
		{
			QString str;
			str.append("Folder: ");
			str.append(mAnnotationsGA->labelNode(v).cstr());
			mToolTipLabels[v] = str;
		}
	}
}

// create file graph for given DOM-Element and given OGDF-Parent Node
void SMVMain::createAnnotationsGraphRecursive(QDomElement docElem, ogdf::node parent)
{
	QDomNode n = docElem.firstChild();
	while( !n.isNull() ) {
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if( !e.isNull() ) {
			ogdf::node v = 0;
			if(e.tagName().compare("project", Qt::CaseInsensitive) == 0)
			{
				//create new node
				v = mAnnotationsGraph->newNode();
				// save type of node
				mNodeType[v] = NODETYPE_PROJECT;
				// save name of project
				if (e.hasAttributes())
				{
				   QStringList attributes;
				   QString  attribute_string;
				   QDomNamedNodeMap attributeMap = e.attributes();

				   for (int i = 0; i < attributeMap.count(); ++i) {
					   QDomNode attribute = attributeMap.item(i);
					   if (attribute.nodeName().compare("name",Qt::CaseInsensitive) == 0)
					   {
						   mAnnotationsGA->labelNode(v) = attribute.nodeValue().toLatin1();
					   }
				   }
				}
				// create edge between this node and parent node
				ogdf::edge e = mAnnotationsGraph->newEdge(parent,v);
			}
			else if(e.tagName().compare("file", Qt::CaseInsensitive) == 0)
			{
				//create new node
				v = mAnnotationsGraph->newNode();
				// save type of node
				mNodeType[v] = NODETYPE_FILE;
				// save name of file
				if (e.hasAttributes())
				{
				   QStringList attributes;
				   QString  attribute_string;
				   QDomNamedNodeMap attributeMap = e.attributes();

				   for (int i = 0; i < attributeMap.count(); ++i) {
					   QDomNode attribute = attributeMap.item(i);
					   if (attribute.nodeName().compare("name",Qt::CaseInsensitive) == 0)
					   {
						   mAnnotationsGA->labelNode(v) = attribute.nodeValue().toLatin1();
					   }
				   }
				}
				// create edge between this node and parent node
				ogdf::edge e = mAnnotationsGraph->newEdge(parent,v);
			}
			else if(e.tagName().compare("folder", Qt::CaseInsensitive) == 0)
			{
				//create new node
				v = mAnnotationsGraph->newNode();
				// save type of node
				mNodeType[v] = NODETYPE_FOLDER;
				// save name of folder
				if (e.hasAttributes())
				{
				   QStringList attributes;
				   QString  attribute_string;
				   QDomNamedNodeMap attributeMap = e.attributes();

				   for (int i = 0; i < attributeMap.count(); ++i) {
					   QDomNode attribute = attributeMap.item(i);
					   if (attribute.nodeName().compare("name",Qt::CaseInsensitive) == 0)
					   {
						   mAnnotationsGA->labelNode(v) = attribute.nodeValue().toLatin1();
					   }
				   }
				}
				// create edge between this node and parent node
				ogdf::edge e = mAnnotationsGraph->newEdge(parent,v);
			}
			else if(e.tagName().compare("fragment", Qt::CaseInsensitive) == 0)
			{
				//create new node
				v = mAnnotationsGraph->newNode();
				// save type of node
				mNodeType[v] = NODETYPE_FRAGMENT;
				// save fragment data
				if (e.hasAttributes())
				{
				   QStringList attributes;
				   QString  attribute_string;
				   QDomNamedNodeMap attributeMap = e.attributes();

				   unsigned int length, offset;

				   for (int i = 0; i < attributeMap.count(); ++i) {
					   QDomNode attribute = attributeMap.item(i);
					   if (attribute.nodeName().compare("length",Qt::CaseInsensitive) == 0)
					   {
						   length = attribute.nodeValue().toInt();
					   }
					   else if (attribute.nodeName().compare("offset",Qt::CaseInsensitive) == 0)
					   {
						   offset = attribute.nodeValue().toInt();
					   }
				   }
				   FragmentDataItem frag = {length, offset};
				   mFragmentData[v] = frag;
				}
				// create edge between this node and parent node
				ogdf::edge e = mAnnotationsGraph->newEdge(parent,v);
			}
			else if(e.tagName().compare("feature", Qt::CaseInsensitive) == 0)
			{
				//create new node
				v = mAnnotationsGraph->newNode();
				// save type of node
				mNodeType[v] = NODETYPE_FEATURE;
				// save id of feature
				for(unsigned int i = 0; i<mFeatureList.size(); i++) 
				{
					if(e.text().compare(mFeatureList[i].name) == 0)
						mFeatureData[v] = i;
				}
				// create edge between this node and parent node
				ogdf::edge e = mAnnotationsGraph->newEdge(parent,v);
			}
			if (v != 0)
				createAnnotationsGraphRecursive(e,v);
			else createAnnotationsGraphRecursive(e,parent);
		}
		n = n.nextSibling();
	}
}

// search XML-Tree of annotations.xml for features and save them into feature list
void SMVMain::createFeatureList(QDomElement docElem)
{
	QDomNode n = docElem.firstChild();
	while( !n.isNull() ) {
       QDomElement e = n.toElement(); // try to convert the node to an element.
       if( !e.isNull() ) {
			if(e.tagName().compare("feature", Qt::CaseInsensitive) == 0)
			{
				// check, if this feature yet exists in feature list
				bool feature_existing = false;
				QString name = e.text();
				if (mFeatureList.size() > 0)
				{
					for (unsigned int i=0; i < mFeatureList.size(); i++)
					{
						if (mFeatureList[i].name.compare(e.text(),Qt::CaseInsensitive) == 0)
							feature_existing = true;
					}
				}
				// add feature, if it is not existing
				if (!feature_existing)
				{
					// save feature name
					Feature newfeature = {e.text(),QColor(qrand() % 256, qrand() % 256, qrand() % 256),true,true};
					// add feature to feature list
					mFeatureList.push_back(newfeature);
				}
			}
			createFeatureList(e);
       }
       n = n.nextSibling();
   }
}

// create file graph
void SMVMain::createFileGraph()
{
	statusBar()->showMessage(tr("Clearing old view..."));
	// clear old view
	for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
	{
		mFileView->scene()->removeItem(mFileViewEdgeGraphicData[e].graphics);
		mFileMiniMap->scene()->removeItem(mFileViewEdgeGraphicData[e].graphicsMiniMap);
	}

	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileView->scene()->removeItem(mFileViewGraphicData[n].graphics);
		mFileMiniMap->scene()->removeItem(mFileViewGraphicData[n].graphicsMiniMap);
	}

	QList<QGraphicsItem*> list = mFileView->scene()->items();
	for (int i = 0; i < list.size(); ++i) {
		mFileView->scene()->removeItem(list.at(i));
	}

	QList<QGraphicsItem*> listMiniMap = mFileMiniMap->scene()->items();
	for (int i = 0; i < list.size(); ++i) {
		// dont delete rect!!!
		if (list.at(i) != (QGraphicsItem*)mFileMiniMap->m_intrect)
			mFileMiniMap->scene()->removeItem(list.at(i));
	}

	mFileView->scene()->clear();
	//scene of mFileMiniMap must not be cleared, because of navigator
	
	mFileGraph.clear();

	statusBar()->showMessage(tr("Creating file graph..."));

	// create copy of complete graph
	mFileGraph = ogdf::Graph(*mAnnotationsGraph);
	
	// create node array for node types
	mFileNodeType = ogdf::NodeArray<GraphNodeType>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		switch (mNodeType[v])
		{
			case NODETYPE_PROJECT:
				mFileNodeType[v] = NODETYPE_PROJECT;
				break;
			case NODETYPE_FILE:
				mFileNodeType[v] = NODETYPE_FILE;
				break;
			case NODETYPE_FOLDER:
				mFileNodeType[v] = NODETYPE_FOLDER;
				break;
			case NODETYPE_FRAGMENT:
				mFileNodeType[v] = NODETYPE_FRAGMENT;
				break;
			case NODETYPE_FEATURE:
				mFileNodeType[v] = NODETYPE_FEATURE;
				break;
			case NODETYPE_UNKNOWN:
				mFileNodeType[v] = NODETYPE_UNKNOWN;
				break;
		}
	}

	mFileGA = ogdf::GraphAttributes(mFileGraph, ogdf::GraphAttributes::nodeType | 
				ogdf::GraphAttributes::nodeLabel |
				ogdf::GraphAttributes::nodeGraphics |
				ogdf::GraphAttributes::edgeGraphics |
				ogdf::GraphAttributes::nodeColor |
				ogdf::GraphAttributes::edgeColor |
				ogdf::GraphAttributes::nodeLevel | 
				ogdf::GraphAttributes::edgeStyle | 
				ogdf::GraphAttributes::nodeStyle |
				ogdf::GraphAttributes::nodeTemplate);
	
	// delete fragment nodes, feature nodes and unknown nodes in file graph
	ogdf::node w = 0;
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}

		if (!((mFileNodeType[v] == NODETYPE_PROJECT) ||
			(mFileNodeType[v] == NODETYPE_FILE) ||
			(mFileNodeType[v] == NODETYPE_FOLDER)))
		{
			w = v;
		}
	}
	if (w != 0) 
	{
		mFileGraph.delNode(w);
		w = 0;
	}

	// create node array for collapse status
	mFileNodeCollapsed = ogdf::NodeArray<bool>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mNodeCollapsed[v] == true)
			mFileNodeCollapsed[v] = true;
		else mFileNodeCollapsed[v] = false;
	}

	// init node array for active/inactive status
	mFileNodeActive = ogdf::NodeArray<bool>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mFileNodeType[v] == NODETYPE_FOLDER)	// folder are deactivated initially
			mFileNodeActive[v] = false;
		else mFileNodeActive[v] = true;				// other types (project, files) are activated
	}

	// copy labels
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileGA.labelNode(v) = mAnnotationsGA->labelNode(v).cstr();
	}

	// copy node ambiguity positions
	mFileNodeAmbiguityPositions = ogdf::NodeArray<int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileNodeAmbiguityPositions[v] = mNodeAmbiguityPositions[v];
	}

	// copy fragment rectangles
	mFileViewFragmentRects = ogdf::NodeArray<std::vector<FragmentRect>>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFragmentRects[v] = mFragmentRects[v];
	}
	
	// copy histogram rectangles
	mFileViewHistogramRects = ogdf::NodeArray<std::vector<FragmentRect>>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewHistogramRects[v] = mHistogramRects[v];
	}

	// delete nodes which aren't needed

	// create an array for nodes to be deleted
	mFileNodeToBeDeleted = ogdf::NodeArray<bool>(mFileGraph);

	// initialize node array, no nodes are deleted by default
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileNodeToBeDeleted[n] = false;
	}

	// mark all file nodes as "to be deleted", which have no active features or which do not content selected features
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		// check if file has fragments with activated features
		if (mFileNodeType[n] == NODETYPE_FILE)
		{
			bool canBeDeleted = true;
			std::vector<FragmentRect> rects = mFileViewFragmentRects[n];
			// search fragment rects
			for (unsigned int i = 0; i < rects.size(); i++)
			{
				// search feature names of fragments
				for (unsigned int j = 0; j < rects[i].names.size(); j++)
				{
					// search feature list
					for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
					{
						// if feature is enabled
						if (mFeatureNodeSelected[v] == true)
						{
							// if feature name in feature list == feature name in fragment rect
							if (QString::compare(mFeatureGA->labelNode(v).cstr(),rects[i].names[j]) == 0)
							{
								canBeDeleted = false;
								break;
							}
						}
					}
				}
			}
			// mark node as "to be deleted"
			if (canBeDeleted == true)
			{
				// special case, node musn't be deleted
				if (!((mFileNodeCollapsed[n] == true) && (mFileNodeActive[n] == true)))
				{
					if (showFullFileGraphAction->isChecked())
					{
						mFileNodeActive[n] = false;
					}
					else
					{
						mFileNodeToBeDeleted[n] = true;
					}
				}
			}
		}
	}

	// now (!!!): activate folders with active files
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (mFileNodeActive[n] == true)
			activateParent(n);
	}
	
	// now we can delete marked file nodes
	w = 0;
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}
		if (mFileNodeToBeDeleted[n] == true)
		{
			w = n;
		}
	}
	if (w != 0) 
	{
		mFileGraph.delNode(w);
		w = 0;
	}

	if (!showFullFileGraphAction->isChecked())	// delete empty folders
	{
		bool nodeMarked;
		do
		{
			nodeMarked = false;
			for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
			{
				// check if folder node has file children
				if (mFileNodeType[n] == NODETYPE_FOLDER)
				{
					//qDebug(mFileGA.labelNode(n).cstr());
					if (mFileNodeToBeDeleted[n] == false)
					{
						bool noChildNodesFound = true;
						// search nodes under this node and delete them
						for(ogdf::adjEntry adj = n->firstAdj();adj; adj = adj->succ())
						{
							ogdf::edge e = adj->theEdge();
							if ((e->source() == n) && (mFileNodeToBeDeleted[e->target()] == false)) // source is this node
							{
								noChildNodesFound = false;		// no deleting, because no children found
								break;
							}
						}
						if (noChildNodesFound == true)
						{
							mFileNodeToBeDeleted[n] = true;
							nodeMarked = true;
						}
					}

				}
			}
		} while (nodeMarked == true);

		// delete marked folder nodes
		w = 0;
		for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
		{
			if (w != 0) 
			{
				mFileGraph.delNode(w);
				w = 0;
			}
			if (mFileNodeToBeDeleted[n] == true)
			{
				w = n;
			}
		}
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}
	}

	// copy fragment data
	mFileViewFragmentData = ogdf::NodeArray<FragmentDataItem>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFragmentData[v] = mFragmentData[v];
	}

	// copy feature data
	mFileViewFeatureData = ogdf::NodeArray<unsigned int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFeatureData[v] = mFeatureData[v];
	}

	// copy file length
	mFileViewFileLength = ogdf::NodeArray<int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFileLength[v] = mFileLength[v];
	}

	// copy tooltip labels
	mFileToolTipLabels = ogdf::NodeArray<QString>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileToolTipLabels[v] = mToolTipLabels[v];
	}

	// copy pixmaps
	mFileViewPreviewPixmaps = ogdf::NodeArray<QPixmap>(mPreviewPixmaps);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewPreviewPixmaps[v] = mPreviewPixmaps[v];
	}
	
	// create graphics for file view

	mFileViewGraphicData = ogdf::NodeArray<FileViewNode>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		//qDebug() << mFileGA.labelNode(v).cstr();
		createFileViewNodeGraphic(v);
	}

	// update pixmaps, if colors have changed
	if ( filepreviewscreated == false )
	{
		filenodesneedupdate = true;
		this->repaint();
	}
	else
	{
		for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
		{
			if (mFileViewGraphicData[v].graphics->Type() == 1)
			{
				//preview_pm = createCodePreviewPixmap(mFileViewGraphicData[v].graphics);
				if ((mFileViewPreviewPixmaps[v].size().width() > 0) && (mFileViewPreviewPixmaps[v].size().height() > 0))
				{
					//qDebug() << "saving pixmap (file view)...";
					mFileViewGraphicData[v].graphics->setNodePixmap(mFileViewPreviewPixmaps[v]);
				}
			}

			if (mFileViewGraphicData[v].graphicsMiniMap->Type() == 1)
			{
				if ((mFileViewPreviewPixmaps[v].size().width() > 0) && (mFileViewPreviewPixmaps[v].size().height() > 0))
				{
					//qDebug() << "saving pixmap (file mini map)...";
					mFileViewGraphicData[v].graphicsMiniMap->setNodePixmap(mFileViewPreviewPixmaps[v]);
				}
			}
		}
	}

	// calculate hierarchical layout
	mFileViewLayout = HIERARCHICAL;
	ogdf::SugiyamaLayout SL;
	SL.setRanking(new ogdf::OptimalRanking);
	SL.setCrossMin(new ogdf::MedianHeuristic);
	ogdf::OptimalHierarchyLayout *ohl = new ogdf::OptimalHierarchyLayout;
	ohl->layerDistance(10.0);
	ohl->nodeDistance(10.0);
	ohl->weightBalancing(0.8);
	SL.setLayout(ohl);
	SL.call(mFileGA);

	// update BoundingBox
	mFileView->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
		mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
		mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
		mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

	mFileMiniMap->scene()->setSceneRect(mFileGA.boundingBox().p1().m_x - GRAPH_MARGIN,
		mFileGA.boundingBox().p1().m_y - GRAPH_MARGIN,
		mFileGA.boundingBox().p2().m_x + 2 * GRAPH_MARGIN,
		mFileGA.boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

	//update positions
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
		mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));
		mFileViewGraphicData[v].graphics->update(mFileViewGraphicData[v].graphics->boundingRect());

		mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
		mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
		mFileViewGraphicData[v].graphicsMiniMap->update(mFileViewGraphicData[v].graphicsMiniMap->boundingRect());
	}

	// create edge graphics for each edge of file graph
	mFileViewEdgeGraphicData = ogdf::EdgeArray<FileViewLinearEdge>(mFileGraph);
	for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
	{
		createFileViewEdgeGraphic(e);
	}

	//scale view
	mFileView->showFullScene();
	mFileMiniMap->showFullScene();

	// create file tree view
	strFileTree.clear();

	// create string of file view
	ogdf::node n = mFileGraph.firstNode();
	strFileTree.append(mFileGA.labelNode(n).cstr());
	strFileTree.append("\n");

	for(ogdf::adjEntry adj = n->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->source() == n) // source is this node
		{
			createFileGraphString(e->target(), 1);
		}
	}
	
	// create model
	mFileTreeViewModel = new FileTreeModel(strFileTree);
	mFileTreeView->setModel(mFileTreeViewModel);
	mFileTreeView->expandAll();

	statusBar()->showMessage(tr("Ready."));

	//printFileTreeViewModel();			// debug
}

// recursive function to create string of file tree
void SMVMain::createFileGraphString(ogdf::node v, int level)
{
	for (int i = 0; i < level; i++)
	{
		strFileTree.append("    ");
	}
	strFileTree.append(mFileGA.labelNode(v).cstr());
	strFileTree.append("\n");
	// search nodes under this node and delete them
	for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->source() == v) // source is this node
		{
			createFileGraphString(e->target(), level+1);
		}
	}
}

// create FileViewNodeGraphics for given node
void SMVMain::createFileViewNodeGraphic(ogdf::node v)
{
	// create graphics object
	mFileViewGraphicData[v].graphics = new FileViewNodeGraphic(this);
	
	// set properties
	mFileViewGraphicData[v].graphics->setLabel(mFileGA.labelNode(v).cstr());
	mFileViewGraphicData[v].graphics->setFileGraphNode(v);

	// TODO: minimal height or width for certain node classes ?
	
	if (mFileNodeType[v] == NODETYPE_PROJECT)
	{
		mFileViewGraphicData[v].graphics->setType(NODETYPE_PROJECT);
	}
	else if (mFileNodeType[v] == NODETYPE_FOLDER)
	{
		mFileViewGraphicData[v].graphics->setType(NODETYPE_FOLDER);
	}
	else if (mFileNodeType[v] == NODETYPE_FILE)
	{
		mFileViewGraphicData[v].graphics->setType(NODETYPE_FILE);
	}

	if (mFileNodeActive[v] == true)
		mFileViewGraphicData[v].graphics->setGreyed(false);
	else
	{
		mFileViewGraphicData[v].graphics->setGreyed(true);	// graphics greyed
	}

	mFileViewGraphicData[v].graphics->setObjectWidth(mFileGA.width(v));
	mFileViewGraphicData[v].graphics->setObjectHeight(mFileGA.height(v));

	mFileGA.x(v) = 0;
	mFileGA.y(v) = 0;

	mFileViewGraphicData[v].graphics->setX(mFileGA.x(v));
	mFileViewGraphicData[v].graphics->setY(mFileGA.y(v));

	mFileViewGraphicData[v].graphics->calculateDimensions();
	mFileViewGraphicData[v].graphics->updateLOD();

	mFileGA.width(v) = double(mFileViewGraphicData[v].graphics->getObjectWidth());
	mFileGA.height(v) = double(mFileViewGraphicData[v].graphics->getObjectHeight());

	mFileView->scene()->addItem(mFileViewGraphicData[v].graphics);

	if (mFileNodeType[v] == NODETYPE_FILE)
	{
		for (unsigned int i = 0; i < mFileViewFragmentRects[v].size(); i++)
		{
			mFileViewGraphicData[v].graphics->createFragmentRects(
				mFileViewFragmentRects[v][i].offset,
				mFileViewFragmentRects[v][i].length,
				mFileViewFragmentRects[v][i].y_start,
				mFileViewFragmentRects[v][i].y_end,
				mFileViewFragmentRects[v][i].colors,
				mFileViewFragmentRects[v][i].names,
				mFileViewFragmentRects[v][i].isActive);
		}
	}

	if (mFileNodeType[v] == NODETYPE_FILE)
	{
		for (unsigned int i = 0; i < mFileViewHistogramRects[v].size(); i++)
		{
			mFileViewGraphicData[v].graphics->createHistogramRects(
				mFileViewHistogramRects[v][i].offset,
				mFileViewHistogramRects[v][i].length,
				mFileViewHistogramRects[v][i].y_start,
				mFileViewHistogramRects[v][i].y_end,
				mFileViewHistogramRects[v][i].colors,
				mFileViewHistogramRects[v][i].names,
				mFileViewHistogramRects[v][i].isActive);
		}
	}
	
	mFileViewGraphicData[v].graphics->setToolTipContent(mFileToolTipLabels[v]);

	// create node graphic for mini map
	mFileViewGraphicData[v].graphicsMiniMap = new FileMiniMapNodeGraphic(this);
	mFileViewGraphicData[v].graphicsMiniMap->setLabel(mFileViewGraphicData[v].graphics->Label());
	mFileViewGraphicData[v].graphicsMiniMap->setFileGraphNode(v);
	mFileViewGraphicData[v].graphicsMiniMap->setType(mFileViewGraphicData[v].graphics->Type());
	if (mFileNodeActive[v] == true)
		mFileViewGraphicData[v].graphicsMiniMap->setGreyed(false);
	else
	{
		mFileViewGraphicData[v].graphicsMiniMap->setGreyed(true);	// node now greyed
	}
	mFileViewGraphicData[v].graphicsMiniMap->setObjectWidth(mFileGA.width(v));
	mFileViewGraphicData[v].graphicsMiniMap->setObjectHeight(mFileGA.height(v));
	mFileViewGraphicData[v].graphicsMiniMap->setX(mFileGA.x(v));
	mFileViewGraphicData[v].graphicsMiniMap->setY(mFileGA.y(v));
	mFileViewGraphicData[v].graphicsMiniMap->calculateDimensions();
	mFileViewGraphicData[v].graphicsMiniMap->updateLOD();
	mFileMiniMap->scene()->addItem(mFileViewGraphicData[v].graphicsMiniMap);
	if (mFileNodeType[v] == NODETYPE_FILE)
	{
		for (unsigned int i = 0; i < mFileViewFragmentRects[v].size(); i++)
		{
			mFileViewGraphicData[v].graphicsMiniMap->createFragmentRects(
				mFileViewFragmentRects[v][i].offset,
				mFileViewFragmentRects[v][i].length,
				mFileViewFragmentRects[v][i].y_start,
				mFileViewFragmentRects[v][i].y_end,
				mFileViewFragmentRects[v][i].colors,
				mFileViewFragmentRects[v][i].names,
				mFileViewFragmentRects[v][i].isActive);
		}
	}
	if (mFileNodeType[v] == NODETYPE_FILE)
	{
		for (unsigned int i = 0; i < mFileViewHistogramRects[v].size(); i++)
		{
			mFileViewGraphicData[v].graphicsMiniMap->createHistogramRects(
				mFileViewHistogramRects[v][i].offset,
				mFileViewHistogramRects[v][i].length,
				mFileViewHistogramRects[v][i].y_start,
				mFileViewHistogramRects[v][i].y_end,
				mFileViewHistogramRects[v][i].colors,
				mFileViewHistogramRects[v][i].names,
				mFileViewHistogramRects[v][i].isActive);
		}
	}
	mFileViewGraphicData[v].graphicsMiniMap->setToolTipContent(mFileToolTipLabels[v]);
	
}

// create FileViewNodeGraphics for given node
void SMVMain::createFileViewEdgeGraphic(ogdf::edge e)
{
	// set coordinates
	int s_x = mFileViewGraphicData[e->source()].graphics->pos().x();
	int s_y = mFileViewGraphicData[e->source()].graphics->pos().y()+(mFileViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	int t_x = mFileViewGraphicData[e->target()].graphics->pos().x();
	int t_y = mFileViewGraphicData[e->target()].graphics->pos().y()-(mFileViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	QPoint s(s_x, s_y);
	QPoint t(t_x, t_y);

	// create graphics object
	mFileViewEdgeGraphicData[e].graphics = new LinearEdgeGraphic(this, s, t);
	mFileView->scene()->addItem(mFileViewEdgeGraphicData[e].graphics);

	// create edge graphic for mini map
	mFileViewEdgeGraphicData[e].graphicsMiniMap = new LinearEdgeGraphic(this, s, t);
	mFileMiniMap->scene()->addItem(mFileViewEdgeGraphicData[e].graphicsMiniMap);
}
// update position, triggered from file view node
void SMVMain::updateFileViewNodePosition(ogdf::node v)
{
	if ((mFileGA.x(v) != mFileViewGraphicData[v].graphics->pos().x())||(mFileGA.y(v) != mFileViewGraphicData[v].graphics->pos().y()))
	{
		mFileGA.x(v) = mFileViewGraphicData[v].graphics->pos().x();
		mFileGA.y(v) = mFileViewGraphicData[v].graphics->pos().y();
		mFileViewGraphicData[v].graphicsMiniMap->setPos(mFileViewGraphicData[v].graphics->pos());

		for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
		{
			updateFileViewEdgePosition(adj->theEdge());
		}
	}
}
// update position, triggered from file minimap node
void SMVMain::updateFileMiniMapNodePosition(ogdf::node v)
{
	if ((mFileGA.x(v) != mFileViewGraphicData[v].graphicsMiniMap->pos().x())||(mFileGA.y(v) != mFileViewGraphicData[v].graphicsMiniMap->pos().y()))
	{
		mFileGA.x(v) = mFileViewGraphicData[v].graphicsMiniMap->pos().x();
		mFileGA.y(v) = mFileViewGraphicData[v].graphicsMiniMap->pos().y();
		mFileViewGraphicData[v].graphics->setPos(mFileViewGraphicData[v].graphicsMiniMap->pos());

		for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
		{
			updateFileViewEdgePosition(adj->theEdge());
		}
	}
}
// update position of given file view edge
void SMVMain::updateFileViewEdgePosition(ogdf::edge e)
{
	//get old bounding rect
	QRectF oldBounds = mFileViewEdgeGraphicData[e].graphics->boundingRect();
	// set coordinates
	qreal s_x = mFileViewGraphicData[e->source()].graphics->pos().x();
	qreal s_y = mFileViewGraphicData[e->source()].graphics->pos().y()+(mFileViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	qreal t_x = mFileViewGraphicData[e->target()].graphics->pos().x();
	qreal t_y = mFileViewGraphicData[e->target()].graphics->pos().y()-(mFileViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	QPointF s(s_x, s_y);
	mFileViewEdgeGraphicData[e].graphics->setSource(s);
	QPointF t(t_x, t_y);
	mFileViewEdgeGraphicData[e].graphics->setTarget(t);
	mFileViewEdgeGraphicData[e].graphics->update(mFileViewEdgeGraphicData[e].graphics->boundingRect());

	mFileViewEdgeGraphicData[e].graphicsMiniMap->setSource(s);
	mFileViewEdgeGraphicData[e].graphicsMiniMap->setTarget(t);
	mFileViewEdgeGraphicData[e].graphicsMiniMap->update(mFileViewEdgeGraphicData[e].graphicsMiniMap->boundingRect());
}

// recreate file view on collapsing/folding
void SMVMain::collapseFileViewNode()
{
	statusBar()->showMessage(tr("Clearing old view..."));
	for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
	{
		mFileView->scene()->removeItem(mFileViewEdgeGraphicData[e].graphics);
		mFileMiniMap->scene()->removeItem(mFileViewEdgeGraphicData[e].graphicsMiniMap);
	}

	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileView->scene()->removeItem(mFileViewGraphicData[n].graphics);
		mFileMiniMap->scene()->removeItem(mFileViewGraphicData[n].graphicsMiniMap);
	}

	QList<QGraphicsItem*> list = mFileView->scene()->items();
	 for (int i = 0; i < list.size(); ++i) {
		 mFileView->scene()->removeItem(list.at(i));
	 }

	 QList<QGraphicsItem*> listMiniMap = mFileMiniMap->scene()->items();
	 for (int i = 0; i < list.size(); ++i) {
		// dont delete rect!!!
		if (list.at(i) != (QGraphicsItem*)mFileMiniMap->m_intrect)
			mFileMiniMap->scene()->removeItem(list.at(i));
	 }

	mFileView->scene()->clear();
	
	mFileGraph.clear();

	// recreate view
	statusBar()->showMessage(tr("Recreate file view..."));
	
	// get new copy of complete graph
	mFileGraph = ogdf::Graph(*mAnnotationsGraph);

	// create node array for collapse status
	mFileNodeCollapsed = ogdf::NodeArray<bool>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mNodeCollapsed[v] == true)
			mFileNodeCollapsed[v] = true;
		else mFileNodeCollapsed[v] = false;
	}

	// create node array for node types
	mFileNodeType = ogdf::NodeArray<GraphNodeType>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		switch (mNodeType[v])
		{
			case NODETYPE_PROJECT:
				mFileNodeType[v] = NODETYPE_PROJECT;
				break;
			case NODETYPE_FILE:
				mFileNodeType[v] = NODETYPE_FILE;
				break;
			case NODETYPE_FOLDER:
				mFileNodeType[v] = NODETYPE_FOLDER;
				break;
			case NODETYPE_FRAGMENT:
				mFileNodeType[v] = NODETYPE_FRAGMENT;
				break;
			case NODETYPE_FEATURE:
				mFileNodeType[v] = NODETYPE_FEATURE;
				break;
			case NODETYPE_UNKNOWN:
				mFileNodeType[v] = NODETYPE_UNKNOWN;
				break;
		}
	}

	// create graph attributes for file graph
	mFileGA = ogdf::GraphAttributes(mFileGraph, ogdf::GraphAttributes::nodeType | 
				ogdf::GraphAttributes::nodeLabel |
				ogdf::GraphAttributes::nodeGraphics |
				ogdf::GraphAttributes::edgeGraphics |
				ogdf::GraphAttributes::nodeColor |
				ogdf::GraphAttributes::edgeColor |
				ogdf::GraphAttributes::nodeLevel | 
				ogdf::GraphAttributes::edgeStyle | 
				ogdf::GraphAttributes::nodeStyle |
				ogdf::GraphAttributes::nodeTemplate);

	// delete fragment nodes, feature nodes and unknown nodes in file graph
	ogdf::node w = 0;
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}

		if (!((mFileNodeType[n] == NODETYPE_PROJECT) ||
			(mFileNodeType[n] == NODETYPE_FILE) ||
			(mFileNodeType[n] == NODETYPE_FOLDER)))
		{
			w = n;
		}
	}
	if (w != 0) 
	{
		mFileGraph.delNode(w);
		w = 0;
	}

	// get node labels
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileGA.labelNode(n) = mAnnotationsGA->labelNode(n).cstr();
	}

	// init node array for active/inactive status
	mFileNodeActive = ogdf::NodeArray<bool>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mFileNodeType[v] == NODETYPE_FOLDER)	// folder are deactivated initially
			mFileNodeActive[v] = false;
		else mFileNodeActive[v] = true;				// other types (project, files) are activated
	}

	// copy node ambiguity positions
	mFileNodeAmbiguityPositions = ogdf::NodeArray<int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileNodeAmbiguityPositions[v] = mNodeAmbiguityPositions[v];
	}

	// copy fragment rectangles
	mFileViewFragmentRects = ogdf::NodeArray<std::vector<FragmentRect>>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFragmentRects[v] = mFragmentRects[v];
	}

	// copy histogram rectangles
	mFileViewHistogramRects = ogdf::NodeArray<std::vector<FragmentRect>>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewHistogramRects[v] = mHistogramRects[v];
	}

	// deactivate rectangles, which are disabled
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFileViewFragmentRects[v].size(); i++)
		{
			for (unsigned int j = 0; j < mFileViewFragmentRects[v][i].names.size(); j++)
			{
				for (unsigned int k = 0; k < mFeatureList.size(); k++)
				{
					if (mFeatureList[k].isActive == false)
					{
						if (QString::compare(mFeatureList[k].name, mFileViewFragmentRects[v][i].names[j]) == 0)
						{
							mFileViewFragmentRects[v][i].isActive[j] = false;	// feature disabled
						}
					}
				}
			}
		}
	}
	// deactivate rectangles, which are disable
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFileViewHistogramRects[v].size(); i++)
		{
			for (unsigned int j = 0; j < mFileViewHistogramRects[v][i].names.size(); j++)
			{
				for (unsigned int k = 0; k < mFeatureList.size(); k++)
				{
					if (mFeatureList[k].isActive == false)
					{
						if (QString::compare(mFeatureList[k].name, mFileViewHistogramRects[v][i].names[j]) == 0)
						{
							mFileViewHistogramRects[v][i].isActive[j] = false;	// feature disabled
						}
					}
				}
			}
		}
	}


	// create an array for nodes to be deleted
	mFileNodeToBeDeleted = ogdf::NodeArray<bool>(mFileGraph);

	// initialize node array, no nodes are deleted by default
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		mFileNodeToBeDeleted[n] = false;
	}

	// mark all nodes as "to be deleted" under collapsed nodes
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (mFileNodeCollapsed[n] == true)
		{
			for(ogdf::adjEntry adj = n->firstAdj();adj; adj = adj->succ())
			{
				ogdf::edge e = adj->theEdge();
				if (e->source() == n) // source is this node
				{
					deleteFileViewSubGraph(e->target());
				}
			}
		}
	}
	
	// mark all file nodes as "to be deleted", which have no active features or which do not content selected features
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		// check if file has fragments with activated features
		if (mFileNodeType[n] == NODETYPE_FILE)
		{
			bool canBeDeleted = true;
			std::vector<FragmentRect> rects = mFileViewFragmentRects[n];
			// search fragment rects
			for (unsigned int i = 0; i < rects.size(); i++)
			{
				// search feature names of fragments
				for (unsigned int j = 0; j < rects[i].names.size(); j++)
				{
					// search feature list
					for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
					{
						// if feature is enabled
						if (mFeatureNodeSelected[v] == true)
						{
							// if feature name in feature list == feature name in fragment rect
							if (QString::compare(mFeatureGA->labelNode(v).cstr(),rects[i].names[j]) == 0)
							{
								canBeDeleted = false;
								break;
							}
						}
					}
				}
			}
			// mark node as "to be deleted"
			if (canBeDeleted == true)
			{
				if (showFullFileGraphAction->isChecked())
				{
					mFileNodeActive[n] = false;
				}
				else
				{
					mFileNodeToBeDeleted[n] = true;
				}
			}
		}
	}

	// now (!!): activate folders with active files
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (mFileNodeActive[n] == true)
			activateParent(n);
	}
	
	// now we can delete marked file nodes
	w = 0;
	for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
	{
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}
		if (mFileNodeToBeDeleted[n] == true)
		{
			w = n;
		}
	}
	if (w != 0) 
	{
		mFileGraph.delNode(w);
		w = 0;
	}

	if (!showFullFileGraphAction->isChecked())	// delete empty folders
	{
		bool nodeMarked;
		do
		{
			nodeMarked = false;
			for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
			{
				// check if folder node has file children
				if (mFileNodeType[n] == NODETYPE_FOLDER)
				{
					if (mFileNodeToBeDeleted[n] == false)
					{
						bool noChildNodesFound = true;
						// search nodes under this node and delete them
						for(ogdf::adjEntry adj = n->firstAdj();adj; adj = adj->succ())
						{
							ogdf::edge e = adj->theEdge();
							if ((e->source() == n) && (mFileNodeToBeDeleted[e->target()] == false)) // source is this node
							{
								noChildNodesFound = false;	// no children found, no deleting
								break;
							}
						}
						if (noChildNodesFound == true)
						{
							// special case, node musn't be deleted
							if (!((mFileNodeCollapsed[n] == true) && (mFileNodeActive[n] == true)))
							{
								if (showFullFileGraphAction->isChecked())
								{
									mFileNodeActive[n] = false;		// node now greyed
								}
								else
								{
									mFileNodeToBeDeleted[n] = true;
								}
								nodeMarked = true;
							}
						}
					}
				}
			}
		} while (nodeMarked == true);

		// delete marked folder nodes
		w = 0;
		for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
		{
			if (w != 0) 
			{
				mFileGraph.delNode(w);
				w = 0;
			}
			if (mFileNodeToBeDeleted[n] == true)
			{
				w = n;
			}
		}
		if (w != 0) 
		{
			mFileGraph.delNode(w);
			w = 0;
		}
	}

	// copy fragment data
	mFileViewFragmentData = ogdf::NodeArray<FragmentDataItem>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFragmentData[v] = mFragmentData[v];
	}

	// copy feature data
	mFileViewFeatureData = ogdf::NodeArray<unsigned int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFeatureData[v] = mFeatureData[v];
	}

	// copy file length
	mFileViewFileLength = ogdf::NodeArray<int>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewFileLength[v] = mFileLength[v];
	}

	// copy tooltip labels
	mFileToolTipLabels = ogdf::NodeArray<QString>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileToolTipLabels[v] = mToolTipLabels[v];
	}

	// copy pixmaps
	mFileViewPreviewPixmaps = ogdf::NodeArray<QPixmap>(mPreviewPixmaps);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileViewPreviewPixmaps[v] = mPreviewPixmaps[v];
	}

	// create file view node graphics
	mFileViewGraphicData = ogdf::NodeArray<FileViewNode>(mFileGraph);
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		createFileViewNodeGraphic(v);
	}
	
	if ( filepreviewscreated == false )
	{
		filenodesneedupdate = true;
		this->repaint();
	}
	else
	{
		for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
		{

			if (mFileViewGraphicData[v].graphics->Type() == 1)
			{
				//preview_pm = createCodePreviewPixmap(mFileViewGraphicData[v].graphics);
				if ((mFileViewPreviewPixmaps[v].size().width() > 0) && (mFileViewPreviewPixmaps[v].size().height() > 0))
				{
					//qDebug() << "saving pixmap (file view)...";
					mFileViewGraphicData[v].graphics->setNodePixmap(mFileViewPreviewPixmaps[v]);
				}
			}

			if (mFileViewGraphicData[v].graphicsMiniMap->Type() == 1)
			{
				if ((mFileViewPreviewPixmaps[v].size().width() > 0) && (mFileViewPreviewPixmaps[v].size().height() > 0))
				{
					//qDebug() << "saving pixmap (file mini map)...";
					mFileViewGraphicData[v].graphicsMiniMap->setNodePixmap(mFileViewPreviewPixmaps[v]);
				}
			}
		}
	}

	// create edge graphics for each edge of file graph
	FileViewLinearEdge ve = FileViewLinearEdge();
	mFileViewEdgeGraphicData.fill(ve);
	mFileViewEdgeGraphicData = ogdf::EdgeArray<FileViewLinearEdge>(mFileGraph);
	for(ogdf::edge e = mFileGraph.firstEdge(); e; e = e->succ())
	{
		createFileViewEdgeGraphic(e);
	}

	// calculate layout
	switch (mFileViewLayout)
	{
		case HIERARCHICAL:
			this->setHierarchicalLayout(FILE_VIEW);
			break;
		case BALLON:
			this->setBallonLayout(FILE_VIEW);
			break;
		case TREE:
			this->setTreeLayout(FILE_VIEW);
			break;
		case CIRCULAR:
			this->setCircularLayout(FILE_VIEW);
			break;
		case ENERGYBASED:
			this->setEnergybasedLayout(FILE_VIEW);
			break;
		case ORTHOGONAL:
			this->setOrthogonalLayout(FILE_VIEW);
			break;
		case FASTHIERARCHICAL:
			this->setFastHierarchicalLayout(FILE_VIEW);
			break;
	}

	// TODO: reposition view on old position of collapsed node

	// search new node position
	unsigned int pos = 0;
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		if (mFileGA.labelNode(v) == mFileViewEventNodeLabel)
		{
			if (mFileViewEventNodeAmbigPosition == mNodeAmbiguityPositions[v])
			{
				mFileView->centerOn(mFileViewGraphicData[v].graphics->scenePos());
				mFileMiniMap->centerOn(mFileViewGraphicData[v].graphicsMiniMap->scenePos());
			}
		}
	}

	// TODO: center on collapsed node

	statusBar()->showMessage(tr("Ready."));
}
// collapse tree view and call collapse file view
void SMVMain::collapseFileNode(QString nodeLabel)
{
	// searchModelIndex of node with nodeLabel
	strSearchedNode = nodeLabel;
	idxSearchedNodes.clear();

	for (int i = 0; i < mFileTreeViewModel->rowCount(); i++)
	{
		for (int j = 0; j < mFileTreeViewModel->columnCount(); j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j).data();
			QString strData = var.toString();
			if (strData.compare(strSearchedNode) == 0)
			{
				idxSearchedNodes.push_back(mFileTreeViewModel->index(i,j));		// index found at index(i,j)
				break;
			}
			else searchModelIndexOfTreeViewNodeRecursive(mFileTreeViewModel->index(i,j));
		}
	}

	// collapse file tree view
	QModelIndex idx = idxSearchedNodes[mFileViewEventNodeAmbigPosition-1];
	if (idx.isValid())
	{
		mFileTreeView->collapse(idx);
	}

	collapseFileViewNode();
}
void SMVMain::activateParent(ogdf::node v)
{
	for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->target() == v)	// this node is target
		{
			if (mFileNodeType[e->target()] == NODETYPE_PROJECT)
			{
				return;
			}
			else
			{
				if (mFileNodeActive[e->source()] == false)
				{
					mFileNodeActive[e->source()] = true;	// activate folder
					activateParent(e->source());
				}
			}
		}
	}
}
void SMVMain::searchModelIndexOfTreeViewNodeRecursive(QModelIndex idx)
{
	for (int i = 0; i < mFileTreeViewModel->rowCount(idx); i++)
	{
		for (int j = 0; j < mFileTreeViewModel->columnCount(idx); j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j,idx).data();
			QString strData = var.toString();
			if (strData.compare(strSearchedNode) == 0)
			{
				//qDebug("index found at");
				//qDebug(QString::number(i).toLatin1());
				//qDebug(QString::number(j).toLatin1());
				idxSearchedNodes.push_back(mFileTreeViewModel->index(i,j,idx));
				break;
			}
			else searchModelIndexOfTreeViewNodeRecursive(mFileTreeViewModel->index(i,j,idx));
		}
	}
}
// search position of tree view node in ambiguity vector
void SMVMain::searchAmbiguityPositionOfTreeViewNodeRecursive(QModelIndex idx)
{
	for (int i = 0; i < mFileTreeViewModel->rowCount(idx); i++)
	{
		// only search in this "folder" (model column)
		for (int j = 0; j < 1; j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j,idx).data();
			QString strData = var.toString();

			if (QString::compare(strSearchedNode, strData) == 0)					// compare names (model data)
			{
				positionSearchedNode++;												// increment hits

				if (mFileTreeViewModel->index(i,j,idx) == idxSearchedNode)			// compare indices
				{
					for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())	// iterate through graph
					{
						QString str = mFileGA.labelNode(v).cstr();					//  get name of node
						if (QString::compare(str, strSearchedNode) == 0)			// compare both names (model data, graph node)
						{
							if (positionSearchedNode == mFileNodeAmbiguityPositions[v])	// searched node found!
							{
								mFileViewEventNodeAmbigPosition = positionSearchedNode;
								searchedNode = v;
								return;
							}
							// else: position is different -> ambiguity! this is not the searched node !
						}
					}
				}
			}
			else
			{
				// search further in this model entry, if it is a folder and if there are children and folder expanded
				if (mFileTreeViewModel->hasChildren(mFileTreeViewModel->index(i,j)))			// are there children?
				{
					if (mFileTreeView->isExpanded(mFileTreeViewModel->index(i,j,idx)))			// is folder expanded ?
					{
						searchAmbiguityPositionOfTreeViewNodeRecursive(mFileTreeViewModel->index(i,j,idx));	// search further
					}
				}
			}
		}
	}
}
// expand tree view and call expand file view
void SMVMain::expandFileNode(QString nodeLabel)
{
	// searchModelIndex of node with nodeLabel
	strSearchedNode = nodeLabel;
	idxSearchedNodes.clear();

	for (int i = 0; i < mFileTreeViewModel->rowCount(); i++)
	{
		for (int j = 0; j < mFileTreeViewModel->columnCount(); j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j).data();
			QString strData = var.toString();
			if (strData.compare(strSearchedNode) == 0)
			{
				idxSearchedNodes.push_back(mFileTreeViewModel->index(i,j));
				break;
			}
			else searchModelIndexOfTreeViewNodeRecursive(mFileTreeViewModel->index(i,j));
		}
	}

	// expand file tree view
	QModelIndex idx = idxSearchedNodes[mFileViewEventNodeAmbigPosition-1];
	if (idx.isValid())
	{
		mFileTreeView->expand(idx);
	}

	collapseFileViewNode();
}
// delete graphic items of subgraph in file view
void SMVMain::deleteFileViewSubGraph(ogdf::node v)
{
	// search nodes under this node and delete them
	for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->source() == v) // source is this node
		{
			deleteFileViewSubGraph(e->target());
		}
	}
	mFileNodeToBeDeleted[v] = true;
}


// create feature graph
void SMVMain::createFeatureGraph()
{
	if(path_model_m.isEmpty())
	{
		QMessageBox msgBox;
		msgBox.setText("No Feature Data existing!\nFeature graph could not be created!");
		msgBox.exec();
		return;
	}

	statusBar()->showMessage(tr("Creating feature graph..."));

	ReadModel graphModel;
	graphModel.loadFile(path_model_m);
	graphModel.init();
	
	// read model.m and create the model graph
	mFeatureGraph = new ogdf::Graph(graphModel.createGraph());

	mFeatureViewGraphicData = ogdf::NodeArray<FeatureViewNode>(*mFeatureGraph);
	// create node array for node types
	mFeatureNodeType = ogdf::NodeArray<GraphEdgeType>(*mFeatureGraph);	
	// create node array for active and inactive features
	mNodeActive = ogdf::NodeArray<bool>(*mFeatureGraph);
	mFeatureNodeSelected = ogdf::NodeArray<bool>(*mFeatureGraph);
	
	mFeatureGA = new GraphAttributes();
	mFeatureGA->init(*mFeatureGraph,
		ogdf::GraphAttributes::nodeGraphics |
		ogdf::GraphAttributes::edgeGraphics |
		ogdf::GraphAttributes::nodeLevel |
		ogdf::GraphAttributes::edgeIntWeight |
		ogdf::GraphAttributes::edgeDoubleWeight |
		ogdf::GraphAttributes::edgeLabel |
		ogdf::GraphAttributes::nodeLabel |
		ogdf::GraphAttributes::edgeType |
		ogdf::GraphAttributes::nodeType |
		ogdf::GraphAttributes::nodeColor |
		ogdf::GraphAttributes::nodeId |
		ogdf::GraphAttributes::edgeArrow |
		ogdf::GraphAttributes::edgeColor |
		ogdf::GraphAttributes::edgeStyle |
		ogdf::GraphAttributes::nodeStyle |
		ogdf::GraphAttributes::nodeTemplate |
		ogdf::GraphAttributes::edgeSubGraph);

	for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
	{
		mFeatureGA->labelNode(v) = graphModel.getLabel(v);
		// set type
		mFeatureGA->type(v) = graphModel.getType(v);
		// init activation status of nodes
		mNodeActive[v] = false;
		mFeatureNodeSelected[v] = true; 
		createFeatureViewNodeGraphic(v);
	}

	//printGraph();			// print graph to console

	mFeatureViewLayout = HIERARCHICAL;

	// hierarchical layout
	ogdf::SugiyamaLayout SL;
	SL.setRanking(new ogdf::OptimalRanking);
	SL.setCrossMin(new ogdf::MedianHeuristic);
	ogdf::OptimalHierarchyLayout *ohl = new ogdf::OptimalHierarchyLayout;
	ohl->layerDistance(10.0);
	ohl->nodeDistance(10.0);
	ohl->weightBalancing(0.8);
	SL.setLayout(ohl);
	SL.call(*mFeatureGA);

	// update BoundingBox
	mFeatureView->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN, mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN, mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN, mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);
	mFeatureMiniMap->scene()->setSceneRect(mFeatureGA->boundingBox().p1().m_x - GRAPH_MARGIN, mFeatureGA->boundingBox().p1().m_y - GRAPH_MARGIN, mFeatureGA->boundingBox().p2().m_x + 2 * GRAPH_MARGIN, mFeatureGA->boundingBox().p2().m_y + 2 * GRAPH_MARGIN);

	//update positions
	for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
	{
		mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
		mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));
		mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());

		mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
		mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
		mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphics->boundingRect());
	}

	// create edge graphics for each edge of file graph
	mFeatureViewEdgeGraphicData = ogdf::EdgeArray<FeatureViewLinearEdge>(*mFeatureGraph);
	for(ogdf::edge e = mFeatureGraph->firstEdge(); e; e = e->succ())
	{
		createFeatureViewEdgeGraphic(e);
	}

	// scale views
	mFeatureView->showFullScene();
	mFeatureMiniMap->showFullScene();

	// create feature tree view
	strFeatureTree.clear();

	// create string of feature view
	ogdf::node n = mFeatureGraph->firstNode();
	strFeatureTree.append(mFeatureGA->labelNode(n).cstr());
	strFeatureTree.append("\n");

	for(ogdf::adjEntry adj = n->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->source() == n) // source is this node
		{
			//qDebug(strFileTree.toLatin1());	// print file tree to console
			createFeatureGraphString(e->target(), 1);
		}
	}
	
	// create model
	mFeatureTreeViewModel = new FeatureTreeModel(strFeatureTree);

	// show feature model in tree view
	mFeatureTreeView->setModel(mFeatureTreeViewModel);
	mFeatureTreeView->expandAll();
}

// recursive function to create string of feature tree
void SMVMain::createFeatureGraphString(ogdf::node v, int level)
{
	for (int i = 0; i < level; i++)
	{
		strFeatureTree.append("    ");
	}
	strFeatureTree.append(mFeatureGA->labelNode(v).cstr());
	strFeatureTree.append("\n");
	// search nodes under this node and delete them
	for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
	{
		ogdf::edge e = adj->theEdge();
		if (e->source() == v) // source is this node
		{
			createFeatureGraphString(e->target(), level+1);
		}
	}
}

// create FeatureViewNodeGraphics for given node
void SMVMain::createFeatureViewNodeGraphic(ogdf::node v)
{
	mFeatureViewGraphicData[v].graphics = new FeatureViewNodeGraphic(this);
	mFeatureViewGraphicData[v].graphics->m_parent = (QGraphicsView*)mFeatureView;
	
	// set properties
	mFeatureViewGraphicData[v].graphics->setLabel(mFeatureGA->labelNode(v).cstr());
	mFeatureViewGraphicData[v].graphics->setFeatureGraphNode(v);

	// get color
	for (unsigned int i = 0; i < mFeatureList.size(); i++)
	{
		if (QString::compare(mFeatureList[i].name, mFeatureGA->labelNode(v).cstr()) == 0)
		{
			mFeatureViewGraphicData[v].graphics->setFeatureColor(mFeatureList[i].color);
			break;
		}
	}
	
	// Add dependency
	if((int)mFeatureGA->type(v) == 0)
	{
		mFeatureViewGraphicData[v].graphics->setType(EDGETYPE_EMPTY);
	}
	else if((int)mFeatureGA->type(v) == 1)
	{
		mFeatureViewGraphicData[v].graphics->setType(EDGETYPE_OPTIONAL);
	}
	else if((int)mFeatureGA->type(v) == 2)
	{
		mFeatureViewGraphicData[v].graphics->setType(EDGETYPE_ALTERNATIVE);
	}
	else if((int)mFeatureGA->type(v) == 3)
	{
		mFeatureViewGraphicData[v].graphics->setType(EDGETYPE_OBLIGATORY);
	}
	
	std::vector<Feature>::iterator it;
	QString strNodeName;
	QByteArray name;
	String strOutput;

	//init all features with true
	for ( it=mFeatureList.begin() ; it < mFeatureList.end(); it++ )
	{
		strNodeName =it->name;
		// convert from QString to String
		name = strNodeName.toLocal8Bit();
		strOutput = name.constData();
		
		if((strOutput.compare(mFeatureGA->labelNode(v),strOutput))==0)
		{
			if ((int)mFeatureGA->type(v) == 3) mNodeActive[v] = false;
			else mNodeActive[v] = true;
			mNodeActive[v] = true;
			mFeatureViewGraphicData[v].graphics->setActivation(true);
			mFeatureViewGraphicData[v].graphics->setIsFeature(true);
			break;
		}
		else
		{		
			mNodeActive[v] = false;
			mFeatureViewGraphicData[v].graphics->setIsFeature(false);
			mFeatureViewGraphicData[v].graphics->setActivation(false);
		}
	}

	mFeatureGA->x(v) = 0;
	mFeatureGA->y(v) = 0;

	mFeatureViewGraphicData[v].graphics->setX(mFeatureGA->x(v));
	mFeatureViewGraphicData[v].graphics->setY(mFeatureGA->y(v));

	mFeatureViewGraphicData[v].graphics->calculateDimensions();

	mFeatureGA->width(v) = double(mFeatureViewGraphicData[v].graphics->getObjectWidth());
	mFeatureGA->height(v) = double(mFeatureViewGraphicData[v].graphics->getObjectHeight());

	mFeatureView->scene()->addItem(mFeatureViewGraphicData[v].graphics);
	
	// create copy of node graphic for feature minimap
	mFeatureViewGraphicData[v].graphicsMiniMap = new FeatureViewNodeGraphic(this);
	mFeatureViewGraphicData[v].graphicsMiniMap->m_parent = (QGraphicsView*)mFeatureMiniMap;
	mFeatureViewGraphicData[v].graphicsMiniMap->setLabel(mFeatureViewGraphicData[v].graphics->Label());
	mFeatureViewGraphicData[v].graphicsMiniMap->setFeatureGraphNode(v);
	mFeatureViewGraphicData[v].graphicsMiniMap->setFeatureColor(mFeatureViewGraphicData[v].graphics->getFeatureColor());
	mFeatureViewGraphicData[v].graphicsMiniMap->setType(mFeatureViewGraphicData[v].graphics->Type());
	mFeatureViewGraphicData[v].graphicsMiniMap->setActivation(mFeatureViewGraphicData[v].graphics->getActivation());
	mFeatureViewGraphicData[v].graphicsMiniMap->setIsFeature(mFeatureViewGraphicData[v].graphics->isFeature());
	mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureGA->x(v));
	mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureGA->y(v));
	mFeatureViewGraphicData[v].graphicsMiniMap->calculateDimensions();
	mFeatureMiniMap->scene()->addItem(mFeatureViewGraphicData[v].graphicsMiniMap);
}

// create FeatureViewEdgeGraphics for given edge
void SMVMain::createFeatureViewEdgeGraphic(ogdf::edge e)
{
	// set coordinates
	int s_x = mFeatureViewGraphicData[e->source()].graphics->pos().x();
	int s_y = mFeatureViewGraphicData[e->source()].graphics->pos().y()+(mFeatureViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	int t_x = mFeatureViewGraphicData[e->target()].graphics->pos().x();
	int t_y = mFeatureViewGraphicData[e->target()].graphics->pos().y()-(mFeatureViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	QPoint s(s_x, s_y);
	QPoint t(t_x, t_y);

	// create graphics object
	mFeatureViewEdgeGraphicData[e].graphics = new FeatureEdgeGraphic(this, s, t);
	mFeatureView->scene()->addItem(mFeatureViewEdgeGraphicData[e].graphics);
	
	// create copy of edge graphic for feature minimap
	mFeatureViewEdgeGraphicData[e].graphicsMiniMap = new FeatureEdgeGraphic(this, s, t);
	mFeatureMiniMap->scene()->addItem(mFeatureViewEdgeGraphicData[e].graphicsMiniMap);
}
// update position of given FeatureViewNode
void SMVMain::updateFeatureViewNodePosition(ogdf::node v)
{
	mFeatureGA->x(v) = mFeatureViewGraphicData[v].graphics->pos().x();
	mFeatureGA->y(v) = mFeatureViewGraphicData[v].graphics->pos().y();

	mFeatureViewGraphicData[v].graphicsMiniMap->setX(mFeatureViewGraphicData[v].graphics->pos().x());
	mFeatureViewGraphicData[v].graphicsMiniMap->setY(mFeatureViewGraphicData[v].graphics->pos().y());

	for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
	{
		updateFeatureViewEdgePosition(adj->theEdge());
	}
}
void SMVMain::updateFeatureViewEdgePosition(ogdf::edge e)
{
	// set coordinates
	qreal s_x = mFeatureViewGraphicData[e->source()].graphics->pos().x();
	qreal s_y = mFeatureViewGraphicData[e->source()].graphics->pos().y()+(mFeatureViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	qreal t_x = mFeatureViewGraphicData[e->target()].graphics->pos().x();
	qreal t_y = mFeatureViewGraphicData[e->target()].graphics->pos().y()-(mFeatureViewGraphicData[e->source()].graphics->boundingRect().height()/2);
	QPointF s(s_x, s_y);
	mFeatureViewEdgeGraphicData[e].graphics->setSource(s);
	mFeatureViewEdgeGraphicData[e].graphicsMiniMap->setSource(s);
	QPointF t(t_x, t_y);
	mFeatureViewEdgeGraphicData[e].graphics->setTarget(t);
	mFeatureViewEdgeGraphicData[e].graphicsMiniMap->setTarget(t);
	mFeatureViewEdgeGraphicData[e].graphics->update(mFeatureViewEdgeGraphicData[e].graphics->boundingRect());
	mFeatureViewEdgeGraphicData[e].graphicsMiniMap->update(mFeatureViewEdgeGraphicData[e].graphicsMiniMap->boundingRect());
}
void SMVMain::updateAlternativeFeatures(ogdf::node v, bool aktive)
{
	ogdf::edge e;
	ogdf::edge k;
	ogdf::node x;

	// alle eingehenden kanten
	forall_adj_edges(e,v)
	{
		if(e->target() != v) 
			continue;
		else
		{
			node parent = e->source();
			// alle kinder vom parent finden 
			forall_adj_edges(k,parent)
			{
				if ((x = k->target()) != v) 
				{
					if(!aktive) 
					{
						mNodeActive[x] = false;
						mFeatureViewGraphicData[x].graphics->setActivation(false);
						mFeatureViewEdgeGraphicData[k].graphics->update(mFeatureViewEdgeGraphicData[k].graphics->boundingRect());
					}
					else
					{
						// avoid deactivation
						mNodeActive[x] = false;
						mFeatureViewGraphicData[x].graphics->setActivation(false);
						mNodeActive[v] = true;
						mFeatureViewGraphicData[v].graphics->setActivation(true);
					}
				}
			}
		}
	}
}
void SMVMain::updateFeatureStates()
{
	for (unsigned int i = 0; i < mFeatureList.size(); i++)
	{
		mFeatureList[i].isActive = false;
		// check if feature is enabled
		for(ogdf::node n = mFeatureGraph->firstNode(); n; n = n->succ())
		{
			//qDebug(mFeatureGA->labelNode(n).cstr());
			
			if (mFeatureGA->type(n) == 3) // obligatorial always active
			{
				if (QString::compare(mFeatureList[i].name,mFeatureGA->labelNode(n).cstr()) == 0)
				{
					mFeatureList[i].isActive = true;	// feature is now active
					break;
				}
			}
			else
			{
				if (QString::compare(mFeatureList[i].name,mFeatureGA->labelNode(n).cstr()) == 0)
				{
					if (mNodeActive[n] == true)
					{
						mFeatureList[i].isActive = true;	// feature is now active
						break;
					}
				}
			}
		}
	}
}
// zoom into feature, show file sub-graph
void SMVMain::zoomIntoFeature(QString feature_name)
{
	QString statusmsg;
	statusmsg.append("Zooming into feature ");
	statusmsg.append(feature_name);
	statusmsg.append("...");
	statusBar()->showMessage(tr(statusmsg.toLatin1()));
	// check if file view has to be changed
	if (QString::compare(featureInFileView,feature_name) == 0)	// check if names are equal
	{
		// file sub-graph does not have to be changed
		mFeatureView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
		//mFeatureView->setNextWidgetToFadeIn(1);
		//mFeatureView->fadeOut();

		for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
		{
			if (QString::compare(mFeatureGA->labelNode(v).cstr(),feature_name) == 0)
			{
				// calculate zoom proportions
				if (filescenepreview->size().width() > 0)
				{
					mFeatureViewGraphicData[v].graphics->drawfilescenepreview = true;

					qreal zoom_factor_hori = (qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_width / (qreal)mFeatureView->geometry().width();
					qreal zoom_factor_vert = (qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_height / (qreal)mFeatureView->geometry().height();

					qreal scaling_factor;
					if (zoom_factor_hori > zoom_factor_vert)
						scaling_factor = 1/zoom_factor_hori;
					else scaling_factor = 1/zoom_factor_vert;

					bool scaling_zoomed_in = true;

					//qDebug() << "zoom_factor_hori = " << zoom_factor_hori;
					//qDebug() << "zoom_factor_vert = " << zoom_factor_vert;
					//qDebug() << "scaling_factor = " << scaling_factor;

					mFileView->setForegroundTransparent();

					mFeatureView->zoomIntoFileView(scaling_factor, scaling_zoomed_in, v);
				}
			}
		}
	}
	else
	{
		// recalculation of file sub-graph for this feature
		for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
		{
			if (QString::compare(mFeatureGA->labelNode(v).cstr(),feature_name) == 0)
			{
				mFeatureNodeSelected[v] = true;		// selection of feature node
			}
			else
			{
				mFeatureNodeSelected[v] = false;	// de-selection of feature node
			}
		}
		// show also all children of this node
		bool noNodeAdded;
		do
		{
			noNodeAdded = true;
			for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
			{
				if (mFeatureNodeSelected[v] == true)
				{
					for(ogdf::adjEntry adj = v->firstAdj();adj; adj = adj->succ())
					{
						ogdf::edge e = adj->theEdge();
						if (e->source() == v) // source is this node
						{
							if (mFeatureNodeSelected[e->target()] == false)
							{
								mFeatureNodeSelected[e->target()] = true;
								noNodeAdded = false;
							}
						}
					}
				}
			}
		} while(!noNodeAdded);

		featureInFileView = feature_name;
		createFileGraph();

		mFeatureView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
		//mFeatureView->setNextWidgetToFadeIn(1);
		//mFeatureView->fadeOut();

		// set zoomed feature
		for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
		{
			if (QString::compare(mFeatureGA->labelNode(v).cstr(), feature_name) == 0)
			{
				mFeatureView->zoomIntoFeatureGraphic = mFeatureViewGraphicData[v].graphics;
	
				// render preview 
				renderfileview = true;
				this->repaint();
				
				// calculate proportions

				if (filescenepreview->size().width() > 0)
				{
					mFeatureViewGraphicData[v].graphics->drawfilescenepreview = true;

					qreal pixmap_ratio = (qreal)filescenepreview->width() / (qreal)filescenepreview->height();
					//qDebug() << "fileviewnodegraphic, pixmap ratio = " << pixmap_ratio;

					qreal item_ratio = mFeatureViewGraphicData[v].graphics->boundingRect().width() / mFeatureViewGraphicData[v].graphics->boundingRect().height();

					if (item_ratio > pixmap_ratio) // item is in proportions wider than pixmap, scale to height of item
					{
						mFeatureViewGraphicData[v].graphics->preview_pixmap_y_start = (int)(mFeatureViewGraphicData[v].graphics->boundingRect().topLeft().y()) + 1;

						mFeatureViewGraphicData[v].graphics->preview_pixmap_height =  (int)(mFeatureViewGraphicData[v].graphics->boundingRect().height() - 2.0);

						mFeatureViewGraphicData[v].graphics->preview_pixmap_width = (int)((qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_height * pixmap_ratio + 0.5);

						mFeatureViewGraphicData[v].graphics->preview_pixmap_x_start = - mFeatureViewGraphicData[v].graphics->preview_pixmap_width / 2;
					}
					else	// item is in proportions higher than pixmap, scale to width of item
					{

						mFeatureViewGraphicData[v].graphics->preview_pixmap_x_start = (int)(mFeatureViewGraphicData[v].graphics->boundingRect().topLeft().x()) + 1;

						mFeatureViewGraphicData[v].graphics->preview_pixmap_width = (int)(mFeatureViewGraphicData[v].graphics->boundingRect().width() - 2.0);

						mFeatureViewGraphicData[v].graphics->preview_pixmap_height = (int)((qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_width / pixmap_ratio + 0.5 );

						mFeatureViewGraphicData[v].graphics->preview_pixmap_y_start = - mFeatureViewGraphicData[v].graphics->preview_pixmap_height / 2;
					}


					/*
					qDebug() << "mFileView->geometry().width() = " << mFileView->geometry().width();
					qDebug() << "mFileView->geometry().height() = " << mFileView->geometry().height();

					qDebug() << "mFeatureViewGraphicData[v].graphics width = " << mFeatureViewGraphicData[v].graphics->boundingRect().width();
					qDebug() << "mFeatureViewGraphicData[v].graphics height = " << mFeatureViewGraphicData[v].graphics->boundingRect().height();

					qDebug() << "preview_pixmap_width = " << mFeatureViewGraphicData[v].graphics->preview_pixmap_width;
					qDebug() << "preview_pixmap_height = " << mFeatureViewGraphicData[v].graphics->preview_pixmap_height;
					qDebug() << "preview_pixmap_x_start = " << mFeatureViewGraphicData[v].graphics->preview_pixmap_x_start;
					qDebug() << "preview_pixmap_y_start = " << mFeatureViewGraphicData[v].graphics->preview_pixmap_y_start;
					*/


					// calculate zoom proportions

					qreal zoom_factor_hori = (qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_width / (qreal)mFeatureView->geometry().width();
					qreal zoom_factor_vert = (qreal)mFeatureViewGraphicData[v].graphics->preview_pixmap_height / (qreal)mFeatureView->geometry().height();

					qreal scaling_factor;
					if (zoom_factor_hori > zoom_factor_vert)
						scaling_factor = 1/zoom_factor_hori;
					else scaling_factor = 1/zoom_factor_vert;

					bool scaling_zoomed_in = true;

					//qDebug() << "zoom_factor_hori = " << zoom_factor_hori;
					//qDebug() << "zoom_factor_vert = " << zoom_factor_vert;
					//qDebug() << "scaling_factor = " << scaling_factor;

					mFileView->setForegroundTransparent();

					mFeatureView->zoomIntoFileView(scaling_factor, scaling_zoomed_in, v);
				}


			}
		}

		statusmsg.clear();
		statusmsg.append("File view: Files for feature '");
		statusmsg.append(feature_name);
		statusmsg.append("' visible.");
		statusBar()->showMessage(tr(statusmsg.toLatin1()));
	}
}

// print feature list
void SMVMain::printFeatureList()
{
	// print list of features onto console
	qDebug("Feature-List:");
	for (unsigned int i=0; i < mFeatureList.size(); i++)
	{
		QString str,name;
		name = mFeatureList[i].name;

		str.append("Feature: List-ID = ");
		str.append(QString::number(i));
		str.append(" Name = ");
		str.append(name);
		qDebug(str.toLatin1());
	}
}

// print graph into output stream
void SMVMain::printGraph()
{
	std::ostringstream oss;
	std::string mystr;
	qDebug("--------------");
	qDebug("Graph content:");
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		oss << v;
		switch(mFileNodeType[v])
		{
			case NODETYPE_PROJECT:
				oss << ", Type = Project";
				if (mFileGA.labelNode(v).length()>0)
					oss << ", Label = " << mFileGA.labelNode(v);
				break;
			case NODETYPE_FILE:
				oss << ", Type = File";
				if (mFileGA.labelNode(v).length()>0)
					oss << ", Label = " << mFileGA.labelNode(v);
				break;
			case NODETYPE_FOLDER:
				oss << ", Type = Folder";
				if (mFileGA.labelNode(v).length()>0)
					oss << ", Label = " << mFileGA.labelNode(v);
				break;
			// not valid for file graph
			/*
			case NODETYPE_FRAGMENT:
				oss << ", Type = Fragment";
				oss << " (Offset = " << mFragmentData[v].offset << ", Length = " << mFragmentData[v].length << ")";
				break;
			case NODETYPE_FEATURE:
				oss << ", Type = Feature" << " (Name = " << mFeatureList[mFeatureData[v]].name.toStdString() << ")";
				break;
			case NODETYPE_UNKNOWN:
				oss << ", Type = Unknown";
				break;
			*/
		}

		if (mFileNodeCollapsed[v] == true)
			oss << ", Collapsed = true ";

		oss << " : ";
		ogdf::adjEntry adj;
		forall_adj(adj,v)
		{
			oss << " " << adj->theEdge();
		}
		qDebug(oss.str().c_str());
		oss.str("");
	}
	qDebug("--------------");
}

void SMVMain::printFileTreeViewModel()
{
	for (int i = 0; i < mFileTreeViewModel->rowCount(); i++)
	{
		for (int j = 0; j < mFileTreeViewModel->columnCount(); j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j).data();
			QString strData = var.toString();
			// debug
			qDebug(strData.toLatin1());
			printFileTreeViewModelRecursive(mFileTreeViewModel->index(i,j));
									
		}
	}
}
void SMVMain::printFileTreeViewModelRecursive(QModelIndex idx)
{
	for (int i = 0; i < mFileTreeViewModel->rowCount(idx); i++)
	{
		for (int j = 0; j < mFileTreeViewModel->columnCount(idx); j++)
		{
			QVariant var = mFileTreeViewModel->index(i,j,idx).data();
			
			QString strData = var.toString();
			// debug
			qDebug(strData.toLatin1());
			printFileTreeViewModelRecursive(mFileTreeViewModel->index(i,j,idx));
		}
	}
}
void SMVMain::treeViewCollapsSlot(QModelIndex idx)
{
	// recalculate ambiguity positions

	// initialize
	mFileNodeAmbiguityPositions.init();
	mFileNodeAmbiguityPositions = ogdf::NodeArray<int>(mFileGraph);
	nodeLabelVector.clear();
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileNodeAmbiguityPositions[v] = false;
	}
	// get every label in list
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mFileGA.labelNode(v);
		if (nodeLabel.length() > 0)
			nodeLabelVector.push_back(nodeLabel);
		
	}
	// make every item unique in list
	nodeLabelList.clear();
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelList.push_back(nodeLabelVector[i]);
	}
	nodeLabelList.unique();
	nodeLabelVector.clear();
	nodeLabelCounts.clear();
	std::list<ogdf::String>::iterator it;
	for (it = nodeLabelList.begin(); it != nodeLabelList.end(); it++)
	{
		nodeLabelVector.push_back(*it);
	}
	// initialize counts of labels
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelCounts.push_back(0);
	}
	
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mFileGA.labelNode(v);
		// get position
		unsigned int  i;
		for (i = 0; i < nodeLabelVector.size(); i++)
		{
			if (ogdf::String::compare(nodeLabelVector[i],nodeLabel) == 0)
			{
				nodeLabelCounts[i] += 1;
				mFileNodeAmbiguityPositions[v] = nodeLabelCounts[i];
				break;
			}
		}
	}

	QVariant var = mFileTreeViewModel->data(idx,0);
	strSearchedNode = var.toString();
	searchedNode = 0;

	// reset index
	idxSearchedNode = idx;
	positionSearchedNode = 0;

	// start search for ambiguity position at first model entry
	searchAmbiguityPositionOfTreeViewNodeRecursive(mFileTreeViewModel->index(0,0));

	mFileViewEventNodePosition = mFileViewGraphicData[searchedNode].graphics->scenePos();
	mFileViewEventNodeLabel = ogdf::String(mFileGA.labelNode(searchedNode));

	mNodeCollapsed[searchedNode] = true;
	collapseFileViewNode();
}
void SMVMain::treeViewExpandSlot(QModelIndex idx)
{
	// recalculate ambiguity positions

	// initialize
	mFileNodeAmbiguityPositions.init();
	mFileNodeAmbiguityPositions = ogdf::NodeArray<int>(mFileGraph);
	nodeLabelVector.clear();
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		mFileNodeAmbiguityPositions[v] = false;
	}
	// get every label in list
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mFileGA.labelNode(v);
		if (nodeLabel.length() > 0)
			nodeLabelVector.push_back(nodeLabel);
		
	}
	// make every item unique in list
	nodeLabelList.clear();
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelList.push_back(nodeLabelVector[i]);
	}
	nodeLabelList.unique();
	nodeLabelVector.clear();
	nodeLabelCounts.clear();
	std::list<ogdf::String>::iterator it;
	for (it = nodeLabelList.begin(); it != nodeLabelList.end(); it++)
	{
		nodeLabelVector.push_back(*it);
	}
	// initialize counts of labels
	for (unsigned int i = 0; i < nodeLabelVector.size(); i++)
	{
		nodeLabelCounts.push_back(0);
	}
	// iterate through graph and calculate ambiguity positons
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		ogdf::String nodeLabel = mFileGA.labelNode(v);
		
		// calculate position for node
		unsigned int  i;
		for (i = 0; i < nodeLabelVector.size(); i++)
		{
			if (ogdf::String::compare(nodeLabelVector[i],nodeLabel) == 0)
			{
				nodeLabelCounts[i] += 1;
				mFileNodeAmbiguityPositions[v] = nodeLabelCounts[i];
				break;
			}
		}
	}

	QVariant var = mFileTreeViewModel->data(idx,0);
	strSearchedNode = var.toString();
	searchedNode = ogdf::node();

	// reset index
	idxSearchedNode = idx;
	positionSearchedNode = 0;

	// start search at first model entry
	searchAmbiguityPositionOfTreeViewNodeRecursive(mFileTreeViewModel->index(0,0));

	mFileViewEventNodePosition = mFileViewGraphicData[searchedNode].graphics->scenePos();
	mFileViewEventNodeLabel = ogdf::String(mFileGA.labelNode(searchedNode));

	mNodeCollapsed[searchedNode] = false;
	collapseFileViewNode();
}

// TODO: feature collapse
void SMVMain::featureTreeViewCollapsSlot(QModelIndex idx)
{
}
// TODO: feature expand
void SMVMain::featureTreeViewExpandSlot(QModelIndex idx)
{
}
void SMVMain::mousePressEvent(QMouseEvent *event)
{
	if (mViewMode == CODE_VIEW)
	{
		editor->getMousePressEventFromMainWindow(mapToGlobal(event->pos()));
	}
	else QMainWindow::mousePressEvent(event);
}
void SMVMain::wheelEvent(QWheelEvent *event)
{
	QMainWindow::wheelEvent(event);
}
void SMVMain::CodeColorTransparency(int value)
{
	editor->setColorTransparency(value);
}
#if 0
QPixmap SMVMain::createCodePreviewPixmap(FileViewNodeGraphic *graphic)
{
	// get file name
	ogdf::node v = graphic->getFileGraphNode();
	if (graphic->Type() == 1)
	{
		// get file path
		QString filePath;
		filePath.clear();
		filePath.append(mFileGA.labelNode(v).cstr());

		ogdf::node parent = ogdf::node(v);
		ogdf::node root = mFileGraph.firstNode();
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
						filePath.insert(0,QString(mFileGA.labelNode(parent).cstr()));
					break;
				}
			}
		} while (parent != root);
		
		// complete file path
		filePath.insert(0,getProjectPath());

		qDebug("try to open file");
		qDebug(filePath.toLatin1());
		// load file into code view
		QFile file(filePath);
		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			// remove old text
			QTextCursor curs = editor->textCursor();
			curs.setPosition(0,QTextCursor::MoveAnchor);
			curs.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
			curs.removeSelectedText();

			// load file content into editor
			editor->setPlainText(file.readAll());
			editor->resetBackground();
			
			curs = editor->textCursor();
			curs.setPosition(0,QTextCursor::MoveAnchor);
			editor->setTextCursor(curs);

			// set graphic and create fragment rectangles
			editor->setCurrentGraphic(graphic);
			
			// calculate fragment rectangles
			editor->createFragmentRects();

			// activate all fragments
			for (int i = 0; i < editor->feature_is_painted.size(); i++)
				editor->feature_is_painted[i] =  true;

			editor->colorTransparency = 255;
			editor->drawFragments();

			// Prepare a pixmap large enough to draw the item into
			return editor->getDocumentPixmap();
		}
	}

	return QPixmap();
}
#endif


void SMVMain::paintEvent(QPaintEvent*)
{
	if (renderfileview == true)
	{
		
		mFileView->setEnabled(true);
		mFileView->show();
		mFileView->showFullSceneInstant();

		QColor c(Qt::white);
		c.setAlpha(0);
		QBrush b(c);
		mFileView->setForegroundBrush(b);

		// render preview

		
		//filescenepreview = QPixmap(mFileView->sceneRect().toRect().width(), mFileView->sceneRect().toRect().height() );
		if (filescenepreview != 0)
		{
			delete filescenepreview;
			filescenepreview = 0;
		}

		//qDebug() << "render preview";

		filescenepreview = new QPixmap( mFeatureView->geometry().width(), mFeatureView->geometry().height() );
		QPainter painter(filescenepreview);
		painter.fillRect(filescenepreview->rect(),QBrush(Qt::white));
		//mFileView->scene()->render(&painter);
		mFileView->render(&painter,filescenepreview->rect(),mFileView->geometry());

		mFileView->hide();
		mFileView->setEnabled(false);

		renderfileview = false;
	}
	if (filenodesneedupdate == true)
	{
		//qDebug() << "updating file previews";
		editor->setEnabled(true);
		editor->show();
		for(ogdf::node n = mFileGraph.firstNode(); n; n = n->succ())
		{
			// create code preview pixmap
			QPixmap preview_pm;
			// get file name
			ogdf::node v = mFileViewGraphicData[n].graphics->getFileGraphNode();
			if (mFileViewGraphicData[n].graphics->Type() == 1)
			{
				/*
				if (QString::compare(mFileGA.labelNode(v).cstr(),"annotations.xml") == 0)
					continue;
				*/
				// get file path
				QString filePath;
				filePath.clear();
				filePath.append(mFileGA.labelNode(v).cstr());
				//qDebug() << "updating file " << filePath;

				ogdf::node parent = ogdf::node(v);
				ogdf::node root = mFileGraph.firstNode();
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
								filePath.insert(0,QString(mFileGA.labelNode(parent).cstr()));
							break;
						}
					}
				} while (parent != root);
				
				// complete file path
				filePath.insert(0,getProjectPath());

				//qDebug("try to open file");
				//qDebug(filePath.toLatin1());
				// load file into code view
				QFile file(filePath);
				if (file.open(QFile::ReadOnly | QFile::Text))
				{
					// remove old text
					QTextCursor curs = editor->textCursor();
					curs.setPosition(0,QTextCursor::MoveAnchor);
					curs.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
					curs.removeSelectedText();

					// load file content into editor
					editor->setPlainText(file.readAll());
					editor->resetBackground();
					
					curs = editor->textCursor();
					curs.setPosition(0,QTextCursor::MoveAnchor);
					editor->setTextCursor(curs);

					// set graphic and create fragment rectangles
					editor->setCurrentGraphic(mFileViewGraphicData[n].graphics);
					
					// calculate fragment rectangles
					editor->createFragmentRects();

					// activate all fragments
					
					for (int i = 0; i < editor->feature_is_painted.size(); i++)
						editor->feature_is_painted[i] =  true;
					

					editor->colorTransparency = 255;
					editor->drawFragments();

					// Prepare a pixmap large enough to draw the item into
					preview_pm = editor->getDocumentPixmap();
				}
			}
			
			if (mFileViewGraphicData[v].graphics->Type() == 1)
			{
				//preview_pm = createCodePreviewPixmap(mFileViewGraphicData[v].graphics);
				if ((preview_pm.size().width() > 0) && (preview_pm.size().height() > 0))
				{
					//qDebug() << "saving pixmap for " << mFileViewGraphicData[v].graphics->Label();
					mPreviewPixmaps[v] = preview_pm;
					mFileViewPreviewPixmaps[v] = preview_pm;

					mFileViewGraphicData[v].graphics->setNodePixmap(preview_pm);
				}
			}

			if (mFileViewGraphicData[v].graphicsMiniMap->Type() == 1)
			{
				if ((preview_pm.size().width() > 0) && (preview_pm.size().height() > 0))
				{
					//qDebug() << "saving pixmap (file mini map)...";
					mFileViewGraphicData[v].graphicsMiniMap->setNodePixmap(preview_pm);
				}
			}
		}

		editor->setEnabled(false);
		editor->hide();
		filenodesneedupdate = false;
		filepreviewscreated = true;
	}
}

void SMVMain::openColorDialog()
{
	mColorDialog->show();
}

void SMVMain::updateFeatureColor(QString featurename, QColor col)
{
	for (unsigned int i = 0; i < mFeatureList.size(); i++)
	{
		if (QString::compare(mFeatureList[i].name,featurename) == 0)
		{
			mFeatureList[i].color = col;
		}
	}
	for(ogdf::node v = mFeatureGraph->firstNode(); v; v = v->succ())
	{
		if (QString::compare(mFeatureViewGraphicData[v].graphics->Label(),featurename) == 0)
		{
			mFeatureViewGraphicData[v].graphics->setFeatureColor(col);
			mFeatureViewGraphicData[v].graphics->update(mFeatureViewGraphicData[v].graphics->boundingRect());
			mFeatureView->update();
			mFeatureViewGraphicData[v].graphicsMiniMap->setFeatureColor(col);
			mFeatureViewGraphicData[v].graphicsMiniMap->update(mFeatureViewGraphicData[v].graphicsMiniMap->boundingRect());
			mFeatureMiniMap->update();
			break;
		}
	}

	// update fragment colors in annotations graph
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFragmentRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mFragmentRects[v][i].names.size(); k++)
			{
				if (QString::compare(mFragmentRects[v][i].names[k],featurename) == 0)
				{
					mFragmentRects[v][i].colors[k] = col;
				}
			}
		}
	}
/*
	// update fragment colors in file graph
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFileViewFragmentRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mFileViewFragmentRects[v][i].names.size(); k++)
			{
				if (QString::compare(mFileViewFragmentRects[v][i].names[k],featurename) == 0)
				{
					mFileViewFragmentRects[v][i].colors[k] = col;
				}
			}
		}
	}
*/
	// update histogram colors in annotations graph
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mHistogramRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mHistogramRects[v][i].names.size(); k++)
			{
				if (QString::compare(mHistogramRects[v][i].names[k],featurename) == 0)
				{
					mHistogramRects[v][i].colors[k] = col;
				}
			}
		}
	}
/*
	// update histogram colors in file graph
	for(ogdf::node v = mFileGraph.firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFileViewHistogramRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mFileViewHistogramRects[v][i].names.size(); k++)
			{
				if (QString::compare(mFileViewHistogramRects[v][i].names[k],featurename) == 0)
				{
					mFileViewHistogramRects[v][i].colors[k] = col;
				}
			}
		}
	}
*/

	filepreviewscreated = false;	// in order to repaint preview pixmaps

	// recreate file graph
	collapseFileViewNode();
}

void SMVMain::setFeaturesTransparent()
{
	mTransSlider->setSliderPosition(mTransSlider->minimum());
	CodeColorTransparency(mTransSlider->minimum());
}

void SMVMain::setFeaturesOpaque()
{
	mTransSlider->setSliderPosition(mTransSlider->maximum());
	CodeColorTransparency(mTransSlider->maximum());
}

void SMVMain::updateFeatureStatus(QString featurename, ogdf::node featurenode, bool active)
{
	// update status visualization of feature nodes
	mFeatureViewGraphicData[featurenode].graphics->setActivation(active);
	mFeatureViewGraphicData[featurenode].graphics->update();
	mFeatureViewGraphicData[featurenode].graphicsMiniMap->setActivation(active);
	mFeatureViewGraphicData[featurenode].graphicsMiniMap->update();

	// update activation status in feature list
	for (unsigned int i = 0; i < mFeatureList.size(); i++)
	{
		if (QString::compare(mFeatureList[i].name, featurename) == 0)
		{
			mFeatureList[i].isActive = active;
		}
	}

	// update fragment activation status in annotations graph
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		for (unsigned int i = 0; i < mFragmentRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mFragmentRects[v][i].names.size(); k++)
			{
				if (QString::compare(mFragmentRects[v][i].names[k],featurename) == 0)
				{
					mFragmentRects[v][i].isActive[k] = active;
				}
			}
		}
	}

	// update histogram activation status in annotations graph
	for(ogdf::node v = mAnnotationsGraph->firstNode(); v; v = v->succ())
	{
		if (mNodeType[v] != NODETYPE_FILE)
			continue;

		/*
		// debug
		qDebug() << "before calculation....";
		for (unsigned int i = 0; i < mHistogramRects[v].size(); i++)
		{
			qDebug() << "name = " << mHistogramRects[v][i].names[0];
			qDebug() << "active = " << mHistogramRects[v][i].isActive[0];
			qDebug() << "y_start = " << mHistogramRects[v][i].y_start;
			qDebug() << "y_end = " << mHistogramRects[v][i].y_end;
		}
		qDebug() << "calculating....";
		*/

		for (unsigned int i = 0; i < mHistogramRects[v].size(); i++)
		{
			for (unsigned int k = 0; k < mHistogramRects[v][i].names.size(); k++)
			{
				if (QString::compare(mHistogramRects[v][i].names[k],featurename) == 0)
				{
					mHistogramRects[v][i].isActive[k] = active;
				}
			}
		}

		if (mHistogramRects[v].size() <= 1)	// continue with next file node, if there is at most one fragment
			continue;

		// recalculate histograms
		
		if (mHistogramRects[v].size() > 1) // otherwise nothing to sort
		{
			// sort depending on activation
			bool switched;
			do
			{
				switched = false;
				for (unsigned int index = 0; index < mHistogramRects[v].size()-1; index++)
				{
					if ( ( mHistogramRects[v][index+1].isActive[0] == true )	&& 
						(mHistogramRects[v][index].isActive[0] == false ) ) // if fragment a is active and b is not
					{
						//qDebug() << "switch " << mHistogramRects[v][index+1].names[0] << " with " << mHistogramRects[v][index].names[0];
						FragmentRect temp_frag(mHistogramRects[v][index]);
						mHistogramRects[v][index] = mHistogramRects[v][index+1];
						mHistogramRects[v][index+1] = temp_frag;
						switched = true;
					}
				}
			} while (switched == true);
		}

		/*
		qDebug() << "sorted list";
		for (unsigned int index = 0; index < mHistogramRects[v].size(); index++)
		{
			qDebug() << mHistogramRects[v][index].names[0] << " (active = " << mHistogramRects[v][index].isActive[0] << ")";
		}
		*/

		// search last position of active fragment
		if (mHistogramRects[v].size() > 1)							// do not continue, if nothing to sort
		{
			if (mHistogramRects[v][0].isActive[0] == true)			// do not continue, if all fragments are deactivated
			{
				unsigned int index;
				for (index = 0; index < mHistogramRects[v].size()-1; index++)
				{
					if (mHistogramRects[v][index+1].isActive[0] == false)
						break;
				}

				//qDebug() << "last index  = " << index;

				// sort fragments in interval 0..index

				if (index > 0) // nothing to sort, if only one active element (index == 0)
				{
					bool switched;
					do
					{
						switched = false;
						for (unsigned int i = 0; i < index-1; i++)
						{
							// if fragment a is shorter than b
							if ( mHistogramRects[v][i].length < mHistogramRects[v][i+1].length )
							{
								FragmentRect temp_frag(mHistogramRects[v][i]);
								mHistogramRects[v][index] = mHistogramRects[v][i+1];
								mHistogramRects[v][i+1] = temp_frag;
								switched = true;
							}
						}
					} while (switched == true);
				}

				/*
				qDebug() << "sorted list";
				for (unsigned int index = 0; index < mHistogramRects[v].size(); index++)
				{
					qDebug() << mHistogramRects[v][index].names[0] << " (active = " << mHistogramRects[v][index].isActive[0] << ")";
				}
				*/

				// get length
				qreal hist_length = 0;
				if (mFileViewHistogramTotalLengths[v] > 0)
					hist_length = ( (qreal)mFileLength[v] / (qreal)mFileViewHistogramAbsoluteLengths[v] ) * (qreal)mFileViewHistogramTotalLengths[v]; 

				// calculate y-coordinates for histogram
				int cumulative_pos = 0;
				qreal y = 0.0;
				if (hist_length > 0.0)
				{
					for (unsigned int i = 0; i < mHistogramRects[v].size(); i++)
					{
						int start_pos = cumulative_pos;
						//qreal y_start = qreal(cumulative_pos) / qreal(mFileLength[v]);
						mHistogramRects[v][i].y_start = 0.99*y;	// start a little lower to avoid visual holes
						cumulative_pos += mHistogramRects[v][i].length;
						//qreal y_end = qreal(cumulative_pos) / qreal(mFileLength[v]);
						y = qreal(cumulative_pos) / qreal(hist_length);
						mHistogramRects[v][i].y_end = y;

						//qDebug() << "name = " << mHistogramRects[v][i].names[0];
						//qDebug() << "active = " << mHistogramRects[v][i].isActive[0];
						//qDebug() << "y_start = " << mHistogramRects[v][i].y_start;
						//qDebug() << "y_end = " << mHistogramRects[v][i].y_end;
					}
				}
			}
		}
	}

	filepreviewscreated = false;	// in order to repaint preview pixmaps
	collapseFileViewNode();			// recreate file graph
}