
/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/
*
* Copyright 1999-2020 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "colormanager.h"

#include <QApplication>
#include <QDir>
#include <QPalette>
#include <QSettings>
#include "../../util/directory.h"
#include "../../backend/managers/cdisplaytemplatemgr.h"

ColorManager * ColorManager::m_instance = nullptr;

ColorManager * ColorManager::instance() {
    if (m_instance == nullptr)
        m_instance = new ColorManager();
    return m_instance;
}


ColorManager::ColorManager(QObject * parent)
    : QObject(parent) {
}

bool ColorManager::darkMode() const {
    QPalette p = qApp->palette();
    QColor b = p.color(QPalette::Base);
    if (b.value() < 128)
        return true;
    return false;
}

QString ColorManager::loadColorMaps() {
    QString errorMessage;
    namespace DU = util::directory;
    const QDir & td = DU::getDisplayTemplatesDir(); // Global template directory
    const QDir & utd = DU::getUserDisplayTemplatesDir(); // User template directory
    const QDir::Filters readableFileFilter(QDir::Files | QDir::Readable);
    const QStringList cssFilter("*.css");

    // Load global app stylesheets
    Q_FOREACH(const QString & file, td.entryList(cssFilter, readableFileFilter))
        loadColorMap(td.canonicalPath() + "/" + file);

    // Load user app stylesheets
    Q_FOREACH(const QString & file, utd.entryList(cssFilter, readableFileFilter))
        loadColorMap(utd.canonicalPath() + "/" + file);

    return errorMessage;
}

ColorMap* ColorManager::createColorMapWithDefaults(){
    ColorMap * colorDefs = new ColorMap();
    if (darkMode()) {
        colorDefs->insert("FOREGROUND_COLOR", qApp->palette().color(QPalette::WindowText).name());
        colorDefs->insert("BACKGROUND_COLOR", qApp->palette().color(QPalette::Base).name());
        colorDefs->insert("HIGHLIGHT_COLOR",  QColor("#ffff00").name());
        colorDefs->insert("CROSSREF_COLOR",  QColor("#aac2ff").name());
        colorDefs->insert("JESUS_WORDS_COLOR",QColor("#ff0000").name());
    } else {
        colorDefs = new QMap<QString, QString>();
        colorDefs->insert("FOREGROUND_COLOR", qApp->palette().color(QPalette::WindowText).name());
        colorDefs->insert("BACKGROUND_COLOR", qApp->palette().color(QPalette::Base).name());
        colorDefs->insert("HIGHLIGHT_COLOR",  QColor("#ffff00").name());
        colorDefs->insert("CROSSREF_COLOR",  QColor("#1414ff").name());
        colorDefs->insert("JESUS_WORDS_COLOR",QColor("#ff0000").name());
    }
    return colorDefs;
}

void ColorManager::loadColorMap(const QString & filePath) {
    QFileInfo cssInfo(filePath);
    QString cMapPath = cssInfo.path() + "/" + cssInfo.completeBaseName() + ".cmap";
    QFileInfo cMapInfo(cMapPath);
    QString fileName = cssInfo.fileName();
    ColorMap * colorMap = createColorMapWithDefaults();
    if (cMapInfo.exists()) {
        QString group = darkMode()? "dark": "light";
        QSettings * cMapSettings = new QSettings(cMapPath,QSettings::IniFormat);
        if (m_colorMaps.contains(fileName)) {
            ColorMap * oldMap = m_colorMaps.value(fileName);
            delete oldMap;
        }
        cMapSettings->beginGroup(group);
        QStringList colorKeys = cMapSettings->childKeys();
        for (QString colorKey:colorKeys) {
            QString value = cMapSettings->value(colorKey).toString();
            colorMap->insert(colorKey,value);
        }
        cMapSettings->endGroup();
    }
    m_colorMaps.insert(fileName, colorMap);
}

QString ColorManager::replaceColors(const QString& content) {
    QString activeTemplate = CDisplayTemplateMgr::activeTemplateName();
    ColorMap * colorMap = m_colorMaps[activeTemplate];
    QString newContent = content;
    QStringList keys = colorMap->keys();
    for (QString key : keys) {
        QString pattern = "#" + key + "#";
        QString value = colorMap->value(key);
        newContent = newContent.replace(pattern, value);
    }
    return newContent;
}

QString ColorManager::getColorByPattern(const QString& pattern, const QString& style) {
    QString activeTemplate;
    if (style.isEmpty())
        activeTemplate = CDisplayTemplateMgr::activeTemplateName();
    ColorMap * colorMap = m_colorMaps[activeTemplate];
    BT_ASSERT(colorMap != nullptr);
    QString value = colorMap->value(pattern);
    BT_ASSERT(!value.isEmpty());
    return value;
}

QString ColorManager::getBackgroundColor(const QString& style) {
    return getColorByPattern("BACKGROUND_COLOR", style);
}

QString ColorManager::getForegroundColor(const QString& style) {
    return getColorByPattern("FOREGROUND_COLOR", style);
}

QString ColorManager::getCrossRefColor(const QString& style) {
    return getColorByPattern("CROSSREF_COLOR", style);
}


