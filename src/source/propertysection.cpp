#include "PropertySection.h"
#include "styles.h"

#include <QToolButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>


PropertySection::PropertySection(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    headerButton = new QToolButton;

    headerButton->setCheckable(true);
    headerButton->setChecked(true);
    headerButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    headerButton->setArrowType(Qt::DownArrow);
    headerButton->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
    );

    headerButton->setStyleSheet(
        STYLESHEET_PROPERTY_SECTION_HEADER
    );


    connect(
        headerButton,
        &QToolButton::clicked,
        this,
        &PropertySection::toggleExpanded
    );


    contentWidget = new QWidget;

    contentWidget->setStyleSheet(
        STYLESHEET_PROPERTY_CONTENT
    );


    contentLayout = new QVBoxLayout(contentWidget);

    contentLayout->setContentsMargins(
        8,
        8,
        8,
        8
    );

    contentLayout->setSpacing(6);

    contentLayout->addStretch();



    scrollArea = new QScrollArea;

    scrollArea->setWidgetResizable(true);

    scrollArea->setWidget(contentWidget);

    scrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    scrollArea->setFrameShape(
        QFrame::NoFrame
    );

    scrollArea->setStyleSheet(
        STYLESHEET_PROPERTY_SECTION_SCROLLAREA
    );



    auto* layout = new QVBoxLayout(this);

    layout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    layout->setSpacing(2);


    layout->addWidget(headerButton);

    layout->addWidget(scrollArea);
}



void PropertySection::setTitle(const QString& title)
{
    headerButton->setText(title);
}


QString PropertySection::title() const
{
    return headerButton->text();
}



void PropertySection::setExpanded(bool expanded)
{
    headerButton->setChecked(expanded);

    headerButton->setArrowType(
        expanded
        ? Qt::DownArrow
        : Qt::RightArrow
    );

    scrollArea->setVisible(expanded);
}



bool PropertySection::isExpanded() const
{
    return headerButton->isChecked();
}



void PropertySection::toggleExpanded()
{
    setExpanded(
        headerButton->isChecked()
    );
}



void PropertySection::addWidget(QWidget* widget)
{
    contentLayout->insertWidget(
        contentLayout->count() - 1,
        widget
    );
}



void PropertySection::addProperty(
    const QString& name,
    QWidget* editor
)
{
    auto* row = new QWidget;

    row->setStyleSheet(
        STYLESHEET_PROPERTY_ROW
    );


    auto* layout = new QHBoxLayout(row);

    layout->setContentsMargins(
        8,
        4,
        8,
        4
    );

    layout->setSpacing(10);



    auto* label = new QLabel(name);

    label->setMinimumWidth(120);



    layout->addWidget(label);

    layout->addWidget(
        editor,
        1
    );


    contentLayout->insertWidget(
        contentLayout->count() - 1,
        row
    );
}



void PropertySection::clear()
{
    while (contentLayout->count() > 1)
    {
        auto item = contentLayout->takeAt(0);

        if (item->widget())
            delete item->widget();

        delete item;
    }
}