#include "custom_list_model.h"

CustomListModel::CustomListModel(ListType _type, PathManager& _path_manager, QObject *parent)
    : QAbstractListModel(parent),
      path_manager(_path_manager),
      type(_type)
{
    refreshList();
}

void CustomListModel::refreshList()
{
    beginRemoveRows(QModelIndex(), 0, items_list.length()-1);
    items_list.clear();
    endRemoveRows();

    if (type == ENUM_LIST_FILES) {
        QList<FileProps>* tmp_files_list = path_manager.get_list_of_files();
        beginInsertRows(QModelIndex(), 0, tmp_files_list->length()-1);
        for (FileProps file_item : *tmp_files_list) {
            items_list.append(ItemProps(file_item.id, file_item.name));
        }
    } else {
        QList<FolderProps>* tmp_folder_list = path_manager.get_list_of_folders();
        beginInsertRows(QModelIndex(), 0, tmp_folder_list->length()-1);
        for (FolderProps folder_item : *tmp_folder_list) {
            items_list.append(ItemProps(folder_item.id, folder_item.name));
        }
    }

    endInsertRows();
}

QHash<int,QByteArray> CustomListModel::roleNames() const
{
    QHash<int,QByteArray> rez;
    rez[ENUM_ITEM_NAME] = "item_name";
    rez[ENUM_ITEM_ID] = "item_id";
    return rez;
}

int CustomListModel::rowCount(const QModelIndex &parent) const
{
    (void)parent;

    return items_list.count();
}

QVariant CustomListModel::data(const QModelIndex &index, int role) const
{
    (void) role;

    if (!index.isValid())
        return QVariant();

    switch (role) {
    case ENUM_ITEM_NAME:
        return QVariant::fromValue(items_list.at(index.row()).name);
        break;
    case ENUM_ITEM_ID:
        return QVariant::fromValue(items_list.at(index.row()).id);
        break;
    default:
        return QVariant();
    }
}

int CustomListModel::get_id_for_index(int i)
{
    return items_list.at(i).id;
}
