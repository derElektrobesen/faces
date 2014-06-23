#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent),
    img_provider(QQuickImageProvider::Image)
{
#ifndef PROCESS_RANDOM_IMAGE
    DECLARE_SQL_CON(q);
    q.exec("select name from test_images order by id limit 20");
    while (q.next())
        images_paths.push_back(q.value(0).toString());
    last_image = -1;
#endif
}

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
    register_provider();

#ifdef PROCESS_RANDOM_IMAGE
    DECLARE_SQL_CON(q);
    q.exec("select id, name from test_images order by random() limit 1");
    //q.exec("select id, name from test_images where id = 1");
    q.next();

    QImage img(q.value(1).toString());
    set_image((last_path = q.value(0).toString()), img);
    qDebug() << "Image path:" << q.value(1).toString();
    q.finish();
#else
    if (++last_image >= images_paths.size())
        last_image = 0;
    QImage img(images_paths[last_image]);
    last_path = QString::number(last_image);
    set_image(last_path, img);
    qDebug() << "Image path:" << images_paths[last_image];
#endif

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

void ImageProcessor::update_name(const QString &new_name) {
    recognizer.update_name(new_name);
}
