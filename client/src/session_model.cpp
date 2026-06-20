// session_model.cpp — Implementation of SessionModel

#include "session_model.h"

SessionModel::SessionModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SessionModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_sessions.size();
}

QVariant SessionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sessions.size())
        return {};

    const SessionItem& item = m_sessions.at(index.row());

    switch (role) {
    case TargetNameRole:   return item.targetName;
    case IsRoomRole:       return item.isRoom;
    case UnreadCountRole:  return item.unreadCount;
    case LastMessageRole:  return item.lastMessage;
    case LastTimestampRole: return item.lastTimestamp;
    default:               return {};
    }
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    return {
        { TargetNameRole,   "targetName" },
        { IsRoomRole,       "isRoom" },
        { UnreadCountRole,  "unreadCount" },
        { LastMessageRole,  "lastMessage" },
        { LastTimestampRole, "lastTimestamp" }
    };
}

void SessionModel::addSession(const QString& targetName, bool isRoom)
{
    if (findSession(targetName) >= 0)
        return;  // already exists

    const int row = m_sessions.size();
    beginInsertRows(QModelIndex(), row, row);
    SessionItem item;
    item.targetName = targetName;
    item.isRoom = isRoom;
    m_sessions.append(item);
    endInsertRows();
}

void SessionModel::updateSession(const QString& targetName,
                                 const QString& lastMessage,
                                 const QString& lastTimestamp)
{
    const int idx = findSession(targetName);
    if (idx < 0)
        return;

    SessionItem& item = m_sessions[idx];
    item.lastMessage = lastMessage;
    item.lastTimestamp = lastTimestamp;

    const QModelIndex modelIndex = index(idx);
    emit dataChanged(modelIndex, modelIndex, { LastMessageRole, LastTimestampRole });
}

void SessionModel::incrementUnread(const QString& targetName)
{
    const int idx = findSession(targetName);
    if (idx < 0)
        return;

    m_sessions[idx].unreadCount++;

    const QModelIndex modelIndex = index(idx);
    emit dataChanged(modelIndex, modelIndex, { UnreadCountRole });
}

void SessionModel::clearUnread(const QString& targetName)
{
    const int idx = findSession(targetName);
    if (idx < 0)
        return;

    m_sessions[idx].unreadCount = 0;

    const QModelIndex modelIndex = index(idx);
    emit dataChanged(modelIndex, modelIndex, { UnreadCountRole });
}

bool SessionModel::hasSession(const QString& targetName) const
{
    return findSession(targetName) >= 0;
}

void SessionModel::clear()
{
    beginResetModel();
    m_sessions.clear();
    endResetModel();
}

int SessionModel::findSession(const QString& targetName) const
{
    for (int i = 0; i < m_sessions.size(); ++i) {
        if (m_sessions.at(i).targetName == targetName)
            return i;
    }
    return -1;
}
