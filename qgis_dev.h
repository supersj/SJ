#ifndef QGIS_DEV_H
#define QGIS_DEV_H

#include <QtGui/QMainWindow>
#include "ui_qgis_dev.h"

#include <qgis_dev_addfeaturetool.h>

#include <QList>
#include <QDockWidget>
#include <QProgressBar>
#include <QLabel>
#include <QDockWidget>
#include <QIcon>
#include <QStackedWidget>
#include <QTimer>

// QGis include
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgslayertreeview.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgsscalecombobox.h>
#include <qgsrendererv2.h>
#include <qgscomposermap.h>
#include <qgscomposerview.h>
#include <qgsraster.h>
#include <qgsmessagebar.h>
#include <qgsbrowsertreeview.h>
#include <qgis_devmaptoolidentifyaction.h>
#include <qgsdockwidget.h>
#include <qgis_dev_addfeaturetool.h>
class QCursor;
class qgis_dev : public QMainWindow
{
    Q_OBJECT

public:
    qgis_dev( QWidget *parent = 0, Qt::WFlags flags = 0 );
    ~qgis_dev();

    void addDockWidget( Qt::DockWidgetArea area, QDockWidget* dockwidget );
    void histogramStretch( bool visibleAreaOnly , QgsRaster::ContrastEnhancementLimits);

    void adjustBrightnessContrast( int delta, bool updateBrightness);
    /// 显示栅格图像的一些功能定义
    //! 局部拉伸显示
    void localHistogramStretch();
    //! 全局拉伸显示
    void fullHistogramStretch();
    //! 局部累计裁剪拉伸显示
    void localCumulativeCutStretch();
    //! 全局累计裁剪拉伸显示
    void fullCumulativeCutStretch();
    //! 增加显示亮度
    void increaseBrightness();
    //! 减少显示亮度
    void decreaseBrightness();
    //! 增加显示对比度
    void increaseContrast();
    //! 减少显示对比度
    void decreaseContrast();
    //! 保持单一实例
    static inline qgis_dev *instance() { return sm_instance; };
    QgsMessageBar* messageBar() {return  m_infoBar   ;}

public slots:
    //! 添加矢量图层
    void addVectorLayers();
    void newVectorLayer();
    void addVectorLayer(QString vecLayerPath, QString basename, QString providerKey );
    //! 添加栅格图层
    void addRasterLayers();

    void autoSelectAddedLayer( QList<QgsMapLayer*> layers );
    void  canvasRefreshStarted();
    void  canvasRefreshFinished() ;
private:
    Ui::qgis_dev ui;
    static qgis_dev* sm_instance;

    QgsMapCanvas* m_mapCanvas; // 地图画布

    //! 图层管理
    QgsLayerTreeView* m_layerTreeView;
    QgsLayerTreeMapCanvasBridge *m_layerTreeCanvasBridge;


    QDockWidget *m_layerTreeDock;
    QDockWidget *m_layerOrderDock;
    QgsDockWidget *m_OverviewDock;
    QDockWidget *m_pGpsDock;
    QDockWidget *m_logDock;

    QLabel* m_scaleLabel; // 在状态栏中显示“scale 1:”
    QgsScaleComboBox* m_scaleEdit; //! 在状态栏中显示比例尺值
    QProgressBar* m_progressBar;
    QLabel* m_coordsLabel; //! 在状态栏显示"Coordinate / Extent"
    QLineEdit* m_coordsEdit; //! 在状态栏显示地理坐标
    QgsMessageBar* m_infoBar;
    QList<QgsMapCanvasLayer> mapCanvasLayerSet; // 地图画布所用的图层集合

    // gongju
    qgis_devMapToolIdentifyAction* m_mapToolIdentify;
    qgis_dev_addFeatureTool * m_mapToolAddFeature;
    QCursor *m_overviewMapCursor;
    void initLayerTreeView();
    void createStatusBar();
    void createOverview();
    void showProgress( int theProgress, int theTotalSteps );

};

#endif // QGIS_DEV_H
