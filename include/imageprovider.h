#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <memory>

#include "main.h"

#define MAX_IMAGES_COUNT    10

class ImageProvider : public QQuickImageProvider
{
public:
    explicit ImageProvider(ImageType type, Flags flags = 0);
    ~ImageProvider();
    virtual void set_new_image(const QString &id, const QImage &image);
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    const QImage &get_image(const QString &id);

signals:

public slots:

private:
    QMap< QString, QPair< QImage, time_t > > images;

};

#endif
