#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtQuick/QQuickImageProvider>
#include <QPainter>

#include "imageprovider.h"
#include "main.h"
#include "databaseengine.h"
#include "facerecognizer.h"

#define PROVIDER_HOST   "processed_images"

class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    ImageProcessor(QObject *parent = 0);
    ~ImageProcessor();

signals:
    void imageChanged(const QString &imageId, const QString &name);
    void imageRecognized(const QString &name);

public slots:
    void process_image(const QString &path);
    bool can_capture();
    void set_random_image();
    void recognize();
    void update_name(const QString &new_name);

protected:
    QString recognize(const QString &imageId, const QImage &img);
    void set_image(const QString &imageId, const QImage &img, const QString &name = "");

    void register_provider();

#ifdef DB_LEARN_MODE
    void global_learn();
#endif

private:
    ImageProvider img_provider;
    QString last_path;

    FaceRecognizer recognizer;
    QMap< QString, int > names;

#ifndef PROCESS_RANDOM_IMAGE
    QVector< QPair< QString, QString > > images_paths;
    int last_image;
#else
    QString last_name;
#endif

};

#endif // IMAGEPROCESSOR_H
