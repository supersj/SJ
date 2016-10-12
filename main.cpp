#include<QtGui/QApplication>
#include<qgsapplication.h>
#include<QApplication>

#include "qgis_dev.h"

int main( int argc, char *argv[] )
{
    QgsApplication myApp( argc, argv, true );
    QgsApplication::setPluginPath( "/usr/lib/qgis/plugins"); // 注意：把这里的路径改成你电脑上Qgis的路径！！！
    QgsApplication::initQgis();

    qgis_dev* myQgis = new qgis_dev();
    myQgis->show();
    return myApp.exec();

}
