#include "DuplicateResultModel.hpp"

DuplicateResultModel::DuplicateResultModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void DuplicateResultModel::setData(const std::vector<DuplicateGroup>& groups)
{
    beginResetModel();
    rows_.clear();

    for (const auto& g : groups) {
        for (const auto& f : g.files) {
            rows_.push_back({ g.size, QString::fromStdString(f.string()) });
        }
    }

    endResetModel();
}

int DuplicateResultModel::rowCount(const QModelIndex&) const
{
    return rows_.size();
}

int DuplicateResultModel::columnCount(const QModelIndex&) const
{
    return 2; // size + path
}

QVariant DuplicateResultModel::data(const QModelIndex& idx, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    const auto& r = rows_[idx.row()];

    if (idx.column() == 0)
        return QString::number(r.size);

    if (idx.column() == 1)
        return r.path;

    return {};
}

QVariant DuplicateResultModel::headerData(int section, Qt::Orientation o, int role) const
{
    if (o == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) return "Размер";
        if (section == 1) return "Файл";
    }
    return {};
}

QString DuplicateResultModel::filePathAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(rows_.size()))
        return {};

    return rows_[row].path;
}

