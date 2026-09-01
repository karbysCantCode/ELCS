#include "tutorialoverlay.h"

#include <QPainter>
#include <QGraphicsView>
#include <QPushButton>
#include <QFontMetrics>
#include <QEvent>
#include <QResizeEvent>
#include <QShowEvent>

TutorialOverlay::TutorialOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    if (parent)
    {
        parent->installEventFilter(this);
        setGeometry(parent->rect());
    }

    
    
    
    
    
    
    closeButton = new QPushButton(QString::fromUtf8("\xE2\x9C\x95"), this); 
    closeButton->setFixedSize(24, 24);
    closeButton->setToolTip("Close tutorial");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(255,255,255,35);"
        "  color: white;"
        "  border: 1px solid rgba(255,255,255,90);"
        "  border-radius: 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgba(255,255,255,70); }"
    );
    connect(closeButton, &QPushButton::clicked, this, &TutorialOverlay::dismissRequested);
    closeButton->hide();

    truthTableWidget = new TruthTableWidget(this);

    hide();
}

bool TutorialOverlay::eventFilter(QObject* watched, QEvent* event)
{
    const bool isMoveOrResize =
        event->type() == QEvent::Resize || event->type() == QEvent::Move;

    if (isMoveOrResize && watched == parentWidget())
    {
        if (parentWidget())
            setGeometry(parentWidget()->rect());

        return QWidget::eventFilter(watched, event);
    }

    
    
    
    
    if (isMoveOrResize)
    {
        for (const QPointer<QWidget>& w : highlightedWidgets)
        {
            if (w == watched)
            {
                refresh();
                break;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void TutorialOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (parentWidget())
        setGeometry(parentWidget()->rect());

    refresh();
}

void TutorialOverlay::showInstruction(const QString& text)
{
    qDebug() << "[DEBUG] TutorialOverlay::showInstruction called, text=" << text;

    instructionText = text;

    if (parentWidget())
        setGeometry(parentWidget()->rect());

    show();
    raise();
    closeButton->show();
    closeButton->raise();
    refresh();
}

void TutorialOverlay::hideInstruction()
{
    qDebug() << "[DEBUG] TutorialOverlay::hideInstruction called";

    instructionText.clear();

    
    
    
    
    
    
    restrictInput = false;
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    clearHighlight();
    variableScenePoints.clear();
    labelView = nullptr;
    closeButton->hide();
    hideTruthTable();
    hide();
}

void TutorialOverlay::stopTrackingHighlightedWidgets()
{
    for (const QPointer<QWidget>& w : highlightedWidgets)
        if (w)
            w->removeEventFilter(this);

    highlightedWidgets.clear();
}

void TutorialOverlay::highlightWidgets(const QVector<QWidget*>& widgets)
{
    stopTrackingHighlightedWidgets();

    for (QWidget* w : widgets)
    {
        if (!w)
            continue;

        highlightedWidgets.append(QPointer<QWidget>(w));
        w->installEventFilter(this);
    }

    highlightedView = nullptr;

    refresh();
}

void TutorialOverlay::highlightWidget(QWidget* widget)
{
    highlightWidgets(widget ? QVector<QWidget*>{widget} : QVector<QWidget*>{});
}

void TutorialOverlay::highlightSceneRect(QGraphicsView* view, const QRectF& sceneRect)
{
    stopTrackingHighlightedWidgets();

    highlightedView = view;
    highlightedSceneRect = sceneRect;

    if (view)
    {
        view->installEventFilter(this);
        if (view->viewport())
            view->viewport()->installEventFilter(this);
    }

    refresh();
}

void TutorialOverlay::clearHighlight()
{
    stopTrackingHighlightedWidgets();
    highlightedView = nullptr;

    refresh();
}

void TutorialOverlay::setVariableLabels(QGraphicsView* view, const QVector<QPair<QString, QPointF>>& labels)
{
    variableScenePoints = labels;
    labelView = view;

    refresh();
}

void TutorialOverlay::setRestrictInputToHighlight(bool restrict)
{
    restrictInput = restrict;
    refresh();
}

void TutorialOverlay::showTruthTable(
    const QVector<TruthTableColumn>& inputs,
    const QVector<TruthTableColumn>& outputs,
    const QVector<QVector<QString>>& expectedOutputRows
)
{
    truthTableWidget->setSpec(inputs, outputs, expectedOutputRows);
    truthTableWidget->raise();

    refresh();
}

void TutorialOverlay::hideTruthTable()
{
    truthTableWidget->clearSpec();

    refresh();
}

QVector<QRect> TutorialOverlay::computeHighlightRects() const
{
    QVector<QRect> rects;

    for (const QPointer<QWidget>& w : highlightedWidgets)
    {
        if (!w)
            continue;

        const QPoint topLeftGlobal = w->mapToGlobal(QPoint(0, 0));
        const QPoint topLeftLocal = mapFromGlobal(topLeftGlobal);

        rects.append(QRect(topLeftLocal, w->size()));
    }

    if (highlightedView)
    {
        const QRect viewportRect = highlightedView->mapFromScene(highlightedSceneRect).boundingRect();
        const QPoint topLeftGlobal = highlightedView->viewport()->mapToGlobal(viewportRect.topLeft());
        const QPoint topLeftLocal = mapFromGlobal(topLeftGlobal);

        rects.append(QRect(topLeftLocal, viewportRect.size()));
    }

    return rects;
}

QVector<QPair<QString, QPoint>> TutorialOverlay::computeVariableLabelAnchors() const
{
    QVector<QPair<QString, QPoint>> anchors;

    if (!labelView || !labelView->viewport())
        return anchors;

    for (const auto& entry : variableScenePoints)
    {
        const QPoint viewportPoint = labelView->mapFromScene(entry.second);
        const QPoint globalPoint = labelView->viewport()->mapToGlobal(viewportPoint);

        anchors.append({entry.first, mapFromGlobal(globalPoint)});
    }

    return anchors;
}

QRect TutorialOverlay::labelTagRect(const QString& text, const QPoint& anchor) const
{
    QFont font;
    font.setPointSize(10);
    font.setBold(true);

    const QFontMetrics fm(font);
    const int paddingX = 8;
    const int paddingY = 4;

    QRect tagRect(0, 0, fm.horizontalAdvance(text) + paddingX * 2, fm.height() + paddingY * 2);

    
    
    tagRect.moveCenter(QPoint(anchor.x(), anchor.y() - tagRect.height() - 10));

    return tagRect;
}

QRect TutorialOverlay::instructionBubbleRect() const
{
    QRect textRect = rect().adjusted(40, 0, -40, -40);
    textRect.setTop(textRect.bottom() - 90);
    return textRect;
}

void TutorialOverlay::repositionCloseButton()
{
    const QRect bubble = instructionBubbleRect();
    closeButton->move(bubble.right() - closeButton->width() - 10, bubble.top() + 10);
}

void TutorialOverlay::repositionTruthTable()
{
    if (!truthTableWidget->hasSpec())
        return;

    truthTableWidget->adjustSize();
    truthTableWidget->move(width() - truthTableWidget->width() - 20, 20);
}

void TutorialOverlay::refresh()
{
    repositionTruthTable();

    const QVector<QRect> highlightRects = computeHighlightRects();

    if (highlightRects.isEmpty())
    {
        clearMask();
    }
    else
    {
        QRegion region(rect());

        for (const QRect& r : highlightRects)
            region -= r;

        /*
            The instruction bubble and any variable-name tags must
            always render in full, even if they happen to
            geometrically overlap a highlighted widget (e.g. a
            workspace highlight spanning most of the window) --
            otherwise whatever part crossed into the cutout region
            just silently wasn't part of this widget any more and
            never got painted. This was the "bubble partially hidden"
            bug: reserve their areas unconditionally, regardless of
            any highlight underneath them.
        */
        if (!instructionText.isEmpty())
            region += instructionBubbleRect();

        for (const auto& anchor : computeVariableLabelAnchors())
            region += labelTagRect(anchor.first, anchor.second);

        if (truthTableWidget->hasSpec())
            region += truthTableWidget->geometry();

        setMask(region);
    }

    
    
    
    
    setAttribute(Qt::WA_TransparentForMouseEvents, !restrictInput);

    repositionCloseButton();

    qDebug() << "[DEBUG] TutorialOverlay::refresh isVisible=" << isVisible()
             << " restrictInput=" << restrictInput
             << " transparentForMouse=" << testAttribute(Qt::WA_TransparentForMouseEvents)
             << " highlightRectCount=" << highlightRects.size()
             << " geometry=" << geometry()
             << " hasMask=" << !this->mask().isEmpty();

    update();
}

void TutorialOverlay::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    refresh();
}

void TutorialOverlay::paintEvent(QPaintEvent*)
{
    
    
    
    const QVector<QRect> highlightRects = computeHighlightRects();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(0, 0, 0, 140));

    QPen highlightPen(QColor(255, 195, 0));
    highlightPen.setWidth(3);
    painter.setPen(highlightPen);
    painter.setBrush(Qt::NoBrush);

    for (const QRect& r : highlightRects)
        painter.drawRoundedRect(r.adjusted(-4, -4, 4, 4), 6, 6);

    
    
    for (const auto& anchor : computeVariableLabelAnchors())
    {
        const QRect tagRect = labelTagRect(anchor.first, anchor.second);

        painter.setPen(QPen(QColor(255, 195, 0), 2));
        painter.drawLine(QPoint(tagRect.center().x(), tagRect.bottom()), anchor.second);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 195, 0));
        painter.drawRoundedRect(tagRect, 4, 4);

        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::black);
        painter.drawText(tagRect, Qt::AlignCenter, anchor.first);
    }

    if (!instructionText.isEmpty())
    {
        const QRect textRect = instructionBubbleRect();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(20, 20, 20, 235));
        painter.drawRoundedRect(textRect, 10, 10);

        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(13);
        painter.setFont(font);
        painter.drawText(
            textRect.adjusted(18, 14, -(closeButton->width() + 24), -14),
            Qt::AlignLeft | Qt::TextWordWrap,
            instructionText
        );
    }
}