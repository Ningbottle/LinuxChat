// message_model.cpp — chat message list model for QML

#include "message_model.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

MessageModel::MessageModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.size())
        return {};

    const auto& msg = m_messages.at(index.row());
    switch (role) {
    case SenderRole:    return msg.sender;
    case ContentRole:   return msg.content;
    case TimestampRole: return msg.timestamp;
    case IsSelfRole:       return msg.isSelf;
    case MessageTypeRole:  return msg.messageType;
    default:               return {};
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        { SenderRole,    "sender" },
        { ContentRole,   "content" },
        { TimestampRole, "timestamp" },
        { IsSelfRole,       "isSelf" },
        { MessageTypeRole,  "messageType" }
    };
}

void MessageModel::addMessage(const QString& sender, const QString& content,
                              const QString& timestamp, bool isSelf)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append({sender, content, timestamp, isSelf});
    endInsertRows();
}

void MessageModel::addMessage(const QString& sender, const QString& content,
                              const QString& timestamp, bool isSelf,
                              const QString& messageType)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    MessageItem item{sender, content, timestamp, isSelf, messageType};
    m_messages.append(item);
    endInsertRows();
}

void MessageModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

void MessageModel::loadFromJsonArray(const QJsonArray& messages, const QString& myUsername)
{
    beginResetModel();
    m_messages.clear();
    for (const auto& msgVal : messages) {
        auto obj = msgVal.toObject();
        MessageItem item;
        item.sender = obj["from"].toString();
        item.content = obj["content"].toString();
        auto ts = obj["timestamp"].toInteger();
        item.timestamp = QDateTime::fromSecsSinceEpoch(ts).toString("hh:mm:ss");
        item.isSelf = (item.sender == myUsername);
        item.messageType = "normal";
        m_messages.append(item);
    }
    endResetModel();
}
