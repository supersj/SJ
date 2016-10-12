#include "qgis_dev.h"

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGridLayout>
#include <QFont>
#include <QLineEdit>
#include <QToolButton>
#include <QMargins>
#include <qdebug.h>
#include <QBitmap>
// QGis include
#include <qgsvectorlayer.h>
#include <qgsnewvectorlayerdialog.h>
#include <qgsmaplayerregistry.h>
#include <qgsrasterlayer.h>
#include <qgsproject.h>
#include <qgslayertreemodel.h>
#include <qgsapplication.h>
#include <qgslayertreelayer.h>
#include <qgslayertreegroup.h>
#include <qgslayertreeregistrybridge.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgsattributedialog.h>
#include <qgscursors.h>
#include <qgsproviderregistry.h>
#include <qgsmessagebaritem.h>


// for attribute table
#include <qgsfeaturelistview.h>
#include <qgsattributetableview.h>
#include <qgsattributetablemodel.h>
#include <qgsfeaturelistmodel.h>
#include <qgsvectorlayercache.h>
#include <qgsattributedialog.h>
#include <qgseditorwidgetfactory.h>

// for layer symbol
#include <qgssymbollayerv2.h>
#include <qgssymbolv2.h>
#include <qgsmarkersymbollayerv2.h>
#include <qgsvectorlayerrenderer.h>
#include <qgsrendercontext.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgssymbollayerv2.h>
#include <qgsmapcanvas.h>
#include <qgsmapoverviewcanvas.h>
#include <qgscategorizedsymbolrendererv2.h>


// for map tools
#include <qgsmaptool.h>
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolidentify.h>
#include <qgsmaptoolpan.h>

// for open source map


#include "qgsmessagebar.h"

// for diagrams
#include <qgshistogramdiagram.h>
#include <qgsdiagramrendererv2.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorsimplifymethod.h>

#include <qgsoptionsdialogbase.h>
#include "qgis_devlayertreeviewmenuprovider.h"
#include <qgslabel.h>
qgis_dev* qgis_dev::sm_instance = 0;

qgis_dev::qgis_dev( QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    /*if ( sm_instance )
    {
        sm_instance->show();
    }
    sm_instance = this;*/

    ui.setupUi( this );

    //! 初始化map canvas, 并装入layout中
    m_mapCanvas = new QgsMapCanvas();
    m_mapCanvas->enableAntiAliasing( true );
    m_mapCanvas->setCanvasColor( QColor( 255, 255, 255 ) );

    //! map tool
    m_mapToolAddFeature = new qgis_dev_addFeatureTool( m_mapCanvas );
    m_mapCanvas->setMapTool( m_mapToolAddFeature );
    m_mapToolAddFeature->activate();

    //! 初始化图层管理器
    //! 初始化信息显示条
    m_infoBar = new QgsMessageBar( this );
    m_infoBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
    m_layerTreeView = new QgsLayerTreeView( this );
    initLayerTreeView();


    //! 布局
    QWidget* centralWidget = this->centralWidget();
    QGridLayout* centralLayout = new QGridLayout( centralWidget );
    centralLayout->addWidget( m_mapCanvas, 0, 0, 1, 1 );
    centralLayout->addWidget( m_infoBar, 1, 0, 1, 1 );

    createStatusBar();
    createOverview();
    // connections
    connect( ui.actionNew_Vector, SIGNAL( triggered() ), this, SLOT( newVectorLayer() ) );
    connect( ui.actionAdd_Vector, SIGNAL( triggered() ), this, SLOT( addVectorLayers() ) );
    connect( ui.actionAdd_Raster, SIGNAL( triggered() ), this, SLOT( addRasterLayers() ) );
}

qgis_dev::~qgis_dev()
{

}

void qgis_dev::addVectorLayer( QString vecLayerPath, QString basename, QString providerKey )
{
    QgsVectorLayer* vecLayer = new QgsVectorLayer( vecLayerPath, basename, providerKey, false );
    if ( !vecLayer->isValid() )
    {
        QMessageBox::critical( this, "error", "layer is invalid" );
        return;
    }

    QgsMapLayerRegistry::instance()->addMapLayer( vecLayer );
    mapCanvasLayerSet.append( vecLayer );
    m_mapCanvas->setExtent( vecLayer->extent() );
    m_mapCanvas->setLayerSet( mapCanvasLayerSet );
    m_mapCanvas->setVisible( true );
    m_mapCanvas->freeze( false );
    m_mapCanvas->refresh();
    m_mapCanvas->setCurrentLayer( vecLayer );
}

