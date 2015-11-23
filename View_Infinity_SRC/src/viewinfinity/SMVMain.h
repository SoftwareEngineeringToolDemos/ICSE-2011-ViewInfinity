

#pragma once

#ifndef SMVMain_H
#define SMVMain_H

#include <QApplication>
#include <QtGui>
#include <QDomDocument>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QDialog>
#include <QColorDialog>
#include <QTextEdit>

#include <ogdf/Basic/Graph.h>
#include <ogdf/Basic/GraphAttributes.h>
#include <ogdf/Basic/GraphCopy.h>

#include <iostream>

// general for all views
class ViewMode;
class NodeIcon;
class GraphNodeType;
class LinearEdgeGraphic;
class FragmentDataItem;

// feature view
class FeatureView;
class FeatureMiniMap;
class FeatureTreeModel;
class FeatureTreeView;
class Feature;
class FeatureViewNodeGraphic;
class FeatureEdgeGraphic;

//file view
class FileView;
class FileMiniMap;
class FileViewNodeGraphic;
class FileMiniMapNodeGraphic;
class FileTreeModel;
class FileTreeView;

//codeview
class CodeView;
class CodeMiniMap;
class SyntaxHighlighter;

class DomModel;

QT_BEGIN_NAMESPACE
class QToolTip;
class QAction;
class QMenu;
class QMenuBar;
class QDomElement;
QT_END_NAMESPACE

//! This is the main class and main window of the View Infinity.
/*!
  The main window holds several dock widgets (toolbars, a stacked widget #pagesWidget as the primary window containing the feature view or file
  view or code view, two tree widgets for features and files, mini maps for each view) and a menu for setting options and opening projects.

  After opening a project the graph structures for the project data are created and graphics scenes for project features and files
  are created. Look at the openFile() function for details.
  
  After that the stacked widget (#pagesWidget) of the SMVMain shows a FeatureView (a graphics view with project features).
  There are also widgets for showing the features and the files as a tree (FeatureTreeView and FileTreeView).
  Additionally, for the primary view, there are mini maps visible in which the current views of the project are shown (FeatureMiniMap, FileMiniMap, CodeMiniMap).
  You can zoom into features, feature trees and source code files to get specific views of the project.
  
  The order of specification process is always as following: 
  - at first activate the features you want to see, by clicking on it (see FeatureView and FeatureViewNodeGraphic for details)
  - zoom into a features or a feature tree via zoomIntoFeature(), the view changes by animation from feature to file view
  - zoom into source code file in order to see the feature code via function FileView::zoomToCodeViewFile()
*/
class SMVMain : public QMainWindow
{
	Q_OBJECT

	//! Contains all possible node types for feature and file graph.
	/*!
		The node types of the nodes are saved in ogdf::NodeArray #mNodeType.
	*/
	enum GraphNodeType{
		NODETYPE_PROJECT, /*!< The node is the project root node. */
		NODETYPE_FILE, /*!< The node represents a file. */
		NODETYPE_FOLDER, /*!< The node represents a folder. */
		NODETYPE_FRAGMENT, /*!< The node represents a fragment of a feature in a specific file. */
		NODETYPE_FEATURE, /*!< The node represents a feature. */
		NODETYPE_UNKNOWN /*!< The node type is unknown (should be avoided). */
	};

	//! Contains possible edge types for feature graph.
	/*!
	  The edge types of the nodes are saved in ogdf::NodeArray #mFeatureNodeType.
	*/
	enum GraphEdgeType{
		EDGETYPE_EMPTY,
		EDGETYPE_OPTIONAL,
		EDGETYPE_ALTERNATIVE,
		EDGETYPE_OBLIGATORY,
		EDGETYPE_OR
	};

	//! Contains view modes of the active project.
	/*!
	  @todo Implementation of a view for UML diagrams.

	  \sa #mViewMode
	*/
	enum ViewMode{
		FEATURE_VIEW, /*!< Is set, if main window shows the feature view. */
		FILE_VIEW, /*!< Is set, if main window shows the file view. */
		CODE_VIEW, /*!< Is set, if main window shows the code view. */
		UML_VIEW, /*!< Is set, if main window shows the UML view.*/
		UNKNOWN /*!< Is set otherwise (should be impossible). */
	};

