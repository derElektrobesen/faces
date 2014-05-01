#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent),
    img_provider(QQuickImageProvider::Image)
{
}

void ImageProcessor::process_image(const QString &path) {
    QUrl imageUrl(path);
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    if (!engine->imageProvider(PROVIDER_HOST))
        engine->addImageProvider(PROVIDER_HOST, &img_provider);

    QQmlImageProviderBase* imageProviderBase = engine->imageProvider(imageUrl.host());
    QQuickImageProvider* imageProvider = static_cast< QQuickImageProvider * >(imageProviderBase);

    QSize imageSize;
    QString imageId = imageUrl.path().remove(0,1);
    QImage image = imageProvider->requestImage(imageId, &imageSize, imageSize);
    if (!image.isNull()) {
        set_image(imageId, image);
    }

    recognize(imageId, image);
}

void ImageProcessor::set_image(const QString &imageId, QImage &img) {
    img_provider.set_new_image(imageId, img);
    emit imageChanged(imageId);
}

void ImageProcessor::recognize(const QString &imageId, QImage &img) {
    QPainter painter;

    img.fill(Qt::red);

    painter.begin(&img);
    painter.setBrush(Qt::black);
    painter.fillRect(10, 10, 100, 100, Qt::SolidPattern);
    painter.end();

    qDebug() << img.rect();

    set_image(imageId + "_recognized", img);
}
