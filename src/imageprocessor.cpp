#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent),
    img_provider(QQuickImageProvider::Image)
{
}

void ImageProcessor::process_image(const QString &path) {
    QUrl imageUrl(path);
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    register_provider();

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

bool ImageProcessor::can_capture() {
#ifdef CAMERA_ENABLED
    return true;
#else
    return false;
#endif
}

void ImageProcessor::set_random_image() {
    DECLARE_SQL_CON(q);
    q.exec("select id, name from test_images order by random() limit 1");
    q.next();

    register_provider();

    ImagePtr img(new QImage(q.value(1).toString()));
    set_image((last_path = q.value(0).toString()), img);
    q.finish();
}

void ImageProcessor::recognize() {
    recognize(last_path, img_provider.get_image(last_path));
}

void ImageProcessor::set_image(const QString &imageId, const ImageConstPtr &img) {
    img_provider.set_new_image(imageId, img);
    emit imageChanged(imageId);
}

void ImageProcessor::register_provider() {
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    if (!engine->imageProvider(PROVIDER_HOST))
        engine->addImageProvider(PROVIDER_HOST, &img_provider);
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
    ImagePtr ptr(new QImage());
    recognizer.recognize(*img, *ptr);
    set_image(imageId + "_recognized", ptr);
}