	//! Contains different graph layouts from Open Graph Drawing Framework.
	/*!
	  @todo Positioning of start and target point of edges according to layouting algorithm. Edges fit only for tree and fast hierarchical algorithm yet.

	  Look into Open Graph Drawing Framework manual for algorithm details and for setting reasonable values for algorithm parameters.

	  \sa setCircularLayout()
	  \sa setFastHierarchicalLayout()
	  \sa setBallonLayout()
	  \sa setTreeLayout()
	  \sa setHierarchicalLayout()
	  \sa setEnergybasedLayout()
	  \sa setOrthogonalLayout()
	*/
	enum Layout{
		HIERARCHICAL, /*!< an Sugiyama layout algorithm with optimal ranking of nodes and optimal hierarchy */
		BALLON, /*!<  child nodes are layouted around parent node, overlapping of edges and nodes possible, works well for medium graphs */
		TREE, /*!< ordinary tree layout, chosen as standard, may lead to large width of graph */
		CIRCULAR, /*!< hierarchies are layouted around a single center without overlapping, leads to long edges very often and low rendering performance */
		ENERGYBASED, /*!< a physically-based layout that needs few space and works well for large graphs */
		ORTHOGONAL, /*!< an planarization layout, does not work well yet */
		FASTHIERARCHICAL /*!< Sugiyama layout: quick computation, may lead to large width of graph, likewise tree layout */
	};

	//! The graphical representation of a feature.
	/*!
	  It consists of two graphical items. The graphics are saved in the ogdf::NodeArray #mFeatureViewGraphicData.
	
	  \sa #mFeatureViewGraphicData
	  \sa createFeatureViewNodeGraphic()
	  \sa getFeatureViewNodeGraphic()
	  \sa updateFeatureViewNodePosition()
	*/
	struct FeatureViewNode{
		FeatureViewNodeGraphic *graphics; /*!< Item is added to the graphics scene in FeatureView. */
		FeatureViewNodeGraphic *graphicsMiniMap; /*!< Item is added to the graphics scene in FeatureMiniMap. */
	};
	
	//! The graphical representation of the connection between two features.
	/*!
	  It consists of two graphical items. The graphics are saved in the ogdf::EdgeArray #mFeatureViewEdgeGraphicData.
	
	  \sa #mFeatureViewEdgeGraphicData
	  \sa createFeatureViewEdgeGraphic()
	  \sa updateFeatureViewEdgePosition()
	*/
	struct FeatureViewLinearEdge{
		FeatureEdgeGraphic *graphics; /*!< Item is added to the graphics scene in FeatureView. */
		FeatureEdgeGraphic *graphicsMiniMap; /*!< Item is added to the graphics scene in FeatureMiniMap. */
	};

	//! The graphical representation of the connection between two files.
	/*!
	  It consists of two graphical items. The graphics are saved in the ogdf::EdgeArray #mFileViewEdgeGraphicData.
	
	  \sa #mFileViewEdgeGraphicData
	  \sa createFileViewEdgeGraphic()
	  \sa updateFileViewEdgePosition()
	*/
	struct FileViewLinearEdge{
		LinearEdgeGraphic *graphics; /*!< Item is added to the graphics scene in FileView. */
		LinearEdgeGraphic *graphicsMiniMap; /*!< Item is added to the graphics scene in FileMiniMap. */
	};

	//! The graphical representation of a file.
	/*!
	  It consists of two graphical items. 
	  The graphics are saved in the ogdf::NodeArray #mFileViewGraphicData.
	
	  \sa #mFileViewGraphicData
	  \sa createFileViewNodeGraphic()
	  \sa getFileViewNodeGraphic()
	  \sa updateFileViewNodePosition()
	  \sa updateFileMiniMapNodePosition()
	*/
	struct FileViewNode{
		FileViewNodeGraphic *graphics; /*!< This graphics item is added to the graphics scene in FileView */
		FileMiniMapNodeGraphic *graphicsMiniMap; /*!< This graphics item is added to the graphics scene in FileMiniMap */
	};

