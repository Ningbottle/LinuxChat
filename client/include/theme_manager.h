#pragma once
#include <QObject>
#include <QVariantMap>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    explicit ThemeManager(QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap skin(const QString& name = QString()) const;
    Q_INVOKABLE QStringList skinNames() const;
    Q_INVOKABLE void setSkin(const QString& name);
    Q_INVOKABLE QString currentSkin() const;

signals:
    void skinChanged();

private:
    QString m_currentSkin = "Minimal";
    void init();
    QMap<QString, QVariantMap> m_skins;
};
