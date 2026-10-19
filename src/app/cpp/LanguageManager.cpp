// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#include "LanguageManager.h"

#include "UiSettingsService.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QQmlEngine>
#include <QVariantMap>

#include <algorithm>

namespace {
const QString LanguageSettingKey = QStringLiteral("language");
const QString SystemLanguage = QStringLiteral("system");
const QString TranslationPrefix = QStringLiteral("locale_");
const QString TranslationSuffix = QStringLiteral(".qm");
const QString TranslationDirectory = QStringLiteral(":/i18n/");

QString nativeLanguageName(const QString &language)
{
    const QLocale locale(language);
    const QString name = locale.nativeLanguageName();
    return name.isEmpty() ? QLocale::languageToString(locale.language()) : name;
}
}

LanguageManager::LanguageManager(UiSettingsService *settings, QObject *parent)
    : QObject(parent)
    , settings(settings)
    , availableLanguageCodes(findAvailableLanguageCodes())
{
    Q_ASSERT(this->settings);

    currentLanguage = this->settings->value(LanguageSettingKey, SystemLanguage).toString();

    if (!isSupported(currentLanguage)) {
        qWarning() << "[LanguageManager] Unsupported saved language" << currentLanguage
                   << "- using the system language";
        currentLanguage = SystemLanguage;
    }
}

QString LanguageManager::language() const
{
    return currentLanguage;
}

void LanguageManager::setLanguage(const QString &language)
{
    if (!isSupported(language)) {
        qWarning() << "[LanguageManager] Unsupported language" << language;
        return;
    }

    if (currentLanguage == language)
        return;

    currentLanguage = language;
    if (!settings->setValue(LanguageSettingKey, currentLanguage))
        qWarning() << "[LanguageManager] Failed to save language" << currentLanguage;

    applyLanguage();
    emit languageChanged();
}

QVariantList LanguageManager::availableLanguages() const
{
    QVariantList languages;
    languages.reserve(availableLanguageCodes.size() + 1);
    languages.append(QVariantMap{
        {QStringLiteral("code"), SystemLanguage},
        {QStringLiteral("name"), qtTrId("settings.language.system")},
    });

    for (const QString &language : availableLanguageCodes) {
        languages.append(QVariantMap{
            {QStringLiteral("code"), language},
            {QStringLiteral("name"), languageDisplayName(language)},
        });
    }

    return languages;
}

void LanguageManager::installTranslators()
{
    if (!englishTranslation.load(TranslationDirectory + QStringLiteral("locale_en.qm")))
        qFatal("Could not load the English translation");

    if (!QCoreApplication::installTranslator(&englishTranslation))
        qFatal("Could not install the English translation");

    applyLanguage();
}

void LanguageManager::setEngine(QQmlEngine *engine)
{
    this->engine = engine;
}

QStringList LanguageManager::findAvailableLanguageCodes() const
{
    const QDir directory(TranslationDirectory);
    const QStringList files = directory.entryList(
        QStringList{TranslationPrefix + QLatin1Char('*') + TranslationSuffix},
        QDir::Files,
        QDir::Name);

    QStringList languages;
    languages.reserve(files.size());
    for (const QString &file : files) {
        const qsizetype prefixLength = TranslationPrefix.size();
        const qsizetype suffixLength = TranslationSuffix.size();
        languages.append(file.mid(prefixLength, file.size() - prefixLength - suffixLength));
    }

    std::sort(languages.begin(), languages.end(), [](const QString &left, const QString &right) {
        const int nameComparison = QString::localeAwareCompare(nativeLanguageName(left),
                                                                nativeLanguageName(right));
        return nameComparison == 0 ? left < right : nameComparison < 0;
    });
    return languages;
}

QString LanguageManager::languageDisplayName(const QString &language) const
{
    const QString name = nativeLanguageName(language);
    for (const QString &otherLanguage : availableLanguageCodes) {
        if (otherLanguage != language && nativeLanguageName(otherLanguage) == name)
            return QStringLiteral("%1 (%2)").arg(name, language);
    }

    return name;
}

bool LanguageManager::isSupported(const QString &language) const
{
    return language == SystemLanguage
           || availableLanguageCodes.contains(language);
}

void LanguageManager::applyLanguage()
{
    if (localizedTranslationInstalled) {
        QCoreApplication::removeTranslator(&localizedTranslation);
        localizedTranslationInstalled = false;
    }

    const QLocale locale = currentLanguage == SystemLanguage
                               ? QLocale::system()
                               : QLocale(currentLanguage);

    QLocale::setDefault(locale);
    QApplication::setLayoutDirection(locale.textDirection());

    if (locale.language() != QLocale::English) {
        if (localizedTranslation.load(locale,
                                      QStringLiteral("locale"),
                                      QStringLiteral("_"),
                                      TranslationDirectory)) {
            localizedTranslationInstalled = QCoreApplication::installTranslator(&localizedTranslation);
            qInfo() << "[LanguageManager] Loaded" << localizedTranslation.filePath();
        } else {
            qWarning() << "[LanguageManager] No translation for" << locale.name()
                       << "- using English";
        }
    }

    if (engine)
        engine->retranslate();

    emit availableLanguagesChanged();
}