void qgis_dev::newVectorLayer()
{
    QString enc;
    QString fileName = QgsNewVectorLayerDialog::runAndCreateLayer( this, &enc );

    if ( !fileName.isEmpty() )
    {
        //then add the layer to the view
        QStringList fileNames;
        fileNames.append( fileName );
        //todo: the last parameter will change accordingly to layer type
        addVectorLayers( );
    }
    else if ( fileName.isNull() )
    {
        QLabel *msgLabel = new QLabel( tr( "Layer creation failed. Please check the <a href=\"#messageLog\">message log</a> for further information." ), messageBar() );
        msgLabel->setWordWrap( true );
        //connect( msgLabel, SIGNAL( linkActivated( QString ) ), mLogDock, SLOT( show() ) );
        QgsMessageBarItem *item = new QgsMessageBarItem( msgLabel, QgsMessageBar::WARNING );
        messageBar()->pushItem( item );
    }
}
void qgis_dev::addVectorLayers()
{
    QString filename = QFileDialog::getOpenFileName( this, tr( "open vector" ), "", "*.shp" );
    if ( filename == QString::null ) { return;}

    QFileInfo fi( filename );
    QString basename = fi.baseName();
    QgsVectorLayer* vecLayer = new QgsVectorLayer( filename, basename, "ogr", false );
    if ( !vecLayer->isValid() )
    {
        QMessageBox::critical( this, "error", "layer is invalid" );
        return;
    }

    addVectorLayer( filename, basename, "ogr" );
}

void qgis_dev::addRasterLayers()
{
    QString filename = QFileDialog::getOpenFileName( this, tr( "open vector" ), "", "*.bmp" );
    QStringList temp = filename.split( QDir::separator() );
    QString basename = temp.at( temp.size() - 1 );
    QgsRasterLayer* rasterLayer = new QgsRasterLayer( filename, basename, "gdal", false );
    if ( !rasterLayer->isValid() )
    {
        QMessageBox::critical( this, "error", "layer is invalid" );
        return;
    }

    QgsMapLayerRegistry::instance()->addMapLayer( rasterLayer );
    mapCanvasLayerSet.append( rasterLayer );
    m_mapCanvas->setExtent( rasterLayer->extent() );
    m_mapCanvas->setLayerSet( mapCanvasLayerSet );
    m_mapCanvas->setVisible( true );
    m_mapCanvas->freeze( false );
    m_mapCanvas->refresh();
    fullHistogramStretch();
}

