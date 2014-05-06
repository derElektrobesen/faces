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
    ImagePtr image(new QImage(imageProvider->requestImage(imageId, &imageSize, imageSize)));
    if (!image->isNull()) {
        set_image(imageId, image);
    }

    recognize(imageId, image);
}

void ImageProcessor::set_image(const QString &imageId, const ImageConstPtr &img) {
    img_provider.set_new_image(imageId, img);
    emit imageChanged(imageId);
}

ImagePtr ImageProcessor::search_face(const QImage &img) const {
    Q_UNUSED(img);  /* TODO */
    return ImagePtr(new QImage());
}

ImagePtr ImageProcessor::recognize_face(const QImage &img) const {
    Q_UNUSED(img);  /* TODO */
    return ImagePtr(new QImage());
}

void ImageProcessor::recognize(const QString &imageId, const ImageConstPtr &img) {
    set_image(imageId + "_recognized", img);
}