	//! A struct for project features and feature trees.
	/*!
	  A feature in a projects consists of an individual name and color, Additionally it is stored, whether the feature is active or inactive and whether it is
	  shown in the FileView currently.

	  @warning The feature struct has no information, whether the instance is a single feature or a tree of features.
	  The feature class should have a variable to seperate features from feature trees. Currently nodes that represent
	  feature trees also have a feature color. Therefore there is a hack in the #FeatureViewNodeGraphic in order to draw feature trees correctly anyway.
	  When creating the the graphic it is checked, if there are child nodes in the graph for this feature (FeatureViewNodeGraphic::setIsFeature()).
	  If it is a feature tree (NOT a single feature) its graphical representation is painted without color and the it cannot receive color drags (FeatureViewNodeGraphic::dropEvent()).

	*/
	struct Feature{
		QString name; /*!< The name of the feature. It has to be individual in the project. */
		QColor color; /*!< The color of the feature. It has to be individual in the project. */
		bool isActive; /*!< This variable is true, if the feature is active. */
		bool shownInFileView; /*!< This variable is true, if the feature is the one zoomed into. */
	};

	//! A struct for an icon
	/*!
	  All icons are created in the constructor SMVMain() and saved into vector #icons.
	  Currently these are icons for the project node and folder nodes. These are used in class #FileViewNodeGraphics.
	*/
	struct NodeIcon{
		QPixmap	*icon_pm; /*!< A pixmap icon.*/
	};

	//! A struct for a fragment of code
	/*!
		When loading a project all fragments are saved into the ogdf::NodeArray #mFragmentData.

		\sa createAnnotationsGraph()
		\sa mFragmentData
	*/
	struct FragmentDataItem{
		unsigned int length; /*!< The number of characters of the fragment. */
		unsigned int offset; /*!< The number of characters from the beginning of the file to the beginning of the code fragment. */
	};

	//! A struct for a set of of overlapping code fragments.
	/*!
	  Several features can use same areas in the source code. This struct uses vector variables in order to be able to save a set fragments using the same code areas.
	  When loading a project all vectors of fragment rectangles are calculated and saved into the ogdf::NodeArray #mFragmentRects.
	  Vectors of fragment rectangles have to be used, since every source code file can contain several fragments of different code areas.
	  
	  #FragmentRect are also needed for saving the features histograms of source files. In this case a fragment rectangle contains only a single rectangle naturally,
	  since it represents the amount of code belonging to the represented feature in a certain source file. 
	  
	  All fragment rectangles belonging to a certain feature have to be recalculated, if the color of the feature changes. This is done in function updateFeatureColor().

	  \sa createAnnotationsGraph()
	  \sa createFileGraph()
	  \sa updateFeatureColor()
	  \sa mFragmentRects
	  \sa mHistogramRects
	*/
	struct FragmentRect{
		unsigned int offset; /*!<  The number of characters from the beginning of the file to the beginning of the code fragment. */
		unsigned int length; /*!<  The number of characters of the fragment. */
		qreal y_start; /*!< The start coordinate in y-direction in interval [0...1]. This is translated in item coordinates when in use. */
		qreal y_end; /*!< The end coordinate in y-direction in interval [0...1]. This is translated in item coordinates when in use */
		std::vector<QColor> colors; /*!<  A vector with colors of the features in the rectangle. */
		std::vector<QString> names; /*!<  A vector with feature names in the rectangle. */
		std::vector<bool> isActive; /*!<  A vector with activation states of features in the rectangle. */
	};

public:
	// public declarations

	//! The constructor of the class.
	/*!
		When the constructor is called, the main window is set up as follows:
		- All variables are initialized.
		- The toolbars and primary views and mini maps are set up. Therefore a QStackedWidget is created
		and the widgets FeatureView, FileView and CodeView are added.
		-  The size of the views are maximized.
	*/
	SMVMain();

	//! The destructor of the class.
	/*!
	  Removal of program data and exit.
	*/
	~SMVMain(void);
	
	//! Set view mode to given value.
	/*!
	  \param mode view mode to set
	*/
	void		setViewMode(ViewMode mode);

	//! Get the current view mode.
	/*!
	  \return The current view mode.
	*/
	ViewMode	getViewMode();

	//! Get path of loaded annotations file.
	/*!
	  \return The full path as QString leading to annotations.xml of the currently loaded project.
	*/
	QString		getPathAnnotationsXML();

