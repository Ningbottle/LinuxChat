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
    minimal["bubbleSelf"] = "#F0F0F0"; minimal["bubbleOther"] = "transparent";
    minimal["bubbleSelfText"] = "#1A1A1A"; minimal["bubbleOtherText"] = "#1A1A1A";
    minimal["fontBody"] = "LXGW WenKai"; minimal["fontBodySize"] = 14;
    minimal["fontTitle"] = "Newsreader"; minimal["fontTitleSize"] = 16;
    minimal["fontCaption"] = "LXGW WenKai"; minimal["fontCaptionSize"] = 11;
    minimal["xs"] = 4; minimal["sm"] = 8; minimal["md"] = 12; minimal["lg"] = 16; minimal["xl"] = 24;
    minimal["rSm"] = 4; minimal["rMd"] = 8; minimal["rLg"] = 12; minimal["rFull"] = 9999;
    minimal["bubbleMaxW"] = 400; minimal["bubblePadH"] = 12; minimal["bubblePadV"] = 14; minimal["bubbleR"] = 4;
    minimal["avatarSize"] = 28;
    m_skins["Minimal"] = minimal;

    // Dense (design-b-dense.html)
    QVariantMap dense;
    dense["canvas"] = "#F0F0F5"; dense["surface"] = "#FFFFFF"; dense["text"] = "#1A1A2E";
    dense["muted"] = "#5A5A72"; dense["subtle"] = "#9292A8"; dense["border"] = "#E2E2EA";
    dense["accent"] = "#5865F2"; dense["accentHover"] = "#4752C4";
    dense["success"] = "#23A559"; dense["warning"] = "#F0B232"; dense["danger"] = "#ED4245";
    dense["bubbleSelf"] = "#DDE3FF"; dense["bubbleOther"] = "#FFFFFF";
    dense["bubbleSelfText"] = "#1A1A2E"; dense["bubbleOtherText"] = "#1A1A2E";
    dense["fontBody"] = "LXGW WenKai"; dense["fontBodySize"] = 12;
    dense["fontTitle"] = "LXGW WenKai"; dense["fontTitleSize"] = 14;
    dense["fontCaption"] = "LXGW WenKai"; dense["fontCaptionSize"] = 10;
    dense["xs"] = 2; dense["sm"] = 4; dense["md"] = 8; dense["lg"] = 12; dense["xl"] = 16;
    dense["rSm"] = 4; dense["rMd"] = 10; dense["rLg"] = 12; dense["rFull"] = 9999;
    dense["bubbleMaxW"] = 480; dense["bubblePadH"] = 10; dense["bubblePadV"] = 10; dense["bubbleR"] = 10;
    dense["avatarSize"] = 36;
    m_skins["Dense"] = dense;

    // Motion (design-c-motion.html)
    QVariantMap motion;
    motion["canvas"] = "#E8E8EC"; motion["surface"] = "rgba(255,255,255,0.55)"; motion["text"] = "#1C1C1E";
    motion["muted"] = "#636366"; motion["subtle"] = "#AEAEB2"; motion["border"] = "rgba(255,255,255,0.60)";
    motion["accent"] = "#4A6CF7"; motion["accentHover"] = "#3B5DE7";
    motion["success"] = "#34C759"; motion["warning"] = "#F0B232"; motion["danger"] = "#FF3B30";
    motion["bubbleSelf"] = "rgba(74,108,247,0.88)"; motion["bubbleOther"] = "rgba(255,255,255,0.70)";
    motion["bubbleSelfText"] = "#FFFFFF"; motion["bubbleOtherText"] = "#1C1C1E";
    motion["fontBody"] = "LXGW WenKai"; motion["fontBodySize"] = 14;
    motion["fontTitle"] = "Newsreader"; motion["fontTitleSize"] = 18;
    motion["fontCaption"] = "LXGW WenKai"; motion["fontCaptionSize"] = 11;
    motion["xs"] = 4; motion["sm"] = 8; motion["md"] = 14; motion["lg"] = 20; motion["xl"] = 28;
    motion["rSm"] = 8; motion["rMd"] = 14; motion["rLg"] = 16; motion["rFull"] = 9999;
    motion["bubbleMaxW"] = 420; motion["bubblePadH"] = 14; motion["bubblePadV"] = 14; motion["bubbleR"] = 16;
    motion["avatarSize"] = 44;
    m_skins["Motion"] = motion;

    // iMessage (design-d-imessage.html)
    QVariantMap imsg;
    imsg["canvas"] = "#FFFFFF"; imsg["surface"] = "#FFFFFF"; imsg["text"] = "#000000";
    imsg["muted"] = "#8E8E93"; imsg["subtle"] = "#AEAEB2"; imsg["border"] = "#E5E5EA";
    imsg["accent"] = "#007AFF"; imsg["accentHover"] = "#0056CC";
    imsg["success"] = "#34C759"; imsg["warning"] = "#FF9500"; imsg["danger"] = "#FF3B30";
    imsg["bubbleSelf"] = "#007AFF"; imsg["bubbleOther"] = "#E9E9EB";
    imsg["bubbleSelfText"] = "#FFFFFF"; imsg["bubbleOtherText"] = "#000000";
    imsg["fontBody"] = "LXGW WenKai"; imsg["fontBodySize"] = 15;
    imsg["fontTitle"] = "LXGW WenKai"; imsg["fontTitleSize"] = 16;
    imsg["fontCaption"] = "LXGW WenKai"; imsg["fontCaptionSize"] = 11;
    imsg["xs"] = 4; imsg["sm"] = 8; imsg["md"] = 12; imsg["lg"] = 16; imsg["xl"] = 20;
    imsg["rSm"] = 8; imsg["rMd"] = 14; imsg["rLg"] = 18; imsg["rFull"] = 9999;
    imsg["bubbleMaxW"] = 280; imsg["bubblePadH"] = 12; imsg["bubblePadV"] = 10; imsg["bubbleR"] = 18;
    imsg["avatarSize"] = 40;
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
