#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "应用程序启动...";
    
    // 设置应用程序信息
    app.setApplicationName("QWeight_demo");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Qt路径计算可视化工具");
    app.setOrganizationName("GitHub Copilot");
    
    qDebug() << "设置应用程序样式...";
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    qDebug() << "创建主窗口...";
    // 创建并显示主窗口
    MainWindow window;
    
    qDebug() << "显示主窗口...";
    window.show();
    
    qDebug() << "进入事件循环...";
    return app.exec();
}
