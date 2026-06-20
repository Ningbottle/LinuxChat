// user_model.cpp — online user list model for QML

#include "user_model.h"

UserModel::UserModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int UserModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_users.size();
}

QVariant UserModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_users.size())
        return {};

    switch (role) {
    case UsernameRole: return m_users.at(index.row());
    default:           return {};
    }
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    return {
        { UsernameRole, "username" }
    };
}

int UserModel::count() const
{
    return m_users.size();
}

void UserModel::setUsers(const QStringList& users)
{
    beginResetModel();
    m_users = users;
    endResetModel();
    emit countChanged();
}
