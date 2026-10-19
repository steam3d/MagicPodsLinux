// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

#pragma once

#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <QVariantList>

class QQmlEngine;
class UiSettingsService;

class LanguageManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages NOTIFY availableLanguagesChanged)

public:
    explicit LanguageManager(UiSettingsService *settings, QObject *parent = nullptr);

    QString language() const;
    void setLanguage(const QString &language);
    QVariantList availableLanguages() const;

    void installTranslators();
    void setEngine(QQmlEngine *engine);

signals:
    void languageChanged();
    void availableLanguagesChanged();

private:
    QStringList findAvailableLanguageCodes() const;
    QString languageDisplayName(const QString &language) const;
    bool isSupported(const QString &language) const;
    void applyLanguage();

    UiSettingsService *settings = nullptr;
    QQmlEngine *engine = nullptr;
    QTranslator englishTranslation;
    QTranslator localizedTranslation;
    QStringList availableLanguageCodes;
    QString currentLanguage;
    bool localizedTranslationInstalled = false;
};
