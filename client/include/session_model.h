#pragma once
// session_model.h — Chat session list model for QML ListView
//
// Each session represents one conversation target (the group room or a
// private chat partner).  Exposed roles map directly to QML delegate
// bindings.

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct SessionItem {
    QString targetName;   // "__room__" or username
    bool isRoom = true;
    int unreadCount = 0;
    QString lastMessage;
    QString lastTimestamp;
};

class SessionModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        TargetNameRole = Qt::UserRole + 1,
        IsRoomRole,
        UnreadCountRole,
        LastMessageRole,
        LastTimestampRole
    };
    Q_ENUM(Roles)

    explicit SessionModel(QObject* parent = nullptr);
    ~SessionModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addSession(const QString& targetName, bool isRoom);
    Q_INVOKABLE void updateSession(const QString& targetName, const QString& lastMessage, const QString& lastTimestamp);
    Q_INVOKABLE void incrementUnread(const QString& targetName);
    Q_INVOKABLE void clearUnread(const QString& targetName);
    Q_INVOKABLE bool hasSession(const QString& targetName) const;
    Q_INVOKABLE void clear();

private:
    QVector<SessionItem> m_sessions;
    int findSession(const QString& targetName) const;
};