	//! Get path of loaded model file.
	/*!
	  \return The full path as QString leading to model.m of the currently loaded project.
	*/
	QString		getPathModelM();

	//! Get path of loaded project folder.
	/*!
	  \return The path as QString leading to the folder of the currently loaded project.
	*/
	QString		getProjectPath();


	//! Get the graphics item in file view for the given node.
	/*!
	  \param v A node from #FileViewGraphicData.
	  \return The node graphic in FileView for the given node out of the ogdf::NodeArray mFileViewGraphicData.
	*/
	FileViewNodeGraphic* getFileViewNodeGraphic(ogdf::node v);

	//! Get the graphics item in file mini map for the given node.
	/*!
	  \param v A node from #FileViewGraphicData
	  \return The node graphic in file mini map for the given node out of the ogdf::NodeArray mFileViewGraphicData.
	*/
	FileMiniMapNodeGraphic* getFileMiniMapNodeGraphic(ogdf::node v);

	//! Get the graphics item in feature view for the given node.
	/*!
	  \param v A node from #mFeatureViewGraphicData.
	  \return The node graphic in FeatureView for the given node out of the ogdf::NodeArray mFeatureViewGraphicData.
	*/
	FeatureViewNodeGraphic* getFeatureViewNodeGraphic(ogdf::node v);

	// functions for file view

	//! Update position of given node.
	/*!
	  This function is called from a FileViewNodeGraphic, when it is moved.
	  If position has changed, the node attributes in #mFileGA are updated
	  and the position of the graphics item in file mini map.
	  Additionally the edges directing from and to the node are updated by
	  calling updateFileViewEdgePosition() for every such edge.

	  \param v A node from #mFeatureViewGraphicData.
	*/
	void updateFileViewNodePosition(ogdf::node v);

	//! Update position of given node.
	/*!
	  This function is called from a FileMiniMapNodeGraphic, when it is moved.
	  If position has changed the node attributes in #mFileGA are updated
	  and the position of the graphics item in file view.
	  Additionally the edges directing from and to the node are updated by
	  calling updateFileViewEdgePosition() for every such edge.

	  \param v A node from #mFeatureViewGraphicData.
	*/
	void updateFileMiniMapNodePosition(ogdf::node v);

	//! Update position of given edge.
	/*!
	  After node positions have changed the connecting edge has also to be updated.
	  At first the new coordinates of the linear edge is calculated using the bounding rectangles of the node graphics.
	  Afterwards the new positions are updated in #mFileViewEdgeGraphicData for file view and the mini map.

	  \param e An edge from #mFeatureViewGraphicData.
	*/
	void updateFileViewEdgePosition(ogdf::edge e);

	//! Collapse folder in file view and tree view
	/*!
	  A folder is collapsed this way, if the user clicks on the collapse symbol of a file node.

	  The first step of collapsing is finding the collapsing item in the #mFileTreeViewModel.
	  This is done recursive by calling searchModelIndexOfTreeViewNodeRecursive().
	  When it has been found, the folder is being collapsed in FileTreeView.
	  After that collapseFileViewNode() is called in order to recreate FileView.

	  \param nodeLabel The label of the node going to be collapsed.
	  \sa searchModelIndexOfTreeViewNodeRecursive()
	*/
	void collapseFileNode(QString nodeLabel);

	//! Expand folder in file view and tree view
	/*!
	  A folder is expanded this way, if the user clicks on the expand symbol of a file node.
	  
	  The function works analog to collapseFileNode().

	  The first step of expanding is finding the expanding item in the #mFileTreeViewModel.
	  This is done recursive by calling searchModelIndexOfTreeViewNodeRecursive().
	  When it has been found, the folder is being expanded in FileTreeView.
	  After that collapseFileViewNode() is called in order to recreate FileView.
	  There is no function "expandFileViewNode()", since collapseFileViewNode() works for both cases.

	  \param nodeLabel The label of the node going to be expanded.
	  \sa searchModelIndexOfTreeViewNodeRecursive()
	*/
	void expandFileNode(QString nodeLabel);

	//!
	/*!
	*/
	void searchModelIndexOfTreeViewNodeRecursive(QModelIndex idx);

