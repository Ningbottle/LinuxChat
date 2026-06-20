#pragma once
// user_model.h — QAbstractListModel exposing online users to QML

#include <QAbstractListModel>
#include <QString>
#include <QStringList>

class UserModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        UsernameRole = Qt::UserRole + 1
    };
    Q_ENUM(Roles)

    explicit UserModel(QObject* parent = nullptr);
    ~UserModel() override = default;

    // ── QAbstractListModel Overrides ────────────────────────────────

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // ── Property Getter ─────────────────────────────────────────────

    [[nodiscard]] int count() const;

    // ── QML-Accessible Mutators ─────────────────────────────────────

    Q_INVOKABLE void setUsers(const QStringList& users);

signals:
    void countChanged();

private:
    QStringList m_users;
};
