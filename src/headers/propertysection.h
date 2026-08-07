#ifndef PROPERTYSECTION_H
#define PROPERTYSECTION_H

#include <QWidget>

class QToolButton;
class QScrollArea;
class QVBoxLayout;

class PropertySection : public QWidget
{
    Q_OBJECT

public:
    explicit PropertySection(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString title() const;

    void setExpanded(bool expanded);
    bool isExpanded() const;

    void addWidget(QWidget* widget);
    void addProperty(const QString& name, QWidget* editor);

    void clear();

private slots:
    void toggleExpanded();

private:
    QToolButton* headerButton;

    QWidget* contentWidget;
    QScrollArea* scrollArea;
    QVBoxLayout* contentLayout;
};

#endif