#ifndef CUSTOM_LIST_MODEL_H
#define CUSTOM_LIST_MODEL_H

#include <QObject>
#include <QQuickItem>
#include <QObjectList>
#include <QQmlListProperty>
#include <QStringListModel>

#include "path_mngr.h"

struct ItemProps {
    ItemProps(int _id, QString _name) {
        id = _id;
        name = _name;
    }
    int id;
    QString name;
};

class CustomListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum CustomListModelEnum{
        ENUM_ITEM_NAME = Qt::UserRole+1,
        ENUM_ITEM_ID,
    };

    enum ListType{
        ENUM_LIST_FILES,
        ENUM_LIST_FOLDERS
    };

    CustomListModel(ListType type, PathManager& path_manager, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int,QByteArray> roleNames() const;

public slots:
    void refreshList(void);
    int get_id_for_index(int i);

private:
    QList<ItemProps> items_list;
    PathManager& path_manager;
    ListType type;
};

#endif // CUSTOM_LIST_MODEL_H
