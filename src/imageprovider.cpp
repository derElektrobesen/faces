#include "imageprovider.h"

ImageProvider::ImageProvider(ImageType type, Flags flags) :
    QQuickImageProvider(type, flags)
{}

ImageProvider::~ImageProvider() {}

void ImageProvider::set_new_image(const QString &id, const QImage &image) {
    if (image.isNull()) {
        qDebug() << "Incorrect image given:" << id;
        return;
    }

    images[ id ] = qMakePair(QImage(image), time(NULL));

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
        return images[ id ].first;
    return QImage();
}

const QImage &ImageProvider::get_image(const QString &id) {
    static const QImage null_img;
    if (images.contains(id))
        return images[ id ].first;
    return null_img;
}
