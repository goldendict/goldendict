#ifndef WEBMULTIMEDIADOWNLOAD_HH
#define WEBMULTIMEDIADOWNLOAD_HH

#include "dictionary.hh"
#include <QtNetwork>

namespace Dictionary {

/// Downloads data from the web, wrapped as a dictionary's DataRequest. This
/// is useful for multimedia files, like sounds and pronunciations.
class WebMultimediaDownload: public DataRequest
{
  Q_OBJECT

  QNetworkReply * reply;

public:

  WebMultimediaDownload( QUrl const &, QNetworkAccessManager & );

  /// Checks if the given url is an http request for an audio file.
  static bool isAudioUrl( QUrl const & );

  virtual void cancel();

private slots:

  void replyFinished( QNetworkReply * );
};

}

#endif // WEBMULTIMEDIADOWNLOAD_HH
