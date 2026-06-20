#pragma once
// message_model.h — QAbstractListModel exposing chat messages to QML

#include <QAbstractListModel>
#include <QDateTime>
#include <QJsonArray>
#include <QString>
#include <QVector>

struct MessageItem {
    QString sender;
    QString content;
    QString timestamp;
    bool isSelf = false;
    QString messageType = "normal";
};

class MessageModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        SenderRole = Qt::UserRole + 1,
        ContentRole,
        TimestampRole,
        IsSelfRole,
        MessageTypeRole
    };
    Q_ENUM(Roles)

    explicit MessageModel(QObject* parent = nullptr);
    ~MessageModel() override = default;

    // ── QAbstractListModel Overrides ────────────────────────────────

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // ── QML-Accessible Mutators ─────────────────────────────────────

    Q_INVOKABLE void addMessage(const QString& sender, const QString& content,
                                const QString& timestamp, bool isSelf);
    Q_INVOKABLE void addMessage(const QString& sender, const QString& content,
                                const QString& timestamp, bool isSelf,
                                const QString& messageType);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void loadFromJsonArray(const QJsonArray& messages, const QString& myUsername);

private:
    QVector<MessageItem> m_messages;
};
