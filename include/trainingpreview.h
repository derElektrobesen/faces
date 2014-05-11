#ifndef TRAININGPREVIEW_H
#define TRAININGPREVIEW_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <QPainter>
#include <QtQuick/QQuickImageProvider>
#include "main.h"
#include "imageprovider.h"
#include "databaseengine.h"

class TrainingPreview : public QObject
{
    Q_OBJECT

struct img_t {
    ImageConstPtr img;
    QString path;
    int id;

    img_t(const ImageConstPtr &ptr, const QString &path, int id) :
        img(ptr), path(path), id(id) {}
    img_t(const img_t &other) :
        img(other.img), path(other.path), id(other.id) {}
    img_t() : img(NULL), id(-1) {}
};

struct sign_t {
    QString sign_name;
    int sign_id;

    sign_t(const QString &name, int id) :
        sign_name(name), sign_id(id) {}
    sign_t(const sign_t &other) :
        sign_name(other.sign_name), sign_id(other.sign_id) {}
    sign_t() : sign_id(-1) {}
};

public:
    TrainingPreview(QObject *parent = 0);

signals:
    void imageChanged(const QString &imageId);
    void textChanged(const QString &text);

public slots:
    void next_image();
    void draw_rect(int x1, int y1, int x2, int y2);

protected:
    void load_images();
    void load_signs();
    void register_providers();

    void save_sign_rect(int image_id, int sign_id, const QRect &rect);

private:
    int cur_index;
    int cur_sign_id;
    const img_t *current_image;
    ImageProvider img_provider;
    ImageProvider rect_img_provider;

    QVector< img_t > images;
    QVector< sign_t > signs;

    QRect last_rect;

};

#endif // TRAININGPREVIEW_H