	//!
	/*!
	*/
	void searchAmbiguityPositionOfTreeViewNodeRecursive(QModelIndex idx);

	//! Update file view and minimap after collapsing or expanding a folder.
	/*!
	  When an item in FileView or in FileTreeView is collapsed, the file graph and its graphical
	  representation in the view has to be recreated. 
	  
	  The function works very alike createFileGraph():

	  - All current graphical data is removed from scenes in FileView and FileMiniMap.
	  - The graph attributes of the full project graph is copied into mFileGraph.
	  - Graph node, but folder, file and project nodes are removed.
	  - Project data is copied for all node arrays pointing on mFileGraph.
	  - Fragment rectangles and histograms are deactivated, if the corresponding features are inactive.
	  - Nodes under collapes nodes are removed.
	  - Nodes without active features are removed.
	  - Folders with active features are activated.
	  - Empty folders are removed.
	  - Fragment data, feature data, tooltip data, pixmaps and file lengths are copied for the resting nodes of the file graph.
	  - Preview pixmaps for source code files are repainted, if this in necessary.
	  - Graphical items for nodes and edges are created and added to the file view and mini map.
	  - Graph is layouted and graphical items are repositioned.
	  - Views are centered on collapsed node.

	  This function is also called, if a feature color has changed in order to rebuild the view with changed color.

	  @warning When deleting graphics item from graphics scene in mini map
	  the navigator (FileViewInteractiveRect) musn't be deleted.
  
	*/
	void collapseFileViewNode();

	//!
	/*!
	*/
	void deleteFileViewSubGraph(ogdf::node v);

	//!
	/*!
	*/
	void printFileTreeViewModel();

	//!
	/*!
	*/
	void printFileTreeViewModelRecursive(QModelIndex idx);

	// functions for other views

	//!
	/*!
	*/
	void updateFeatureViewNodePosition(ogdf::node v);

	//!
	/*!
	*/
	void updateFeatureViewEdgePosition(ogdf::edge e);

	//!
	/*!
	*/
	void updateAlternativeFeatures(ogdf::node v, bool aktive);

	//!
	/*!
	*/
	void updateFeatureStates();

	//!
	/*!
	*/
	void updateFeatureColor(QString featurename, QColor col);

	//!
	/*!
	*/
	void updateFeatureStatus(QString featurename, ogdf::node featurenode, bool active);

	// zooming

	//!
	/*!
	*/
	void zoomIntoFeature(QString feature_name);

	// annotations graph
	ogdf::Graph										*mAnnotationsGraph; /*!< */
	ogdf::GraphAttributes							*mAnnotationsGA; /*!< */
	ogdf::NodeArray<GraphNodeType>					mNodeType; /*!< */
	ogdf::NodeArray<bool>							mNodeCollapsed;	 /*!< */			// folding status of graph
	ogdf::NodeArray<FragmentDataItem>				mFragmentData; /*!< */
	ogdf::NodeArray<unsigned int>					mFeatureData; /*!< */
	std::vector<Feature>							mFeatureList; /*!< */
	ogdf::NodeArray<int>							mFileLength; /*!< */
	ogdf::NodeArray<std::vector<FragmentRect>>		mFragmentRects;	/*!< */		// get fragments for files (offset and length in [0..1] and color
	ogdf::NodeArray<std::vector<FragmentRect>>		mHistogramRects; /*!< */// get fragments for files (offset and length in [0..1] and color
	ogdf::NodeArray<unsigned int>					mNodeAmbiguityPositions; /*!< */// avoid ambiguities
	std::vector<ogdf::String>						nodeLabelVector; /*!< */
	std::list<ogdf::String>							nodeLabelList; /*!< */
	std::vector<unsigned int>						nodeLabelCounts; /*!< */
	ogdf::NodeArray<QString>						mToolTipLabels; /*!< */
	ogdf::NodeArray<QPixmap>						mPreviewPixmaps; /*!< */
	
	// DOM
    DomModel										*model; /*!< */
	QDomElement										docElem; /*!< */

