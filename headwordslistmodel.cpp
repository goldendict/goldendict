#include "headwordslistmodel.h"

HeadwordListModel::HeadwordListModel(QObject *parent)
    : QAbstractListModel(parent), wordsCount(0)
{}

int HeadwordListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : wordsCount;
}

int HeadwordListModel::totalCount() const{
  return totalSize;
}

bool HeadwordListModel::isFinish() const{
  return wordsCount >=totalSize;
}

QString HeadwordListModel::getRow(int row)
{
  if(fileSortedList.empty()){
    fileSortedList<<fileList;
    fileSortedList.sort();
  }
  return fileSortedList.at(row);
}

int HeadwordListModel::wordCount() const{
  return wordsCount;
}

QVariant HeadwordListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= totalSize || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        return fileList.at(index.row());
    }
    return QVariant();
}

bool HeadwordListModel::canFetchMore(const QModelIndex &parent) const
{
  if (parent.isValid())
    return false;
  return (wordsCount < totalSize);
}

void HeadwordListModel::fetchMore(const QModelIndex &parent)
{
  if (parent.isValid())
    return;
  int remainder = fileList.size() - wordsCount;
  int itemsToFetch = qMin(100, remainder);

  if (itemsToFetch <= 0)
    return;

  beginInsertRows(QModelIndex(), wordsCount, wordsCount + itemsToFetch - 1);

  wordsCount+= itemsToFetch;

  endInsertRows();

  emit numberPopulated(wordsCount);
}

void HeadwordListModel::setDict(Dictionary::Class * dict){
  _dict = dict;
  totalSize = _dict->getWordCount();
  wordsCount=0;
  QThreadPool::globalInstance()->start(
    [ this ]()
    {
      beginResetModel();
      _dict->getHeadwords( fileList );
      totalSize = fileList.size();
      emit finished(totalSize);
      endResetModel();
    } );

}

