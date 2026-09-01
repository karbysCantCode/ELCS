#include "hotkeymanager.h"

#include <QShortcut>
#include <QSettings>
#include <QWidget>

#include <algorithm>

HotkeyManager* globalHotkeyManager = nullptr;

namespace {

QString settingsKeyForContext(const std::string& context)
{
    return QString("hotkeyOverlay/context/%1").arg(QString::fromStdString(context));
}

}

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent)
{
}

void HotkeyManager::ensureContextKnown(const std::string& context)
{
    if (std::find(contextOrder.begin(), contextOrder.end(), context) != contextOrder.end())
        return;

    contextOrder.push_back(context);
    registry[context] = {};

    emit contextsChanged();
}

void HotkeyManager::registerHotkey(
    const std::string& context,
    const QKeySequence& sequence,
    const std::string& description
)
{
    ensureContextKnown(context);

    registry[context].push_back(HotkeyEntry{sequence, description});

    emit hotkeysChanged();
}

QShortcut* HotkeyManager::registerActionHotkey(
    const std::string& context,
    const QKeySequence& sequence,
    const std::string& description,
    QWidget* parent,
    std::function<void()> callback,
    Qt::ShortcutContext shortcutContext
)
{
    registerHotkey(context, sequence, description);

    auto* shortcut = new QShortcut(sequence, parent);
    shortcut->setContext(shortcutContext);

    QObject::connect(shortcut, &QShortcut::activated, parent, [callback]() {
        callback();
    });

    return shortcut;
}

void HotkeyManager::setActiveContext(const std::string& context)
{
    ensureContextKnown(context);

    if (p_activeContext == context)
        return;

    p_activeContext = context;

    emit activeContextChanged(p_activeContext);
}

std::vector<HotkeyEntry> HotkeyManager::hotkeysForContext(const std::string& context) const
{
    std::vector<HotkeyEntry> result;

    auto globalIt = registry.find("global");
    if (globalIt != registry.end())
        result.insert(result.end(), globalIt->second.begin(), globalIt->second.end());

    if (context != "global")
    {
        auto it = registry.find(context);
        if (it != registry.end())
            result.insert(result.end(), it->second.begin(), it->second.end());
    }

    return result;
}

bool HotkeyManager::overlayGloballyEnabled() const
{
    QSettings settings("ELCS", "ELCS");
    return settings.value("hotkeyOverlay/globalEnabled", true).toBool();
}

void HotkeyManager::setOverlayGloballyEnabled(bool enabled)
{
    QSettings settings("ELCS", "ELCS");
    settings.setValue("hotkeyOverlay/globalEnabled", enabled);

    emit settingsChanged();
}

bool HotkeyManager::contextEnabled(const std::string& context) const
{
    QSettings settings("ELCS", "ELCS");
    return settings.value(settingsKeyForContext(context), true).toBool();
}

void HotkeyManager::setContextEnabled(const std::string& context, bool enabled)
{
    QSettings settings("ELCS", "ELCS");
    settings.setValue(settingsKeyForContext(context), enabled);

    emit settingsChanged();
}