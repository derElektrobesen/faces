#include "trainingpreview.h"

TrainingPreview::TrainingPreview(QObject *parent) :
    QObject(parent),
    cur_index(0),
    img_provider(QQuickImageProvider::Image)
{
}

void TrainingPreview::load_images() {
    QImage *img = NULL;
    DECLARE_SQL_CON(q);

    q.prepare("select id, name from images order by id");
    q.exec();

    while (q.next()) {
        img = new QImage(q.value(1).toString());
        if (img->isNull()) {
            qDebug() << q.value(1).toString() << ": image loading failure";
            continue;
        }
        images.push_back(img_t(ImageConstPtr(img), q.value(1).toString(), q.value(0).toInt()));
    }
    q.finish();
}

void TrainingPreview::next_image() {
    if (images.isEmpty())
        load_images();
    if (images.isEmpty()) {
        qDebug() << "No images to load";
        return;
    }

    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();
    if (!engine->imageProvider("extern"))
        engine->addImageProvider("extern", &img_provider);

    if (cur_index >= images.length())
        cur_index = 0;

    const img_t &image = images.at(cur_index++);
    img_provider.set_new_image(image.path, image.img);
    emit imageChanged(QString("image://extern/" + image.path));
}
