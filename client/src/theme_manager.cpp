#include "theme_manager.h"

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
    init();
}

void ThemeManager::init() {
    // Minimal (design-a-minimal.html)
    QVariantMap minimal;
    minimal["canvas"] = "#FFFFFF"; minimal["surface"] = "#FAFAFA"; minimal["text"] = "#1A1A1A";
    minimal["muted"] = "#8E8E93"; minimal["subtle"] = "#AEAEB2"; minimal["border"] = "#E5E5EA";
    minimal["accent"] = "#8E8E93"; minimal["accentHover"] = "#636366";
    minimal["success"] = "#34C759"; minimal["warning"] = "#F0B232"; minimal["danger"] = "#ED4245";
    minimal["bubbleSelf"] = "#F0F0F0"; minimal["bubbleOther"] = "#F2F2F7";
    minimal["bubbleSelfText"] = "#1A1A1A"; minimal["bubbleOtherText"] = "#1A1A1A";
    minimal["radiusSm"] = 4; minimal["radiusMd"] = 8; minimal["radiusLg"] = 12;
    minimal["sidebarW"] = 260; minimal["bubbleMaxW"] = 0.55;
    minimal["avatarBg"] = "#7C8CF0";
    m_skins["Minimal"] = minimal;

    // Dense (design-b-dense.html)
    QVariantMap dense;
    dense["canvas"] = "#F0F0F5"; dense["surface"] = "#FFFFFF"; dense["text"] = "#1A1A2E";
    dense["muted"] = "#5A5A72"; dense["subtle"] = "#9292A8"; dense["border"] = "#D8D8E0";
    dense["accent"] = "#5865F2"; dense["accentHover"] = "#4752C4";
    dense["success"] = "#23A559"; dense["warning"] = "#F0B232"; dense["danger"] = "#ED4245";
    dense["bubbleSelf"] = "#DDE3FF"; dense["bubbleOther"] = "#FFFFFF";
    dense["bubbleSelfText"] = "#1A1A2E"; dense["bubbleOtherText"] = "#1A1A2E";
    dense["radiusSm"] = 6; dense["radiusMd"] = 10; dense["radiusLg"] = 16;
    dense["sidebarW"] = 280; dense["bubbleMaxW"] = 0.70;
    dense["avatarBg"] = "#5865F2";
    m_skins["Dense"] = dense;

    // Motion (design-c-motion.html)
    QVariantMap motion;
    motion["canvas"] = "#E8E8EC"; motion["surface"] = "rgba(255,255,255,0.55)"; motion["text"] = "#1a1a2e";
    motion["muted"] = "#555570"; motion["subtle"] = "#8888a0"; motion["border"] = "rgba(255,255,255,0.6)";
    motion["accent"] = "#4a6cf7"; motion["accentHover"] = "#3b5de7";
    motion["success"] = "#22c55e"; motion["warning"] = "#F0B232"; motion["danger"] = "#FF3B30";
    motion["bubbleSelf"] = "rgba(74,108,247,0.88)"; motion["bubbleOther"] = "rgba(255,255,255,0.7)";
    motion["bubbleSelfText"] = "#FFFFFF"; motion["bubbleOtherText"] = "#1a1a2e";
    motion["radiusSm"] = 8; motion["radiusMd"] = 16; motion["radiusLg"] = 20;
    motion["sidebarW"] = 280; motion["bubbleMaxW"] = 0.70;
    motion["avatarBg"] = "#4a6cf7";
    m_skins["Motion"] = motion;

    // iMessage (design-d-imessage.html)
    QVariantMap imsg;
    imsg["canvas"] = "#FFFFFF"; imsg["surface"] = "#FFFFFF"; imsg["text"] = "#000000";
    imsg["muted"] = "#8E8E93"; imsg["subtle"] = "#AEAEB2"; imsg["border"] = "#E5E5EA";
    imsg["accent"] = "#007AFF"; imsg["accentHover"] = "#0056CC";
    imsg["success"] = "#34C759"; imsg["warning"] = "#FF9500"; imsg["danger"] = "#FF3B30";
    imsg["bubbleSelf"] = "#007AFF"; imsg["bubbleOther"] = "#E9E9EB";
    imsg["bubbleSelfText"] = "#FFFFFF"; imsg["bubbleOtherText"] = "#000000";
    imsg["radiusSm"] = 8; imsg["radiusMd"] = 18; imsg["radiusLg"] = 16;
    imsg["sidebarW"] = 320; imsg["bubbleMaxW"] = 0.65;
    imsg["avatarBg"] = "#007AFF";
    m_skins["iMessage"] = imsg;
}

QVariantMap ThemeManager::skin(const QString& name) const {
    QString key = name.isEmpty() ? m_currentSkin : name;
    return m_skins.value(key, m_skins["Minimal"]);
}

QStringList ThemeManager::skinNames() const {
    return {"Minimal", "Dense", "Motion", "iMessage"};
}

void ThemeManager::setSkin(const QString& name) {
    if (m_skins.contains(name) && m_currentSkin != name) {
        m_currentSkin = name;
        emit skinChanged();
    }
}

QString ThemeManager::currentSkin() const {
    return m_currentSkin;
}
