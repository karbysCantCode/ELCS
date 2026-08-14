#ifndef TUTORIALOVERLAY_H
#define TUTORIALOVERLAY_H

#include <QWidget>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QPointer>
#include <QVector>
#include <QPair>

class QGraphicsView;
class QPushButton;

/*
    A transparent, dimmed overlay that sits on top of the rest of the
    UI to run a tutorial "spotlight": everything is darkened except
    the highlighted widget(s), with an instruction bubble docked at
    the bottom and (optionally) small floating name tags pointing at
    specific placed components/pins/wires on the canvas.

    Highlights are tracked *live* against the target widget(s) -- see
    highlightWidgets(). The instruction bubble and any variable-name
    tags are *always* fully painted regardless of whether they
    happen to geometrically overlap a highlighted region: their
    areas get unioned back into the paint/click mask specifically so
    a workspace-spanning highlight never silently clips them.

    A small close ("X") button is always available on the bubble,
    wired to dismissRequested() -- this is what gives the user a way
    to close a tutorial once it's done (or bail out early), since a
    trailing "you're all done!" step with no conditions has no other
    way to know it should go away.

    ----------------------------------------------------------------
    Hooking this into your Qt Creator-designed window
    ----------------------------------------------------------------
    No changes needed in the Designer/.ui file. Construct one
    directly in code, parented to whatever widget should be
    considered "the whole window" for dimming purposes -- usually
    your QMainWindow's central widget:

        overlay = new TutorialOverlay(ui->centralwidget);

    If the dimmed area itself doesn't track the window's overall size,
    it's almost always because the widget you parented it to isn't
    itself directly resized by the window -- try parenting to
    `ui->centralwidget->window()` instead, which is always the actual
    top-level widget.
    ----------------------------------------------------------------
*/
class TutorialOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit TutorialOverlay(QWidget* parent);

    // Shows (and raises) the overlay with this instruction text.
    void showInstruction(const QString& text);
    void hideInstruction();

    // Highlights exactly this set of widgets, tracked live -- pass
    // an empty list (or call clearHighlight()) to remove all
    // highlights.
    void highlightWidgets(const QVector<QWidget*>& widgets);
    void highlightWidget(QWidget* widget);

    // Highlights a rectangle in a QGraphicsView's scene coordinates,
    // also tracked live against the view. Replaces any widget
    // highlights set via highlightWidget(s)().
    void highlightSceneRect(QGraphicsView* view, const QRectF& sceneRect);

    void clearHighlight();

    // Small floating name tags, each pointing at a scene position in
    // `view` -- e.g. [("gateX", posOfGateX), ("gateY", posOfGateY)]
    // so the user can actually tell which placed component a
    // tutorial-bound variable refers to. Replaces any previous set;
    // pass an empty list to clear them. Positions are re-read from
    // `view` on every repaint, so panning/zooming stays in sync as
    // long as *something* triggers a repaint afterwards (a resize,
    // a new call to this function, etc.) -- pure panning with no
    // other state change won't proactively refresh a stale tag.
    void setVariableLabels(QGraphicsView* view, const QVector<QPair<QString, QPointF>>& labels);

    // When true, clicks outside every highlighted rect (and outside
    // the instruction bubble / variable tags, which are always
    // interactive-if-restricted-elsewhere) are swallowed -- only the
    // highlighted region(s) stay interactive. When false (the
    // default), every click passes straight through everywhere
    // except the always-clickable close button.
    void setRestrictInputToHighlight(bool restrict);

signals:
    // The user clicked the close ("X") button on the instruction
    // bubble. TutorialManager treats this as "done" if this was the
    // last step, otherwise as an early cancel.
    void dismissRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QVector<QPointer<QWidget>> highlightedWidgets;

    QGraphicsView* highlightedView = nullptr;
    QRectF highlightedSceneRect;

    QVector<QPair<QString, QPointF>> variableScenePoints;
    QGraphicsView* labelView = nullptr;

    QString instructionText;
    bool restrictInput = false;

    QPushButton* closeButton = nullptr;

    void stopTrackingHighlightedWidgets();

    QVector<QRect> computeHighlightRects() const;
    QVector<QPair<QString, QPoint>> computeVariableLabelAnchors() const;
    QRect labelTagRect(const QString& text, const QPoint& anchor) const;
    QRect instructionBubbleRect() const;

    void repositionCloseButton();

    // Re-syncs geometry to the parent, recomputes the mask from
    // current highlight/bubble/label geometry, and repaints.
    void refresh();
};

#endif