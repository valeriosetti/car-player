#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QtQuick>

#include "main.h"
#include "spectrumgraph.h"
#include "path_mngr.h"
#include "custom_list_model.h"
#include "gstmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    PathManager path_manager;
    GstManager gst_manager(argc, argv, path_manager);
    CustomListModel file_list_model(CustomListModel::ENUM_LIST_FILES, path_manager);
    CustomListModel folder_list_model(CustomListModel::ENUM_LIST_FOLDERS, path_manager);

    QObject::connect(&path_manager, SIGNAL(path_changed()), &file_list_model, SLOT(refreshList()));
    QObject::connect(&path_manager, SIGNAL(path_changed()), &folder_list_model, SLOT(refreshList()));

    qmlRegisterType<SpectrumGraph>("cpp_custom_components", 1, 0, "SpectrumGraph");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    engine.rootContext()->setContextProperty("pathManager", &path_manager);
    engine.rootContext()->setContextProperty("fileListModel", &file_list_model);
    engine.rootContext()->setContextProperty("folderListModel", &folder_list_model);
    engine.rootContext()->setContextProperty("gstManager", &gst_manager);

    return app.exec();
}