	// file graph
	ogdf::Graph										mFileGraph;	 /*!< */				// complete file file graph
	ogdf::GraphAttributes							mFileGA; /*!< */					// attributes of file graph
	ogdf::NodeArray<GraphNodeType>					mFileNodeType;	 /*!< */			// node types of file
	ogdf::NodeArray<bool>							mFileNodeActive;	 /*!< */		// node active or inactive (greyed out) ?
	ogdf::NodeArray<bool>							mFileNodeCollapsed;	 /*!< */		// folding status of file graph
	ogdf::NodeArray<FragmentDataItem>				mFileViewFragmentData; /*!< */
	ogdf::NodeArray<unsigned int>					mFileViewFeatureData; /*!< */
	ogdf::NodeArray<int>							mFileViewFileLength; /*!< */
	ogdf::NodeArray<std::vector<FragmentRect>>		mFileViewFragmentRects;	 /*!< */	// get fragments for files (offset and length in [0..1] and color
	ogdf::NodeArray<std::vector<FragmentRect>>		mFileViewHistogramRects; /*!< */	// get histograms for files (offset and length in [0..1] and color
	ogdf::NodeArray<int>							mFileViewHistogramTotalLengths;		/*!< The total (cumulative) length of all fragments of a file node. Source code used from more than one feature is counted several times. (set union)*/
	ogdf::NodeArray<int>							mFileViewHistogramAbsoluteLengths; /*!< The absolute length of all fragments of a file node. Source code used from more than one feature is counted only once (intersection). */

	ogdf::NodeArray<FileViewNode>					mFileViewGraphicData; /*!< */		// node graphics of file graph
	ogdf::EdgeArray<FileViewLinearEdge>				mFileViewEdgeGraphicData; /*!< */	// edge graphics of file graph
	ogdf::NodeArray<bool>							mFileNodeToBeDeleted; /*!< */		// nodes for deleting
	ogdf::NodeArray<int>							mFileNodeAmbiguityPositions; /*!< */		// avoid ambiguities
	ogdf::NodeArray<QString>						mFileToolTipLabels; /*!< */
	ogdf::NodeArray<QPixmap>						mFileViewPreviewPixmaps; /*!< */
	
	QPointF											mFileViewEventNodePosition; /*!< */	// contents position of node, which triggered action
	ogdf::String									mFileViewEventNodeLabel; /*!< */	// contents label of node, which triggered action
	unsigned int									mFileViewEventNodeAmbigPosition; /*!< */ // contents position in "ambiguity" vector

	// file tree view
	FileTreeView									*mFileTreeView; /*!< */
	QDockWidget										*mFileTreeViewDock; /*!< */
	FileTreeModel									*mFileTreeViewModel; /*!< */
	QString											strFileTree; /*!< */
	QString											strSearchedNode; /*!< */			// searched node label for synchronize file view with tree view
	std::vector<QModelIndex>						idxSearchedNodes;	 /*!< */		// searched node index for synchronize file view with tree view
	unsigned int									positionSearchedNode; /*!< */
	QModelIndex										idxSearchedNode; /*!< */
	ogdf::node										searchedNode; /*!< */
	QString											featureInFileView; /*!< */

	// feature tree view
	FeatureTreeView									*mFeatureTreeView; /*!< */
	QDockWidget										*mFeatureTreeViewDock; /*!< */
	FeatureTreeModel								*mFeatureTreeViewModel; /*!< */
	QString											strFeatureTree; /*!< */

	// feature view
	ogdf::Graph										*mFeatureGraph; /*!< */
	ogdf::GraphAttributes							*mFeatureGA; /*!< */
	ogdf::NodeArray<GraphEdgeType>					mFeatureNodeType; /*!< */
	ogdf::NodeArray<FeatureViewNode>				mFeatureViewGraphicData; /*!< */
	FeatureViewLinearEdge							edge; /*!< */
	ogdf::EdgeArray<FeatureViewLinearEdge>			mFeatureViewEdgeGraphicData; /*!< */
	ogdf::NodeArray<bool>							mNodeActive; /*!< */				// active or non-active status of a node	
	ogdf::NodeArray<bool>							mFeatureNodeSelected; /*!< */		// feature node selected for file view

	// folgende variablen benötigt?
	QPointF											mFeatureViewEventNodePosition; /*!< */	// contents position of node, which triggered action
	ogdf::String									mFeatureViewEventNodeLabel; /*!< */		// contents label of node, which triggered action