void qgis_dev::initLayerTreeView()
{
    QgsLayerTreeModel* model = new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), this );
    model->setFlag( QgsLayerTreeModel::AllowNodeRename );
    model->setFlag( QgsLayerTreeModel::AllowNodeReorder );
    model->setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility );
    model->setFlag( QgsLayerTreeModel::ShowLegendAsTree );
    model->setAutoCollapseLegendNodes( 10 );

    m_layerTreeView->setModel( model );

    // 添加右键菜单
    m_layerTreeView->setMenuProvider( new qgis_devLayerTreeViewMenuProvider( m_layerTreeView, m_mapCanvas ) );

    connect( QgsProject::instance()->layerTreeRegistryBridge(), SIGNAL( addedLayersToLayerTree( QList<QgsMapLayer*> ) ),
             this, SLOT( autoSelectAddedLayer( QList<QgsMapLayer*> ) ) );

    // 设置这个路径是为了获取图标文件
    QString iconDir = "/home/qgis/Desktop/images/themes/default/";

    // add group tool button
    QToolButton * btnAddGroup = new QToolButton();
    btnAddGroup->setAutoRaise( true );
    btnAddGroup->setIcon( QIcon( iconDir + "mActionAdd.png" ) );

    btnAddGroup->setToolTip( tr( "Add Group" ) );
    connect( btnAddGroup, SIGNAL( clicked() ), m_layerTreeView->defaultActions(), SLOT( addGroup() ) );

    // expand / collapse tool buttons
    QToolButton* btnExpandAll = new QToolButton();
    btnExpandAll->setAutoRaise( true );
    btnExpandAll->setIcon( QIcon( iconDir + "mActionExpandTree.png" ) );
    btnExpandAll->setToolTip( tr( "Expand All" ) );
    connect( btnExpandAll, SIGNAL( clicked() ), m_layerTreeView, SLOT( expandAll() ) );

    QToolButton* btnCollapseAll = new QToolButton();
    btnCollapseAll->setAutoRaise( true );
    btnCollapseAll->setIcon( QIcon( iconDir + "mActionCollapseTree.png" ) );
    btnCollapseAll->setToolTip( tr( "Collapse All" ) );
    connect( btnCollapseAll, SIGNAL( clicked() ), m_layerTreeView, SLOT( collapseAll() ) );

    // remove item button
    QToolButton* btnRemoveItem = new QToolButton();
    // btnRemoveItem->setDefaultAction( this->m_actionRemoveLayer );
    btnRemoveItem->setAutoRaise( true );


    // 按钮布局
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins( QMargins( 5, 0, 5, 0 ) );
    toolbarLayout->addWidget( btnAddGroup );
    toolbarLayout->addWidget( btnCollapseAll );
    toolbarLayout->addWidget( btnExpandAll );
    toolbarLayout->addWidget( btnRemoveItem );
    toolbarLayout->addStretch();

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->setMargin( 0 );
    vboxLayout->addLayout( toolbarLayout );
    vboxLayout->addWidget( m_layerTreeView );

    // 装进 widget中
    m_layerTreeDock = new QDockWidget( tr( "Layers" ), this );
    m_layerTreeDock->setObjectName( "Layers" );
    m_layerTreeDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

    QWidget* w = new QWidget();
    w->setLayout( vboxLayout );
    m_layerTreeDock->setWidget( w );
    addDockWidget( Qt::LeftDockWidgetArea, m_layerTreeDock );

    // 连接地图画布和图层管理器
    m_layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge( QgsProject::instance()->layerTreeRoot(), m_mapCanvas, this );
    connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ),
             m_layerTreeCanvasBridge, SLOT( writeProject( QDomDocument& ) ) );
    connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ),
             m_layerTreeCanvasBridge, SLOT( readProject( QDomDocument ) ) );



}

void qgis_dev::createStatusBar()
{
    statusBar()->setStyleSheet( "QStatusBar::item {border: none;}" );

    //! 添加进度条
    m_progressBar = new QProgressBar( statusBar() );
    m_progressBar->setObjectName( "m_progressBar" );
    m_progressBar->setMaximum( 100 );
    m_progressBar->show();
    m_progressBar->setAlignment(Qt::AlignLeft);
    statusBar()->addPermanentWidget( m_progressBar, 1 );
    connect( m_mapCanvas, SIGNAL( renderStarting() ), this, SLOT( canvasRefreshStarted() ) );
    connect( m_mapCanvas, SIGNAL( mapCanvasRefreshed() ), this, SLOT( canvasRefreshFinished() ) );

    QFont myFont( "Arial", 9 );
    statusBar()->setFont( myFont );

    //! 添加坐标显示标签
    m_coordsLabel = new QLabel( QString(), statusBar() );
    m_coordsLabel->setObjectName( "m_coordsLabel" );
    m_coordsLabel->setFont( myFont );
    m_coordsLabel->setMinimumWidth( 10 );
    m_coordsLabel->setMargin( 3 );
    m_coordsLabel->setAlignment( Qt::AlignCenter );
    m_coordsLabel->setFrameStyle( QFrame::NoFrame );
    m_coordsLabel->setText( tr( "Coordinate:" ) );
    m_coordsLabel->setToolTip( tr( "Current map coordinate" ) );
    statusBar()->addPermanentWidget( m_coordsLabel, 0 );

    //! 坐标编辑标签
    m_coordsEdit = new QLineEdit( QString(), statusBar() );
    m_coordsEdit->setObjectName( "m_coordsEdit" );
    m_coordsEdit->setFont( myFont );
    m_coordsEdit->setMinimumWidth( 10 );
    m_coordsEdit->setMaximumWidth( 300 );
    m_coordsEdit->setContentsMargins( 0, 0, 0, 0 );
    m_coordsEdit->setAlignment( Qt::AlignCenter );
    statusBar()->addPermanentWidget( m_coordsEdit, 0 );
    connect( m_coordsEdit, SIGNAL( returnPressed() ), this, SLOT( userCenter() ) );

    //! 比例尺标签
    m_scaleLabel = new QLabel( QString(), statusBar() );
    m_scaleLabel->setObjectName( "m_scaleLabel" );
    m_scaleLabel->setFont( myFont );
    m_scaleLabel->setMinimumWidth( 10 );
    m_scaleLabel->setMargin( 3 );
    m_scaleLabel->setAlignment( Qt::AlignCenter );
    m_scaleLabel->setFrameStyle( QFrame::NoFrame );
    m_scaleLabel->setText( tr( "Scale" ) );
    m_scaleLabel->setToolTip( tr( "Current map scale" ) );
    statusBar()->addPermanentWidget( m_scaleLabel, 0 );

    m_scaleEdit = new QgsScaleComboBox( statusBar() );
    m_scaleEdit->setObjectName( "m_scaleEdit" );
    m_scaleEdit->setFont( myFont );
    m_scaleEdit->setMinimumWidth( 10 );
    m_scaleEdit->setMaximumWidth( 100 );
    m_scaleEdit->setContentsMargins( 0, 0, 0, 0 );
    m_scaleEdit->setToolTip( tr( "Current map scale (formatted as x:y)" ) );
    statusBar()->addPermanentWidget( m_scaleEdit, 0 );
    connect( m_scaleEdit,  SIGNAL( scaleChanged() ), this, SLOT( userScale() ) );


}

