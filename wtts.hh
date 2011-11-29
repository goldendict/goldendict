#ifndef WTTS_HH
#define WTTS_HH

#include "config.hh"

/// Support for any web text to speech via a templated url.
namespace WebTTS {
//QByteArray makeTTsBtn(Config::WebTts &,QString,QString);

class WebTssMaker
{
    public:
    WebTssMaker();
    WebTssMaker(Config::WebTtss const &,qint32);
    std::string MakeTssView(QString);
private:
    Config::WebTtss wts;
    QString lang;
    QString flag;
};
}

#endif // WTTS_HH