	ViewMode										mViewMode; /*!< */

	QWidget											*mCurrentView; /*!< */
	FeatureView										*mFeatureView; /*!< */
	FeatureMiniMap									*mFeatureMiniMap; /*!< */
	bool											feature_mini_map_first_time; /*!< */
	QDockWidget										*mFeatureMiniMapDock; /*!< */
	FileView										*mFileView; /*!< */
	FileMiniMap										*mFileMiniMap; /*!< */
	QDockWidget										*mFileMiniMapDock; /*!< */
	QAction											*showFullFileGraphAction; /*!< */
	CodeMiniMap										*mCodeMiniMap; /*!< */
	QDockWidget										*mCodeMiniMapDock; /*!< */

	QColorDialog									*mColorDialog; /*!< */

	// code view
	CodeView										*editor; /*!< */
    SyntaxHighlighter								*highlighter; /*!< */
	QSlider											*mTransSlider; /*!< */	// color transparency slider

	//buttons
	QAction											*NodeButton; /*!< */
	QAction											*FileButton; /*!< */
	QAction											*CodeButton; /*!< */
	QAction											*ZMButton; /*!< */
	QAction											*ZPButton; /*!< */
	QAction											*Stripe1; /*!< */
	QAction											*Stripe2; /*!< */
	QAction											*Stripe3; /*!< */
	QAction											*Stripe4; /*!< */
	QAction											*Stripe5; /*!< */
	QAction											*Stripe6; /*!< */
	QAction											*Stripe7; /*!< */
	QAction											*Stripe8; /*!< */
	QAction											*Stripe9; /*!< */
	QAction											*Stripe10; /*!< */
	QAction											*aboutAct; /*!< */
	QAction											*ColorButton; /*!< */
	QAction											*TransparencyButton; /*!< */
	QAction											*OpacityButton; /*!< */

	bool											projectLoaded; /*!< */
	bool											showFullGraph; /*!< */

    QStackedWidget									*pagesWidget; /*!< */

	std::vector<NodeIcon>							icons; /*!< */

	QPixmap											*filescenepreview; /*!< Variable holds preview of graphics scene in file view. */
	bool											renderfileview; /*!< Is true, if a preview pixmap of the file view has to be rendered. */

// Qt slots
public slots:

	//!
	/*!
	*/
	void treeViewCollapsSlot(QModelIndex idx);

	//!
	/*!
	*/
	void treeViewExpandSlot(QModelIndex idx);

	//!
	/*!
	*/
	void featureTreeViewCollapsSlot(QModelIndex idx);

	//!
	/*!
	*/
	void featureTreeViewExpandSlot(QModelIndex idx);

	//!
	/*!
	*/
	void stripe0();

	//!
	/*!
	*/
	void stripe1();

	//!
	/*!
	*/
	void stripe2();

	//!
	/*!
	*/
	void stripe3();

	//!
	/*!
	*/
	void stripe4();

	//!
	/*!
	*/
	void stripe5();

	//!
	/*!
	*/
	void stripe6();

	//!
	/*!
	*/
	void stripe7();

	//!
	/*!
	*/
	void stripe8();

	//!
	/*!
	*/
	void stripe9();

	//!
	/*!
	*/
	void stripe10();

	//!
	/*!
	*/
	void zmButton();

	//!
	/*!
	*/
	void zpButton();

	//!
	/*!
	*/
	void openColorDialog();

	//!
	/*!
	*/
	void setFullFileGraph();

	// fade in views

	//!
	/*!
	*/
	void directFadeInFeatureView();

	//!
	/*!
	*/
	void directFadeInFileView();

	//!
	/*!
	*/
	void directFadeInCodeView();

	// fade in views

	//!
	/*!
	*/
	void fadeInFeatureView();

	//!
	/*!
	*/
	void fadeInFileView();

	//!
	/*!
	*/
	void fadeInCodeView();

	//!
	/*!
	*/
	void showFeatureView();

	//!
	/*!
	*/
	void showFileView();

