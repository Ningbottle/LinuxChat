#include "font_manager.h"
#include <QFontDatabase>
#include <QCoreApplication>
#include <QDebug>

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() 
    : m_bodyFontFamily("LXGW WenKai")
    , m_titleFontFamily("Newsreader")
    , m_codeFontFamily("Cascadia Code")
{
}

bool FontManager::loadFonts() {
    int id1 = QFontDatabase::addApplicationFont(":/fonts/LXGWWenKai-Regular.ttf");
    if (id1 == -1) {
        qWarning() << "Failed to load LXGWWenKai-Regular.ttf";
    } else {
        QStringList families = QFontDatabase::applicationFontFamilies(id1);
        if (!families.isEmpty()) {
            m_bodyFontFamily = families.first();
            qDebug() << "Loaded font family:" << m_bodyFontFamily;
        }
    }
    
    int id2 = QFontDatabase::addApplicationFont(":/fonts/LXGWWenKai-Medium.ttf");
    if (id2 == -1) {
        qWarning() << "Failed to load LXGWWenKai-Medium.ttf";
    }
    
    int id3 = QFontDatabase::addApplicationFont(":/fonts/Newsreader-VariableFont_opsz,wght.ttf");
    if (id3 == -1) {
        qWarning() << "Failed to load Newsreader font";
    } else {
        QStringList families = QFontDatabase::applicationFontFamilies(id3);
        if (!families.isEmpty()) {
            m_titleFontFamily = families.first();
            qDebug() << "Loaded title font family:" << m_titleFontFamily;
        }
    }
    
    return true;
}

QFont FontManager::bodyFont(int pointSize) const {
    QFont font(m_bodyFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::Normal);
    return font;
}

QFont FontManager::titleFont(int pointSize) const {
    QFont font(m_titleFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::DemiBold);
    return font;
}

QFont FontManager::codeFont(int pointSize) const {
    QFont font(m_codeFontFamily);
    font.setPointSize(pointSize);
    font.setWeight(QFont::Normal);
    return font;
}