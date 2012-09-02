#include "is_online.hh"
#include <QtNetwork/QTcpSocket>

bool network_status::checkNetworkAccess(int timeout)
{
  QTcpSocket sock;
  sock.connectToHost("www.google.com", 80);
  return sock.waitForConnected(timeout);
}