void qgis_dev::autoSelectAddedLayer( QList<QgsMapLayer*> layers )
{
    if ( layers.count() )
    {
        QgsLayerTreeLayer* nodeLayer = QgsProject::instance()->layerTreeRoot()->findLayer( layers[0]->id() );

        if ( !nodeLayer )
        {
            return;
        }

        QModelIndex index = m_layerTreeView->layerTreeModel()->node2index( nodeLayer );
        m_layerTreeView->setCurrentIndex( index );
    }
}

void qgis_dev::canvasRefreshStarted()
{
    showProgress(-1,0);
}

void qgis_dev::canvasRefreshFinished()
{
    showProgress(0,0);
}

void qgis_dev::addDockWidget( Qt::DockWidgetArea area, QDockWidget* dockwidget )
{
    QMainWindow::addDockWidget( area, dockwidget );
    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    dockwidget->show();
    m_mapCanvas->refresh();
}

void qgis_dev::histogramStretch( bool visibleAreaOnly, QgsRaster::ContrastEnhancementLimits theLimits )
{
    QgsMapLayer* myLayer = m_layerTreeView->currentLayer();
    if ( !myLayer ) // 判断是否为地图图层
    {
        return;
    }

    QgsRasterLayer* myRasterLayer = qobject_cast<QgsRasterLayer*>( myLayer );
    if ( !myRasterLayer ) // 判断是否为栅格图层
    {
        return;
    }

    QgsRectangle myRectangle;
    if ( visibleAreaOnly ) // 这里控制是否仅用当前可见范围的值域进行拉伸
    {
        myRectangle = m_mapCanvas->mapSettings().outputExtentToLayerExtent( myRasterLayer, m_mapCanvas->extent() );
    }
    // 这一句是关键
    qDebug()<<"get here";
    myRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, theLimits, myRectangle );
    m_mapCanvas->refresh();
}

void qgis_dev::adjustBrightnessContrast( int delta, bool updateBrightness = true  )
{
    foreach( QgsMapLayer* layer, m_layerTreeView->selectedLayers() ) // 遍历所有选择的图层
    {
        if ( !layer )// 判断是否为地图图层
        {
            return;
        }

        QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
        if ( !rasterLayer )// 判断是否为栅格图层
        {
            return;
        }

        // 这里是关键，用QgsBrightnessFilter类控制亮度与对比度
        QgsBrightnessContrastFilter* brightnessFilter = rasterLayer->brightnessFilter();
        if ( updateBrightness )
        {
            brightnessFilter->setBrightness( brightnessFilter->brightness() + delta );
        }
        else
        {
            brightnessFilter->setContrast( brightnessFilter->contrast() + delta );
        }

        rasterLayer->triggerRepaint(); // 重画栅格图层
    }
}
// 以下是实现
void qgis_dev::localHistogramStretch()
{
    histogramStretch( true, QgsRaster::ContrastEnhancementMinMax );
}

