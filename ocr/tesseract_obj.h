#ifndef TESSERACT_OBJ_H
#define TESSERACT_OBJ_H
#include "ocr_obj.h"
#include <QRunnable>

class TROcrObj : public GDOcrObj, public QRunnable
{
    Q_OBJECT
public:
    static QString ocrEngineName;
public:
    explicit TROcrObj(QObject *parent = nullptr);
    ~TROcrObj();

    static OCR_LANS getAreaLans(const QString &dataPath);
    void enableLans(const QStringList &langs, int zone = 0, int zidx = 0, int zidxE = 0);

    QString engineName() const { return ocrEngineName; }


    bool init(const QString &path);

    bool isImgLoaded(bool *exthread = 0);

    void run();
protected:
    OCR_STAT doOcrBMP(const QByteArray &p);
    OCR_STAT doOcrFile(const QString &file);

private:
    struct Pri;
    Pri *data;

};

#endif // TESSERACT_OBJ_H
