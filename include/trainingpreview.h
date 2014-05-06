#ifndef TRAININGPREVIEW_H
#define TRAININGPREVIEW_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
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

public:
    TrainingPreview(QObject *parent = 0);

signals:
    void imageChanged(const QString &imageId);

public slots:
    void next_image();

protected:
    void load_images();

private:
    int cur_index;
    ImageProvider img_provider;

    QVector< img_t > images;

};

#endif // TRAININGPREVIEW_H
