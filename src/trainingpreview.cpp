#include "trainingpreview.h"

#define EXTERN_IMAGES_PROVIDER  "extern"
#define DRAWN_RECT_PROVIDER     "drawn_rect"
#define swap(x, y) ({ auto t = x; x = y; y = t; })

TrainingPreview::TrainingPreview(QObject *parent) :
    QObject(parent),
    cur_index(0),
    cur_sign_id(0),
    img_provider(QQuickImageProvider::Image),
    rect_img_provider(QQuickImageProvider::Image)
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

void TrainingPreview::load_signs() {
    DECLARE_SQL_CON(q);

    q.prepare("select id, name, count from signs order by id");
    q.exec();

    while (q.next()) {
        QString name = q.value(1).toString();
        int id = q.value(0).toInt();
        for (int i = q.value(2).toInt(); i != 0; i--)
            signs.push_back(sign_t(name, id));
    }

    q.finish();
}

void TrainingPreview::register_providers() {
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();
    if (!engine->imageProvider(EXTERN_IMAGES_PROVIDER))
        engine->addImageProvider(EXTERN_IMAGES_PROVIDER, &img_provider);
    if (!engine->imageProvider(DRAWN_RECT_PROVIDER))
        engine->addImageProvider(DRAWN_RECT_PROVIDER, &rect_img_provider);
}

void TrainingPreview::save_sign_rect(int image_id, int sign_id, const QRect &rect) {
    DECLARE_SQL_CON(q);

    q.prepare("insert into samples(sign_id, image_id, x1, y1, x2, y2) values (?, ?, ?, ?, ?, ?)");
    q.bindValue(0, sign_id);
    q.bindValue(1, image_id);
    q.bindValue(2, rect.left());
    q.bindValue(3, rect.top());
    q.bindValue(4, rect.right());
    q.bindValue(5, rect.bottom());
    q.exec();
    q.finish();
}

void TrainingPreview::next_image() {
    if (images.isEmpty()) {
        load_images();
        load_signs();
    }
    if (images.isEmpty()) {
        qDebug() << "No images to load";
        return;
    }

    register_providers();

    if (cur_index >= images.length())
        cur_index = 0;

    const img_t *image = current_image;
    QString prefix;
    if (image) {
        if (last_rect == QRect())
            prefix = "Again: ";
        else {
            save_sign_rect(image->id, signs.at(cur_sign_id).sign_id, last_rect);
            if (++cur_sign_id >= signs.length()) {
                image = NULL;
            }
        }
    }

    if (!image) {
        cur_sign_id = 0;
        image = &(images.at(cur_index++));
        current_image = image;
    }
    emit textChanged(prefix + signs.at(cur_sign_id).sign_name);

    QString name = image->path + QString::number(rand());

    last_rect = QRect();
    img_provider.set_new_image(name, image->img);
    emit imageChanged("image://" EXTERN_IMAGES_PROVIDER "/" + name);
}

void TrainingPreview::draw_rect(int x1, int y1, int x2, int y2) {
    if (!current_image) {
        qDebug() << "No image given";
        return;
    }

    if (x1 == x2 || y1 == y2) {
        qDebug() << "Incorrect rect given";
        return;
    }

    register_providers();

    QImage *new_image = new QImage(*(current_image->img));
    QPainter painter(new_image);

    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);

    last_rect = QRect(x1, y1, x2 - x1, y2 - y1);

    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(last_rect);

    QString path = current_image->path + QString::number(rand());
    rect_img_provider.set_new_image(path, ImageConstPtr(new_image));
    emit imageChanged("image://" DRAWN_RECT_PROVIDER "/" + path);
}
