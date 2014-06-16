#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include <QPainter>
#include "haarcascade.h"
#include "fann.h"

class FaceRecognizer : public QObject
{
    Q_OBJECT

public:
    explicit FaceRecognizer(QObject *parent = 0);
    ~FaceRecognizer();

    void recognize(const QImage &in_img, QImage &out_img);

protected:
    void recognize_face(double factor, QVector<QRect> &faces);
    void convert_to_grayscale(QImage *img);
    void prepare_image(const QImage *img, QImage *gray_image);
    void draw_rect(const QRect &rect, QImage *img, QColor color);
    bool choose_rect(const QVector<QRect> &recognized_faces, QImage *new_image, const QImage *gray_image);

    void add_dobeshi_conversion(const QVector<uchar> &in_data, QVector<uchar> &out_data);
    void dobeshi_conv_impl(const uchar *i_data, int len, uchar *o_data);

protected:
    void load_fann_db();
    void store_fann_db();

private:
    HaarCascade data;
    QImage *out;
    const QImage *in;

    QVector< uint > integral_matrix;
    QVector< uint > matrix_of_squares;
    QVector< uchar > pixels;

    fann *neuro_network;
    bool neural_network_changed;
};

#endif // FACERECOGNIZER_H
