#ifndef PATH_MNGR_H
#define PATH_MNGR_H

#include <string>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>

struct BasePathProps {
    int id;
    QString name;
};

struct FileProps : BasePathProps {
    QString id3_tag;
    // Add other properties as required
};

struct FolderProps : BasePathProps {
    // Add other properties as required
};

class PathManager : public QObject
{
    Q_OBJECT

public:
    PathManager();
    QList<FileProps>* get_list_of_files(void);
    QList<FolderProps>* get_list_of_folders(void);

    QString get_full_path_for_id(int id);

public slots:
    void change_folder(const QString &msg);

signals:
    void path_changed(void);

private:
    int update_internal_lists(void);
    void dump_files_and_folders(void);

    QString current_path;
    QList<FileProps> files;
    QList<FolderProps> folders;
};

#endif // PATH_MNGR_H
