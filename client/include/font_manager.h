#pragma once

#include <QFont>
#include <QString>

class FontManager {
public:
    static FontManager& instance();
    bool loadFonts();
    QFont bodyFont(int pointSize = 15) const;
    QFont titleFont(int pointSize = 16) const;
    QFont codeFont(int pointSize = 13) const;
    QString bodyFontFamily() const { return m_bodyFontFamily; }
    QString titleFontFamily() const { return m_titleFontFamily; }
    QString codeFontFamily() const { return m_codeFontFamily; }

private:
    FontManager();
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    QString m_bodyFontFamily;
    QString m_titleFontFamily;
    QString m_codeFontFamily;
};