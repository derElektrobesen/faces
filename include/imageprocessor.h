#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtQuick/QQuickImageProvider>
#include <QPainter>

#include "imageprovider.h"
#include "main.h"

#define PROVIDER_HOST   "processed_images"

class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    ImageProcessor(QObject *parent = 0);

signals:
    void imageChanged(const QString &imageId);

public slots:
    void process_image(const QString &path);

protected:
    void recognize(const QString &imageId, const ImageConstPtr &img);
    ImagePtr search_face(const QImage &img) const;
    ImagePtr recognize_face(const QImage &img) const;
    void set_image(const QString &imageId, const ImageConstPtr &img);

private:
    ImageProvider img_provider;

};

#endif // IMAGEPROCESSOR_H
