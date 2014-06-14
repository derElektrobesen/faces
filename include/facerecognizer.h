#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include <QPainter>
#include "haarcascade.h"

class FaceRecognizer : public QObject
{
    Q_OBJECT

public:
    explicit FaceRecognizer(QObject *parent = 0);

    void recognize(const QImage &in_img, QImage &out_img);

protected:
    void recognize_face(double factor);
    void convert_to_grayscale(const QImage *img);
    void prepare_image(const QImage *img);

private:
    HaarCascade data;
    QImage *out;
    const QImage *in;

    QVector< uint > integral_matrix;
    QVector< uint > matrix_of_squares;
    QVector< uchar > pixels;
};

#endif // FACERECOGNIZER_H
