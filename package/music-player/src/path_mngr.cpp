#include "path_mngr.h"
#include "qdebug.h"

#include <dirent.h>
#include <errno.h>
#include <iostream>

int PathManager::update_internal_lists()
{
    DIR *d;
    struct dirent *dir;
    FileProps* tmp_file;
    FolderProps* tmp_folder;

    d = opendir(current_path.toStdString().c_str());

    if (!d) {
        qErrnoWarning("Error opening %s", current_path.toStdString().c_str());
        return EINVAL;
    }

    files.clear();
    folders.clear();

    while ((dir = readdir(d)) != NULL) {
        switch (dir->d_type) {
        case DT_DIR:
            if (strcmp(".", dir->d_name) == 0) {
                continue;
            }
            tmp_folder = new FolderProps;
            tmp_folder->id = folders.length();
            tmp_folder->name = dir->d_name;
            folders.push_back(*tmp_folder);
            break;

        case DT_REG:
            tmp_file = new FileProps;
            tmp_file->id = files.length();
            tmp_file->name = dir->d_name;
            files.push_back(*tmp_file);
            break;

        // ignore other types (links, ...)
        }
    }
    closedir(d);

    //dump_files_and_folders();
    emit path_changed();

    return 0;
}

PathManager::PathManager(void)
    //:current_path("/media/valerio/MUSIC/linkin park")
    :current_path("/media/sda1")
{
    update_internal_lists();
}

void PathManager::dump_files_and_folders()
{
    qDebug() << "Current path: " + current_path;
    for (FileProps item : files ) {
        qDebug() << "Files: " + item.name;
    }
    for (FolderProps item : folders ) {
        qDebug() << "Dir: " + item.name;
    }
}

QList<FileProps>* PathManager::get_list_of_files(void)
{
    return &files;
}

QList<FolderProps>* PathManager::get_list_of_folders(void)
{
    return &folders;
}

void PathManager::change_folder(const QString &msg)
{
    if (strcmp(msg.toUtf8(), "..") == 0) {
        QStringRef tmp(&current_path, 0, current_path.toStdString().find_last_of("\\/"));
        current_path = tmp.toString();
    } else {
        current_path = current_path + "/" + msg;
    }

    update_internal_lists();
}

QString PathManager::get_full_path_for_id(int id)
{
    for (FileProps item : files) {
        if (item.id == id) {
            return current_path + "/" + item.name;
        }
    }
    return "";
}
