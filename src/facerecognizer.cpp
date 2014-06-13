#include "facerecognizer.h"

FaceRecognizer::FaceRecognizer(QObject *parent) :
    QObject(parent)
{
    data.load();
}

void FaceRecognizer::recognize(const QImage &in_img, QImage &out_img) {
    if (in_img.isNull()) {
        qDebug() << "Null image given";
        return;
    }

    out = &out_img;
    prepare_image(&in_img);

    recognize_face(DEFAULT_RECO_FACTOR);
}

void FaceRecognizer::recognize_face(double factor) {
    int n_stages;
    const HaarCascade::stage *stages = data.get_stages(&n_stages);

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
    *img = img->convertToFormat(QImage::Format_Indexed8, Qt::MonoOnly);
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
    image_state_t gray(gray_bits, gray_depth, gray_bpl);
    image_state_t def(img->bits(), img->depth() / 8, img->bytesPerLine());
    print_matrixes(&gray, &def);
#endif
}

#ifdef DEBUG

#define MATR_MAX_OUT 4
#define PRINT(name, var, bpl, depth) ({ \
    qDebug() << "\n" name << "BPL = " + QString::number(bpl) + "; DEPTH = " + QString::number(depth); \
    char buf[10]; \
    for (int x = 0; x < MATR_MAX_OUT; x++) { \
        QDebug d = qDebug(); \
        for (int y = 0; y < MATR_MAX_OUT; y++) { \
            snprintf(buf, sizeof(buf), "%02X ", var[bpl * y + depth * x]); \
            d << buf; \
        } \
    } \
})

void FaceRecognizer::print_matrixes(const image_state_t *gray_st, const image_state_t *default_st) {
    int w = gray_img.width();
    PRINT("Default image", default_st->bytes, default_st->bpl, default_st->depth);
    PRINT("Gray image", gray_st->bytes, gray_st->bpl, gray_st->depth);
    PRINT("Integral matrix", integral_matrix, w, 1);
    PRINT("Matrix of squares", matrix_of_squares, w, 1);
}

#undef PRINT
#undef MATR_MAX_OUT

#endif
