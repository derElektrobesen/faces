#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <memory>

#include "main.h"

#define MAX_IMAGES_COUNT    10

typedef std::shared_ptr< QImage > ImagePtr;
typedef std::shared_ptr< const QImage > ImageConstPtr;

class ImageProvider : public QQuickImageProvider
{
public:
    explicit ImageProvider(ImageType type, Flags flags = 0);
    virtual void set_new_image(const QString &id, const ImageConstPtr &image);
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

signals:

public slots:

private:
    QMap< QString, QPair< ImageConstPtr, time_t > > images;

};

#endif
