#include "facerecognizer.h"

FaceRecognizer::FaceRecognizer(QObject *parent) :
    QObject(parent)
{
    data.load();
}

void FaceRecognizer::recognize(const QImage &in_img, QImage &out_img) {
    out = &out_img;
    prepare_image(&in_img);
}

void FaceRecognizer::recognize_face(double factor) {
    Q_UNUSED(factor);
}

void FaceRecognizer::convert_to_grayscale(QImage *img) {
    int w = img->width();
    int h = img->height();
    int gray;

    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++) {
            gray = qGray(img->pixel(i, j));
            img->setPixel(i, j, qRgb(gray, gray, gray));
        }
}

void FaceRecognizer::prepare_image(const QImage *img) {
    in = img;
    gray_img = img->mirrored();
    convert_to_grayscale(&gray_img);

    int gray_depth = gray_img.depth() / 8;
    int w = img->width();
    int h = img->height();

    int gray_bpl = gray_img.bytesPerLine();
    uchar *gray_bits = gray_img.bits();

    integral_matrix.resize(h * w);
    matrix_of_squares.resize(h * w);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            if (!x)
                integral_matrix[w * y] = (y ? integral_matrix[w * (y - 1)] : 0) + gray_bits[gray_bpl * y];
            else if (!y)
                integral_matrix[x] = integral_matrix[x - 1] + gray_bits[gray_depth * x];
            else
                integral_matrix[w * y + x] = gray_bits[gray_bpl * y + gray_depth * x]
                        + integral_matrix[w * (y - 1) + x]
                        + integral_matrix[w * y + x - 1]
                        + integral_matrix[w * (y - 1) + x - 1];
        }

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int pow = gray_bits[gray_depth * y + gray_bpl * x] * gray_bits[gray_depth * y + gray_bpl * x];
            if (!x)
                matrix_of_squares[w * y] = (y ? matrix_of_squares[w * (y - 1)] : 0) + pow;
            else if (!y)
                matrix_of_squares[x] = matrix_of_squares[x - 1] + pow;
            else
                matrix_of_squares[w * y + x] = pow
                        + matrix_of_squares[w * (y - 1) + x]
                        + matrix_of_squares[w * y + x - 1]
                        + matrix_of_squares[w * (y - 1) + x - 1];
        }
#ifdef DEBUG
    print_matrixes();
#endif
}

#ifdef DEBUG
#define MATR_MAX_OUT 4
void FaceRecognizer::print_matrixes() {
    int w = gray_img.width();
    for (int x = 0; x < MATR_MAX_OUT; x++) {
        QDebug d = qDebug();
        for (int y = 0; y < MATR_MAX_OUT; y++)
            d << integral_matrix[w * y + x];
    }
    qDebug() << "\n";

    for (int x = 0; x < MATR_MAX_OUT; x++) {
        QDebug d = qDebug();
        for (int y = 0; y < MATR_MAX_OUT; y++)
            d << matrix_of_squares[w * y + x];
    }
}
#endif
