#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include <QPainter>
#include "haarcascade.h"
#include "fann.h"

enum reco_filter_type_t {
    HOAR_FILTER,
    D4_FILTER,
    D6_FILTER,
    D10_FILTER
};

class FaceRecognizer : public QObject
{
    Q_OBJECT

public:
    explicit FaceRecognizer(QObject *parent = 0);
    ~FaceRecognizer();

    bool recognize(const QImage &in_img, QImage &out_img, QString &name);
    void update_name(const QString &new_name);
    void store_neuro_networks();

protected:
    void recognize_face(double factor, QVector<QRect> &faces);
    void convert_to_grayscale(QImage *img);
    void prepare_image(const QImage *img, QImage *gray_image);
    void draw_rect(const QRect &rect, QImage *img, QColor color);
    QRect choose_rect(const QVector<QRect> &recognized_faces, QImage *new_image, const QImage *gray_image);

    fann *load_fann_db(int name_id);
    void store_fann_db(const fann *ann, int name_id);

    fann *create_new_neuronet(const QString &name);
    void load_people();

    bool set_filter(const QImage *img, QVector< uchar > &out, reco_filter_type_t f_type);
    QString neuro_search(const QVector<uchar> &data);

private:
    HaarCascade data;
    QImage *out;
    const QImage *in;

    QVector< uint > integral_matrix;
    QVector< uint > matrix_of_squares;
    QVector< uchar > pixels;

    QMap< QString, QPair< bool, fann * > > nets;
    QMap< QString, int > people;

    QVector< uchar > last_recognized_face;
};

#endif // FACERECOGNIZER_H
