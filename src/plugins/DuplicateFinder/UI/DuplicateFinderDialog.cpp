#include "DuplicateFinderDialog.hpp"
#include "DuplicateResultModel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QHeaderView>

DuplicateFinderDialog::DuplicateFinderDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Поиск дубликатов файлов");
    resize(900, 600);

    auto* mainLayout = new QVBoxLayout(this);

    // --- Блок директорий для сканирования ---
    auto* scanLayout = new QHBoxLayout();
    scanDirsList_ = new QListWidget();
    scanDirsList_->setMaximumHeight(80);
    auto* addScanBtn = new QPushButton("Добавить");
    auto* remScanBtn = new QPushButton("Удалить");

    connect(addScanBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onAddScanDir);
    connect(remScanBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onRemoveScanDir);

    auto* scanBtns = new QVBoxLayout();
    scanBtns->addWidget(addScanBtn);
    scanBtns->addWidget(remScanBtn);
    scanBtns->addStretch();

    scanLayout->addWidget(scanDirsList_);
    scanLayout->addLayout(scanBtns);

    mainLayout->addWidget(new QLabel("Директории для сканирования:"));
    mainLayout->addLayout(scanLayout);

    // --- Блок исключённых директорий ---
    auto* exclLayout = new QHBoxLayout();
    excludeDirsList_ = new QListWidget();
    excludeDirsList_->setMaximumHeight(80);
    auto* addExclBtn = new QPushButton("Добавить");
    auto* remExclBtn = new QPushButton("Удалить");


    connect(addExclBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onAddExcludeDir);
    connect(remExclBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onRemoveExcludeDir);

    auto* exclBtns = new QVBoxLayout();
    exclBtns->addWidget(addExclBtn);
    exclBtns->addWidget(remExclBtn);
    exclBtns->addStretch();

    exclLayout->addWidget(excludeDirsList_);
    exclLayout->addLayout(exclBtns);

    mainLayout->addWidget(new QLabel("Исключить директории:"));
    mainLayout->addLayout(exclLayout);

    // --- Параметры ---
    levelSpin_ = new QSpinBox();
    levelSpin_->setRange(-1, 100);
    levelSpin_->setValue(0);

    minSizeSpin_ = new QSpinBox();
    minSizeSpin_->setRange(1, INT_MAX);
    minSizeSpin_->setValue(1);

    blockSizeSpin_ = new QSpinBox();
    blockSizeSpin_->setRange(1, 1024 * 1024);
    blockSizeSpin_->setValue(1024);

    algoCombo_ = new QComboBox();
    algoCombo_->addItem("crc32");
    algoCombo_->addItem("md5");

    auto* paramsLayout = new QGridLayout();
    paramsLayout->addWidget(new QLabel("Уровень:"), 0, 0);
    paramsLayout->addWidget(levelSpin_, 0, 1);
    paramsLayout->addWidget(new QLabel("Мин. размер:"), 1, 0);
    paramsLayout->addWidget(minSizeSpin_, 1, 1);
    paramsLayout->addWidget(new QLabel("Размер блока:"), 2, 0);
    paramsLayout->addWidget(blockSizeSpin_, 2, 1);
    paramsLayout->addWidget(new QLabel("Алгоритм:"), 3, 0);
    paramsLayout->addWidget(algoCombo_, 3, 1);

    mainLayout->addWidget(new QLabel("Параметры поиска:"));
    mainLayout->addLayout(paramsLayout);

    // --- Маски ---
    auto* maskLayout = new QHBoxLayout();
    masksList_ = new QListWidget();
    masksList_->setMaximumHeight(80);
    auto* addMaskBtn = new QPushButton("Добавить");
    auto* remMaskBtn = new QPushButton("Удалить");

    connect(addMaskBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onAddMask);
    connect(remMaskBtn, &QPushButton::clicked, this, &DuplicateFinderDialog::onRemoveMask);

    auto* maskBtns = new QVBoxLayout();
    maskBtns->addWidget(addMaskBtn);
    maskBtns->addWidget(remMaskBtn);
    maskBtns->addStretch();

    maskLayout->addWidget(masksList_);
    maskLayout->addLayout(maskBtns);

    mainLayout->addWidget(new QLabel("Маски файлов:"));
    mainLayout->addLayout(maskLayout);

    // --- Кнопка запуска ---
    runButton_ = new QPushButton("Найти дубликаты");
    connect(runButton_, &QPushButton::clicked, this, &DuplicateFinderDialog::onRunSearch);
    mainLayout->addWidget(runButton_);

    // --- Таблица результатов ---
    resultTable_ = new QTableView();
    resultTable_->setModel(new DuplicateResultModel(this));
    resultTable_->horizontalHeader()->setStretchLastSection(true);
    resultTable_->verticalHeader()->setVisible(false);
    resultTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultTable_->setAlternatingRowColors(true);
    resultTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultTable_->setMinimumHeight(300);
    connect(resultTable_, &QTableView::doubleClicked,
        this, &DuplicateFinderDialog::onResultDoubleClicked);

    auto* header = resultTable_->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Size — компактная
    header->setSectionResizeMode(1, QHeaderView::Stretch);          // File — растягивается

    mainLayout->addWidget(resultTable_);
}

// --- Слоты ---

void DuplicateFinderDialog::onAddScanDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выбрать директорию");
    if (!dir.isEmpty())
        scanDirsList_->addItem(dir);
}

void DuplicateFinderDialog::onRemoveScanDir()
{
    delete scanDirsList_->takeItem(scanDirsList_->currentRow());
}

void DuplicateFinderDialog::onAddExcludeDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выбрать директорию");
    if (!dir.isEmpty())
        excludeDirsList_->addItem(dir);
}

void DuplicateFinderDialog::onRemoveExcludeDir()
{
    delete excludeDirsList_->takeItem(excludeDirsList_->currentRow());
}

void DuplicateFinderDialog::onAddMask()
{
    bool ok = false;
    QString mask = QInputDialog::getText(this, "Добавить маску", "*.*", QLineEdit::Normal, "", &ok);
    if (ok && !mask.isEmpty())
        masksList_->addItem(mask);
}

void DuplicateFinderDialog::onRemoveMask()
{
    delete masksList_->takeItem(masksList_->currentRow());
}

void DuplicateFinderDialog::onRunSearch()
{
    ScanParams p;

    for (int i = 0; i < scanDirsList_->count(); ++i)
        p.scanDirs.push_back(scanDirsList_->item(i)->text().toStdString());

    for (int i = 0; i < excludeDirsList_->count(); ++i)
        p.excludeDirs.push_back(excludeDirsList_->item(i)->text().toStdString());

    for (int i = 0; i < masksList_->count(); ++i)
        p.masks.push_back(masksList_->item(i)->text().toStdString());

    p.level = levelSpin_->value();
    p.minSize = minSizeSpin_->value();
    p.blockSize = blockSizeSpin_->value();
    p.algo = algoCombo_->currentText().toStdString();

    auto groups = findDuplicates(p);

    auto* model = qobject_cast<DuplicateResultModel*>(resultTable_->model());
    model->setData(groups);
}

void DuplicateFinderDialog::onResultDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    auto* model = qobject_cast<DuplicateResultModel*>(resultTable_->model());
    selectedPath_ = model->filePathAt(index.row());
    accept();
}


