#ifndef TRUTHTABLEWIDGET_H
#define TRUTHTABLEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QString>

class Pin;
class QLabel;
class QGridLayout;
class QTimer;

struct TruthTableColumn
{
    QString label;
    Pin* pin = nullptr;
};

class TruthTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TruthTableWidget(QWidget* parent = nullptr);

    void setSpec(
        const QVector<TruthTableColumn>& inputs,
        const QVector<TruthTableColumn>& outputs,
        const QVector<QVector<QString>>& expectedOutputRows = {}
    );

    void clearSpec();

    bool hasSpec() const { return !inputColumns.isEmpty(); }

private slots:
    void refreshHighlight();

private:
    QVector<TruthTableColumn> inputColumns;
    QVector<TruthTableColumn> outputColumns;
    QVector<QVector<QString>> expectedOutputRows;

    QGridLayout* grid = nullptr;
    QVector<QVector<QLabel*>> cellLabels;

    QTimer* pollTimer = nullptr;

    void rebuildGrid();
    int currentRowIndex() const;
};

#endif // TRUTHTABLEWIDGET_H