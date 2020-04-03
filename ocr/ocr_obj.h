#ifndef GDOCR_OBJ_H
#define GDOCR_OBJ_H
#include "mutex.hh"
#include <QObject>

typedef QList<QPair<QString, QString> > OCR_LAN;
typedef QList<QPair<QString /*area*/, OCR_LAN> > OCR_LANS;

class GDOcrObj : public QObject
{
    Q_OBJECT
public:
    typedef enum OCR_STAT {
        OCR_DONE_NULL = 0,
        OCR_DONE_TEXT = 1,
        OCR_PENDING = 2,
        OCR_FAILED = 3
    }OCR_STAT;

    virtual ~GDOcrObj();

    virtual QString engineName() const = 0;
    virtual bool init(const QString &path) = 0;
    virtual void enableLans(const QStringList &langs, int zone = 0, int zidxB = 0, int zidxE = 0) = 0;

    bool isWorking() const  { return _onJob.loadAcquire(); }

    static GDOcrObj *gdOcrObject(QObject *parent = nullptr, const QString &engine = QString());
    static QStringList gdOcrEngines();
    static OCR_LANS getAreaLans(const QString &engine, const QString &dataPath);

    static void prettifyOcrText(char* text, int *newLen);

public slots:
    void ocrMemory(const QByteArray &bmpdata); /* bmp data */
    void ocrFile(const QString &file);

signals:
    void errors(const QString &err);
    void textOcred(const QString &text);

protected:
    explicit GDOcrObj(QObject *parent = nullptr) : QObject(parent), _onJob(0)
    {
        connect(this, SIGNAL(delayedUnJob()), SLOT(jobDone()), Qt::QueuedConnection);
    }

    virtual OCR_STAT doOcrBMP(const QByteArray &bmpdata) = 0;
    virtual OCR_STAT doOcrFile(const QString &file) = 0;

    int postText(const QString &text);
    void postError(const QString &ProcName, int ErrorCode = 0);

    inline void onJob() { _onJob.ref(); }
protected slots:
    inline void unJob() { _onJob.storeRelease(0); }
private slots:
    void jobDone();
signals:
    void delayedUnJob();

private:
    static GDOcrObj *ocrObj;
    AtomicInt32 _onJob;
};

#endif // GDOCR_OBJ_H
