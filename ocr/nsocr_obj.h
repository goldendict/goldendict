#ifndef NSOCR_OBJ_H
#define NSOCR_OBJ_H
#include "ocr_obj.h"

#include <QSize>

class NSOcrObj : public GDOcrObj
{
    Q_OBJECT
public:
    static QString ocrEngineName;
public:
    explicit NSOcrObj(QObject *parent = nullptr, const QSize &maxsize = QSize());
    ~NSOcrObj();

    static OCR_LANS getAreaLans();
    void enableLans(const QStringList &langs, int zone = 0, int zidx = 0, int zidxE = 0);

    QString engineName() const { return ocrEngineName; }


    bool init(const QString &path);

protected:
    OCR_STAT doOcrBMP(const QByteArray &p);
    OCR_STAT doOcrFile(const QString &file);

protected:
    void timerEvent(QTimerEvent *event);

private:
    OCR_STAT doOcr(int res);
    bool procOcrText();
    void enableLang(const QString &lang);
private:
    struct Pri;
    Pri *data;
};

#endif // NSOCR_OBJ_H
