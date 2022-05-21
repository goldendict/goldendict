#ifndef ANKICONNECTOR_H
#define ANKICONNECTOR_H

#include "config.hh"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class AnkiConnector : public QObject
{
  Q_OBJECT
public:
  explicit AnkiConnector( QObject * parent, Config::Class const & cfg );

  void sendToAnki( QString const & word, QString const & text );

private:
  QNetworkAccessManager * mgr;
  Config::Class const & cfg;
public :
signals:
  void errorText( QString const & );
private slots:
  void finishedSlot(QNetworkReply * reply);
};

#endif // ANKICONNECTOR_H
