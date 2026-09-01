#include "truthtablewidget.h"
#include "component.h"
#include "styles.h"

#include <QGridLayout>
#include <QLabel>
#include <QTimer>

namespace {
constexpr int POLL_INTERVAL_MS = 100;
}

TruthTableWidget::TruthTableWidget(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(STYLESHEET_FRAME_CARD);

    grid = new QGridLayout(this);
    grid->setContentsMargins(10, 10, 10, 10);
    grid->setSpacing(4);

    pollTimer = new QTimer(this);
    pollTimer->setInterval(POLL_INTERVAL_MS);
    connect(pollTimer, &QTimer::timeout, this, &TruthTableWidget::refreshHighlight);

    hide();
}

void TruthTableWidget::setSpec(
    const QVector<TruthTableColumn>& inputs,
    const QVector<TruthTableColumn>& outputs,
    const QVector<QVector<QString>>& expected
)
{
    inputColumns = inputs;
    outputColumns = outputs;
    expectedOutputRows = expected;

    rebuildGrid();

    if (!inputColumns.isEmpty())
    {
        show();
        adjustSize();
        pollTimer->start();
        refreshHighlight();
    }
    else
    {
        clearSpec();
    }
}

void TruthTableWidget::clearSpec()
{
    inputColumns.clear();
    outputColumns.clear();
    expectedOutputRows.clear();
    cellLabels.clear();

    pollTimer->stop();

    QLayoutItem* item;
    while ((item = grid->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();

        delete item;
    }

    hide();
}

void TruthTableWidget::rebuildGrid()
{
    cellLabels.clear();

    QLayoutItem* item;
    while ((item = grid->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();

        delete item;
    }

    if (inputColumns.isEmpty())
        return;

    const int rowCount = 1 << inputColumns.size();

    auto addHeaderCell = [&](int col, const QString& text) {
        auto* label = new QLabel(text);
        label->setStyleSheet(STYLESHEET_LABEL_SECONDARY);
        label->setAlignment(Qt::AlignCenter);
        grid->addWidget(label, 0, col);
    };

    for (int c = 0; c < inputColumns.size(); c++)
        addHeaderCell(c, inputColumns[c].label);

    for (int c = 0; c < outputColumns.size(); c++)
        addHeaderCell(inputColumns.size() + c, outputColumns[c].label);

    for (int r = 0; r < rowCount; r++)
    {
        QVector<QLabel*> rowLabels;

        for (int c = 0; c < inputColumns.size(); c++)
        {
            const bool bit = (r >> (inputColumns.size() - 1 - c)) & 1;

            auto* cell = new QLabel(bit ? "1" : "0");
            cell->setAlignment(Qt::AlignCenter);
            cell->setMinimumWidth(28);
            grid->addWidget(cell, r + 1, c);
            rowLabels.push_back(cell);
        }

        for (int c = 0; c < outputColumns.size(); c++)
        {
            QString text = QString::fromUtf8("\u2013");

            if (r < expectedOutputRows.size() && c < expectedOutputRows[r].size())
                text = expectedOutputRows[r][c];

            auto* cell = new QLabel(text);
            cell->setAlignment(Qt::AlignCenter);
            cell->setMinimumWidth(28);
            grid->addWidget(cell, r + 1, inputColumns.size() + c);
            rowLabels.push_back(cell);
        }

        cellLabels.push_back(rowLabels);
    }
}

int TruthTableWidget::currentRowIndex() const
{
    int row = 0;

    for (int c = 0; c < inputColumns.size(); c++)
    {
        Pin* pin = inputColumns[c].pin;

        if (!pin)
            return -1;

        const States state = pin->getEffectingState();

        if (state != States::LOW && state != States::HIGH)
            return -1;

        row = (row << 1) | (state == States::HIGH ? 1 : 0);
    }

    return row;
}

void TruthTableWidget::refreshHighlight()
{
    if (cellLabels.isEmpty())
        return;

    const int activeRow = currentRowIndex();

    for (int r = 0; r < cellLabels.size(); r++)
    {
        const bool isActive = (r == activeRow);

        const QString rowStyle = isActive
            ? "QLabel { background: #2878d4; color: white; font-weight: 600; border-radius: 3px; padding: 2px; }"
            : "QLabel { background: transparent; color: #d5d9e0; padding: 2px; }";

        for (int c = 0; c < cellLabels[r].size(); c++)
            cellLabels[r][c]->setStyleSheet(rowStyle);

        for (int c = 0; c < outputColumns.size(); c++)
        {
            const int labelIndex = inputColumns.size() + c;

            if (labelIndex >= cellLabels[r].size())
                continue;

            if (!isActive || !outputColumns[c].pin)
                continue;

            const States state = outputColumns[c].pin->getEffectingState();

            QLabel* cell = cellLabels[r][labelIndex];

            cell->setText(
                state == States::HIGH ? "1" :
                state == States::LOW ? "0" :
                state == States::FLOATING ? "Z" :
                state == States::CONFLICT ? "!" : "?"
            );

            cell->setStyleSheet(
                QString("QLabel { background: %1; color: white; font-weight: 600; border-radius: 3px; padding: 2px; }")
                    .arg(stateColor(state).name())
            );
        }
    }
}