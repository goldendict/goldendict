#ifndef HEADWORDSLISTMODEL_H
#define HEADWORDSLISTMODEL_H

#include "dictionary.hh"

#include <QAbstractListModel>
#include <QStringList>

class HeadwordListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    HeadwordListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int totalCount() const;
    int wordCount() const;
    bool isFinish() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QString getRow(int row);
signals:
    void numberPopulated(int number);
    void finished(int number);

public slots:
  void setDict(Dictionary::Class * dict);

protected:
  bool canFetchMore(const QModelIndex &parent) const override;
  void fetchMore(const QModelIndex &parent) override;


private:
    QStringList fileList;
    QStringList fileSortedList;
    long wordsCount;
    long totalSize;
    Dictionary::Class * _dict;
};

#endif // HEADWORDSLISTMODEL_H