	//!
	/*!
	*/
	void showCodeView();

private slots:

//! Slot for loading a project.
/*!
  This slot is called from #fileMenu .

  At first a #QFileDialog is opened to get the path of project folder (#path_project) containing the annotations.xml (#path_annotations_xml) and model.m (#path_model_m).
  If there was a project loaded already, this data is removed and memory released.
  After that the model is read into a #DomModel using the function loadAnnotationsXML().
  Now createAnnotationsGraph() is used to create one large graph containing all project data.

  Now the functions createAnnotationsGraph(), createFeatureGraph(),	createFileGraph() save all project specific data in graph structure (#ogdf::Graph, #ogdf::GraphAttributes and #ogdf::NodeArray)
  using the same node pointers. These pointers are initially created and saved in the graph #mAnnotationsGraph with its attributes #mAnnotationsGA.
  This graph holds all data of the feature model which are read by the #ReadModel class from the input files annoations.xml and model.m .

  The file diagram is updated for the current (initial) feature configuration by calling the function collapseFileViewNode();

  Finally the view is scaled to show the current view in in full size (e.g. FeatureView::showFullScene())

  \sa loadAnnotationsXML()
  \sa createAnnotationsGraph()
  \sa createFeatureGraph()
  \sa createFileGraph()
  \sa collapseFileViewNode()
*/
	void openFile();

	//!
	/*!
	*/
	void setCircularLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setFastHierarchicalLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setBallonLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setTreeLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setHierarchicalLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setEnergybasedLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void setOrthogonalLayout(ViewMode view = UNKNOWN);

	//!
	/*!
	*/
	void writeGML();

	//!
	/*!
	*/
	void about();

	//!
	/*!
	*/
	void CodeColorTransparency(int);

	//!
	/*!
	*/
	void setFeaturesTransparent();

	//!
	/*!
	*/
	void setFeaturesOpaque();

protected:

	//!
	/*!
	*/
	void mousePressEvent(QMouseEvent *event);

	//!
	/*!
	*/
	void wheelEvent(QWheelEvent *event);

	//!
	/*!
	*/
	void paintEvent(QPaintEvent*);


private:

	//!
	/*!
	*/
	void createActions();

	//!
	/*!
	*/
	void createMenu();

	//!
	/*!
	*/
	void createToolBars();

	//!
	/*!
	*/
	void createStatusBar();

	//!
	/*!
	*/
	void createDockWindows();

	//!
	/*!
	*/
	void loadAnnotationsXML();

	//!
	/*!
	*/
	void createAnnotationsGraph();

	//!
	/*!
	*/
	void createAnnotationsGraphRecursive(QDomElement docElem, ogdf::node parent);

	//! create list of all features
	/*!
	*/
	void createFeatureList(QDomElement docElem);

	// create file graph
	//!
	/*!
	*/
	void createFileGraph();

	//!
	/*!
	*/
	void createFileGraphString(ogdf::node v, int level);

	//!
	/*!
	*/
	void createFileViewNodeGraphic(ogdf::node v);

	//!
	/*!
	*/
	void createFileViewEdgeGraphic(ogdf::edge e);

	//!
	/*!
	*/
	void activateParent(ogdf::node v);

	// create feature graph
	//!
	/*!
	*/
	void createFeatureGraph();

	//!
	/*!
	*/
	void createFeatureGraphString(ogdf::node v, int level);

	//!
	/*!
	*/
	void createFeatureViewNodeGraphic(ogdf::node v);

	//!
	/*!
	*/
	void createFeatureViewEdgeGraphic(ogdf::edge e);

	//!
	/*!
	*/
	void printFeatureList();

	//!
	/*!
	*/
	void printGraph();
	
	// declaration of general variables
	QMenu			*fileMenu; /*!< */
	QMenu			*ansichtMenu; /*!< */
	QMenu			*windowMenu; /*!< */
	QMenu			*helpMenu; /*!< */

	QToolBar		*viewToolBar; /*!< */
    QToolBar		*zoomToolBar; /*!< */
	QToolBar		*extrasToolBar; /*!< */
	QToolBar		*opacityToolBar; /*!< */

	QString			path_project; /*!< */
	QString			path_annotations_xml; /*!< */
	QString			path_model_m; /*!< */

	Layout			mFileViewLayout; /*!< */
	Layout			mFeatureViewLayout; /*!< */

	bool			filenodesneedupdate; /*!< */
	bool			filepreviewscreated; /*!< */
};

#endif
