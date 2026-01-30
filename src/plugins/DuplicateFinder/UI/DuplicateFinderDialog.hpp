#pragma once
#include <QDialog>
#include <QListWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QPushButton>
#include "duplicate_finder.hpp"

class DuplicateFinderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DuplicateFinderDialog(QWidget* parent = nullptr);
    QString selectedPath() const { return selectedPath_; }


private slots:
    void onAddScanDir();
    void onRemoveScanDir();
    void onAddExcludeDir();
    void onRemoveExcludeDir();
    void onAddMask();
    void onRemoveMask();
    void onRunSearch();
    void onResultDoubleClicked(const QModelIndex& index);

private:
    // UI элементы
    QListWidget* scanDirsList_;
    QListWidget* excludeDirsList_;
    QListWidget* masksList_;

    QSpinBox* levelSpin_;
    QSpinBox* minSizeSpin_;
    QSpinBox* blockSizeSpin_;
    QComboBox* algoCombo_;

    QTableView* resultTable_;
    QPushButton* runButton_;
    QString selectedPath_;
};