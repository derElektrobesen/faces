#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent),
    img_provider(QQuickImageProvider::Image)
{
#ifndef PROCESS_RANDOM_IMAGE
    DECLARE_SQL_CON(q);
    q.exec("select t.path, n.name from test_images t join names n on n.id = t.name_id order by t.id");
    while (q.next())
        images_paths.push_back(qMakePair(q.value(0).toString(), q.value(1).toString()));
    last_image = -1;
#endif

#ifdef DB_LEARN_MODE
    qDebug() << "Global learning";
    global_learn();
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
    q.exec("select t.id, t.path, n.name from test_images t join names n on n.id = t.name_id order by random() limit 1");
    q.next();

    QImage img(q.value(1).toString());
    last_name = q.value(2).toString();
    set_image((last_path = q.value(0).toString()), img, last_name);
    qDebug() << "Image path:" << q.value(1).toString();
    q.finish();

#else
    if (++last_image >= images_paths.size())
        last_image = 0;
    QImage img(images_paths[last_image].first);
    last_path = QString::number(last_image);
    set_image(last_path, img, images_paths[last_image].second);
    qDebug() << "Image path:" << images_paths[last_image].first;
#endif

}

ImageProcessor::~ImageProcessor() {}

void ImageProcessor::recognize() {
    recognize(last_path, img_provider.get_image(last_path));
}

void ImageProcessor::set_image(const QString &imageId, const QImage &img, const QString &name) {
    img_provider.set_new_image(imageId, img);
    emit imageChanged(imageId, name);
}

void ImageProcessor::register_provider() {
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();

    if (!engine->imageProvider(PROVIDER_HOST))
        engine->addImageProvider(PROVIDER_HOST, &img_provider);
}

#ifdef DB_LEARN_MODE
void ImageProcessor::global_learn() {
    DECLARE_SQL_CON(q);
    q.exec("select t.path, n.name, t.id from test_images t join names n on n.id = t.name_id "
           "where t.id > 0 "
           "order by random()");

    QImage out;
    QString name;
    int i = 0;
    while (q.next()) {
        /* Process all images */
        QImage img(q.value(0).toString());
        if (img.isNull()) {
            qDebug() << "Image" << q.value(0).toString() << "openning failure";
            continue;
        }

        if (!recognizer.recognize(img, out, name))
            continue;

        qDebug() << "Recognized:" << name << "; real name:" << q.value(1).toString();
        recognizer.update_name(q.value(1).toString());

        if (i++ > 10) {
            i = 0;
            recognizer.store_neuro_networks();
        }

        qDebug() << "Last recognizer id:" << q.value(2).toInt();
    }
    q.finish();
}
#endif

QString ImageProcessor::recognize(const QString &imageId, const QImage &img) {
    QImage result(img);
    QString name;
    recognizer.recognize(img, result, name);
#ifndef PROCESS_RANDOM_IMAGE
    QString last_name = images_paths[last_image].second;
#endif
    set_image(imageId + "_recognized", result, last_name);
    emit imageRecognized(name);
    return name;
}

void ImageProcessor::update_name(const QString &new_name) {
    recognizer.update_name(new_name);
}
