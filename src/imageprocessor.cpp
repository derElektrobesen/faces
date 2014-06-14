#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent),
    img_provider(QQuickImageProvider::Image)
{}

void ImageProcessor::process_image(const QString &path) {
    QUrl imageUrl(path);
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    register_provider();

    QQmlImageProviderBase* imageProviderBase = engine->imageProvider(imageUrl.host());
    QQuickImageProvider* imageProvider = static_cast< QQuickImageProvider * >(imageProviderBase);

    QSize imageSize;
    QString imageId = imageUrl.path().remove(0,1);
    QImage image(imageProvider->requestImage(imageId, &imageSize, imageSize));
    if (!image.isNull()) {
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
    //q.exec("select id, name from test_images where id = 1");
    q.next();

    register_provider();

    QImage img(q.value(1).toString());
    set_image((last_path = q.value(0).toString()), img);
    qDebug() << "Image path:" << q.value(1).toString();
    q.finish();
}

ImageProcessor::~ImageProcessor() {}

void ImageProcessor::recognize() {
    recognize(last_path, img_provider.get_image(last_path));
}

void ImageProcessor::set_image(const QString &imageId, const QImage &img) {
    img_provider.set_new_image(imageId, img);
    emit imageChanged(imageId);
}

void ImageProcessor::register_provider() {
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    if (!engine->imageProvider(PROVIDER_HOST))
        engine->addImageProvider(PROVIDER_HOST, &img_provider);
}

void ImageProcessor::recognize(const QString &imageId, const QImage &img) {
    QImage result(img);
    recognizer.recognize(img, result);
    set_image(imageId + "_recognized", result);
}
