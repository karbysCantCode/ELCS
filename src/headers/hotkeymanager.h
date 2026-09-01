#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class QShortcut;
class QWidget;

struct HotkeyEntry
{
    QKeySequence sequence;
    std::string description;
};

class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject* parent = nullptr);

    void registerHotkey(
        const std::string& context,
        const QKeySequence& sequence,
        const std::string& description
    );

    QShortcut* registerActionHotkey(
        const std::string& context,
        const QKeySequence& sequence,
        const std::string& description,
        QWidget* parent,
        std::function<void()> callback,
        Qt::ShortcutContext shortcutContext = Qt::WidgetWithChildrenShortcut
    );

    void setActiveContext(const std::string& context);
    const std::string& activeContext() const {return p_activeContext;}

    std::vector<HotkeyEntry> hotkeysForContext(const std::string& context) const;
    const std::vector<std::string>& contexts() const {return contextOrder;}

    bool overlayGloballyEnabled() const;
    void setOverlayGloballyEnabled(bool enabled);

    bool contextEnabled(const std::string& context) const;
    void setContextEnabled(const std::string& context, bool enabled);

signals:
    void hotkeysChanged();
    void activeContextChanged(const std::string& context);
    void contextsChanged();
    void settingsChanged();

private:
    std::unordered_map<std::string, std::vector<HotkeyEntry>> registry;
    std::vector<std::string> contextOrder;
    std::string p_activeContext;

    void ensureContextKnown(const std::string& context);
};

extern HotkeyManager* globalHotkeyManager;

#endif 