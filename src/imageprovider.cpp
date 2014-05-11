#include "imageprovider.h"

ImageProvider::ImageProvider(ImageType type, Flags flags) :
    QQuickImageProvider(type, flags)
{
}

void ImageProvider::set_new_image(const QString &id, const ImageConstPtr &image) {
    if (image->isNull()) {
        qDebug() << "Incorrect image given:" << id;
        return;
    }

    images[ id ] = qMakePair(image, time(NULL));

    if (images.size() > MAX_IMAGES_COUNT) {
        auto it = images.constBegin();
        time_t oldest = it.value().second;
        QString key = it.key();

        for (++it; it != images.constEnd(); it++) {
            if (it.value().second < oldest)
                key = it.key();
        }

        images.remove(key);
    }
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);

    if (images.contains(id))
        return *(images[ id ].first);
    return QImage();
}

const ImageConstPtr &ImageProvider::get_image(const QString &id) {
    if (images.contains(id))
        return images[ id ].first;
    return NULL;
}
