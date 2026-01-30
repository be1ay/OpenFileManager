#pragma once
#include <QAbstractTableModel>
#include "duplicate_finder.hpp"

class DuplicateResultModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DuplicateResultModel(QObject* parent = nullptr);

    void setData(const std::vector<DuplicateGroup>& groups);

    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int role) const override;
    QVariant headerData(int section, Qt::Orientation, int role) const override;
    QString filePathAt(int row) const;

private:
    struct Row {
        std::size_t size;
        QString path;
    };

    std::vector<Row> rows_;
};