void qgis_dev::fullHistogramStretch()
{
    histogramStretch( false, QgsRaster::ContrastEnhancementMinMax );
}

void qgis_dev::localCumulativeCutStretch()
{
    histogramStretch( true, QgsRaster::ContrastEnhancementCumulativeCut );
}

void qgis_dev::fullCumulativeCutStretch()
{
    histogramStretch( false, QgsRaster::ContrastEnhancementCumulativeCut );
}

void qgis_dev::increaseBrightness()
{
    int step = 1;
    if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
    {
        step = 10;
    }
    adjustBrightnessContrast( step );
}

void qgis_dev::decreaseBrightness()
{
    int step = -1;
    if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
    {
        step = -10;
    }
    adjustBrightnessContrast( step );
}

void qgis_dev::increaseContrast()
{
    int step = 1;
    if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
    {
        step = 10;
    }
    adjustBrightnessContrast( step, false );
}

void qgis_dev::decreaseContrast()
{
    int step = -1;
    if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
    {
        step = -10;
    }
    adjustBrightnessContrast( step, false );
}

void qgis_dev::createOverview()
{
    QgsMapOverviewCanvas* overviewCanvas = new QgsMapOverviewCanvas( NULL, m_mapCanvas );
    overviewCanvas->setWhatsThis( tr( "Map overview canvas." ) );

    QBitmap overviewPanBmp = QBitmap::fromData( QSize( 16, 16 ), pan_bits ); // ������qgscursors.h�ļ���
    QBitmap overviewPanBmpMask = QBitmap::fromData( QSize( 16, 16 ), pan_mask_bits );
    m_overviewMapCursor = new QCursor( overviewPanBmp, overviewPanBmpMask, 0, 0 );
    overviewCanvas->setCursor( *m_overviewMapCursor );

    // װ��ӥ��ͼ��
    m_OverviewDock = new QgsDockWidget( tr( "Overview" ), this );
    m_OverviewDock->setObjectName( "Overview" );
    m_OverviewDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    m_OverviewDock->setWidget( overviewCanvas );
    addDockWidget( Qt::LeftDockWidgetArea, m_OverviewDock );

    m_OverviewDock->setMinimumSize( 200, 200 );

    m_mapCanvas->enableOverviewMode( overviewCanvas );

    // ������ͼ��������������ͼ�㶼���뵽ӥ��ͼ��
    if ( m_layerTreeView )
    {
        foreach( QgsLayerTreeLayer* nodeLayer, m_layerTreeView->layerTreeModel()->rootGroup()->findLayers() )
        {
            nodeLayer->setCustomProperty( "overview", 1 );
        }
    }

}

void qgis_dev::showProgress( int theProgress, int theTotalSteps )
{
  if ( theProgress == theTotalSteps )
  {
    m_progressBar->reset();
    m_progressBar->hide();
  }
  else
  {
    //only call show if not already hidden to reduce flicker
    if ( !m_progressBar->isVisible() )
    {
      m_progressBar->show();
    }
    m_progressBar->setMaximum( theTotalSteps );
    m_progressBar->setValue( theProgress );

    if ( m_progressBar->maximum() == 0 )
    {
      // for busy indicator (when minimum equals to maximum) the oxygen Qt style (used in KDE)
      // has some issues and does not start busy indicator animation. This is an ugly fix
      // that forces creation of a temporary progress bar that somehow resumes the animations.
      // Caution: looking at the code below may introduce mild pain in stomach.
      if ( strcmp( QApplication::style()->metaObject()->className(), "Oxygen::Style" ) == 0 )
      {
        QProgressBar pb;
        pb.setAttribute( Qt::WA_DontShowOnScreen ); // no visual annoyance
        pb.setMaximum( 0 );
        pb.show();
        QApplication::processEvents();
      }
    }

  }
}
