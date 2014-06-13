#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include "haarcascade.h"

class FaceRecognizer : public QObject
{
    Q_OBJECT

#ifdef DEBUG
    struct image_state_t {
        const uchar *bytes;
        int depth;
        int bpl; /* Bytes per line */

        image_state_t(const uchar *bytes, int depth, int bpl) :
            bytes(bytes), depth(depth), bpl(bpl) {}
    };
#endif

public:
    explicit FaceRecognizer(QObject *parent = 0);

    void recognize(const QImage &in_img, QImage &out_img);

protected:
    void recognize_face(double factor);
    void convert_to_grayscale(QImage *img);
    void prepare_image(const QImage *img);

#ifdef DEBUG
    void print_matrixes(const image_state_t *gray_st, const image_state_t *default_st);
#endif

private:
    HaarCascade data;
    QImage *out;
    const QImage *in;
    QImage gray_img;

    QVector< int > integral_matrix;
    QVector< int > matrix_of_squares;
};

#endif // FACERECOGNIZER_H
