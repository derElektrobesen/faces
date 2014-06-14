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

inline static uint f_sum(const QVector< uint > &matrix, const QImage *img,
                          int x_s, int y_s, int w_r_scaled, int h_r_scaled) {
    uint S[] = { 1, 1, 1, 1 };
    int w = img->width();

    if (y_s < 1) {
        S[0] = 0;
        S[1] = 0;
    }
    if (x_s < 1) {
        S[0] = 0;
        S[2] = 0;
    }

    S[3] = matrix[w * (y_s + h_r_scaled - 1) + x_s + w_r_scaled - 1];
    if (S[0])
        S[0] = matrix[w * (y_s - 1) + x_s - 1];
    if (S[1])
        S[1] = matrix[w * (y_s - 1) + x_s + w_r_scaled - 1];
    if (S[2])
        S[2] = matrix[w * (y_s + h_r_scaled - 1) + x_s - 1];
    return S[3] + S[0] - S[1] - S[2];
}

#define f_sum_1(__x_s__, __y_s__, __w_r_scaled__, __h_r_scaled__) \
    f_sum(integral_matrix, in, __x_s__, __y_s__, __w_r_scaled__, __h_r_scaled__)
#define f_sum_2(__x_s__, __y_s__, __w_r_scaled__, __h_r_scaled__) \
    f_sum(matrix_of_squares, in, __x_s__, __y_s__, __w_r_scaled__, __h_r_scaled__)

void FaceRecognizer::recognize_face(double factor) {
    int d_scales[MAX_RECO_WND];
    double d_scale = 1.0;

    QVector< QRect > faces;

    int img_w = in->width();
    int img_h = in->height();

    int n_stages, wnd_w, wnd_h;
    int c_wnd_w, c_wnd_h;
    auto stages = data.get_stages(&n_stages);
    data.get_window(&wnd_w, &wnd_h);

#ifdef DEBUG
    int iter = 1;
    for (auto stage = stages; stage < stages + n_stages; stage++, iter++) {
        int nodes = 0;
        for (auto node = stage->nodes_ptr; node < stage->nodes_ptr + stage->nodes_count; node++)
            nodes += node->rects_count;
        qDebug() << "Stage" << iter << ":" << stage->nodes_count << "nodes;" << nodes << "rects.";
    }

    int count_window = 0;
    iter = 1;
#endif

    do {
        for (int i = 0; i < st_arr_len_s(d_scales); i++ )
            d_scales[i] = qFloor(i * d_scale);

        c_wnd_w = qFloor(wnd_w * d_scale);
        c_wnd_h = qFloor(wnd_h * d_scale);

        float inv = 1.0 / float(c_wnd_w * c_wnd_h);
        int x1, y1, x2, y2;
        float sum_cascade = 0;

        int x_step = qMax(1, qMin(4, c_wnd_w / 8));
        int y_step = qMax(1, qMin(4, c_wnd_h / 8));

        for (y1 = 0; y1 <= img_h - c_wnd_h - 1; y1 += y_step) {
            y2 = y1 + c_wnd_h;

            for (x1 = 0; x1 <= img_w - c_wnd_w - 1; x1 += x_step) {
                x2 = x1 + c_wnd_w;

                float mean = f_sum_1(x1, y1, c_wnd_w, c_wnd_h) * inv;
                float variance = f_sum_2(x1, y1, c_wnd_w, c_wnd_h) * inv - mean * mean;
                float stddev = 1.0;

                if (variance > 0.0)
                    stddev = qSqrt(variance);

                if (stddev < 5)
                    continue;

                // Sliding window processing
                int f_failed = 0;
                for (auto stage = stages; stage < stages + n_stages; stage++) {
                    float sum_stage = 0;
                    for (auto feature = stage->nodes_ptr; feature < stage->nodes_ptr + stage->nodes_count; feature++) {
                        long sum_feature = 0;
                        for (auto rect = feature->rects_ptr; rect < feature->rects_ptr + feature->rects_count; rect++) {
                            int x_r_scaled = d_scales[rect->x];
                            int y_r_scaled = d_scales[rect->y];
                            int w_r_scaled = d_scales[rect->width];
                            int h_r_scaled = d_scales[rect->height];

                            int x_s = x1 + x_r_scaled;
                            int y_s = y1 + y_r_scaled;

                            sum_feature += f_sum_1(x_s, y_s, w_r_scaled, h_r_scaled) * rect->weight;
                        } // rects

                        float leaf_th = feature->threshold * stddev;
                        if (sum_feature * inv < leaf_th)
                            sum_stage += feature->left_val;
                        else
                            sum_stage += feature->right_val;
                    } // features

                    sum_cascade += sum_stage;

                    if (sum_stage < stage->threshold) {
#if defined DEBUG && 0
                        qDebug() << "sum_stage =" << sum_stage << "stage_threshold =" << stage->threshold
                                    << "Window =" << x1 << y1 << x2 << y2;
#endif
                        f_failed = 1;
                        break;
                    }
                } // stages

                if (!f_failed && stddev > 25.0)
                    faces.push_back(QRect(x1, y1, x2 - x1, y2 - y1));

#ifdef DEBUG
                count_window++;
                if (!f_failed)
                    qDebug() << x1 << y1 << x2 << y2 << "[" << stddev << "]";
#endif
            }
        }

#ifdef DEBUG
        qDebug() << iter << ":" << c_wnd_w << "x" << c_wnd_h << "; scale =" << d_scale << "; windows = " << count_window;
        iter++;
#endif

        d_scale *= factor;
    } while (qMin(img_w, img_h) >= qMin(c_wnd_w, c_wnd_h));

    QPainter painter(out);
    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    for (auto it = faces.begin(); it != faces.end(); it++) {
        painter.drawRect(*it);
        qDebug() << *it;
    }
}

void FaceRecognizer::convert_to_grayscale(const QImage *img) {
    int w = img->width();
    int h = img->height();
    uchar gray;

    pixels.resize(w * h);
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            gray = qGray(img->pixel(i, j));
            pixels[j * w + i] = gray;
        }
}

#ifdef DEBUG
#define PRINT_ELEMS 5

inline static void print_matrix(const QVector< uint > &matr, int w) {
    for (int i = 0; i < PRINT_ELEMS; i++) {
        QDebug d = qDebug();
        for (int j = 0; j < PRINT_ELEMS; j++)
            d << matr[i * w + j];
    }
}

inline static void print_matrix(const QVector< uchar > &matr, int w) {
    for (int i = 0; i < PRINT_ELEMS; i++) {
        QDebug d = qDebug();
        for (int j = 0; j < PRINT_ELEMS; j++)
            d << hex << matr[i * w + j];
    }
}

inline static void print_matrix(const QImage *image) {
    for (int i = 0; i < PRINT_ELEMS; i++) {
        QDebug d = qDebug();
        for (int j = 0; j < PRINT_ELEMS; j++)
            d << hex << image->pixel(i, j);
    }
}

#undef PRINT_ELEMS
#endif

void FaceRecognizer::prepare_image(const QImage *img) {
    in = img;
    QImage gray_img(img->mirrored());
    convert_to_grayscale(&gray_img);

    int w = img->width();
    int h = img->height();

    integral_matrix.resize(h * w);
    matrix_of_squares.resize(h * w);
#if 0
    FILE *f = fopen("/tmp/integral", "rb");
    fread(&integral_matrix[0], w * h * sizeof(integral_matrix[0]), 1, f);
    fclose(f);

    f = fopen("/tmp/squares", "rb");
    fread(&matrix_of_squares[0], w * h * sizeof(matrix_of_squares[0]), 1, f);
    fclose(f);

    print_matrix(integral_matrix, w);
    qDebug() << "\nMatrix of squares";
    print_matrix(matrix_of_squares, w);
    qDebug() << "\n\n";

    return;
#endif

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            uint pixel = pixels[w * y + x];
            if (!x)
                integral_matrix[w * y] = (y ? integral_matrix[w * (y - 1)] : 0) + pixel;
            else if (!y)
                integral_matrix[x] = integral_matrix[x - 1] + pixel;
            else
                integral_matrix[w * y + x] = pixel
                        + integral_matrix[w * (y - 1) + x]
                        + integral_matrix[w * y + x - 1]
                        - integral_matrix[w * (y - 1) + x - 1];
        }

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            uint pixel = pixels[y * w + x];
            uint pow = pixel * pixel;
            if (!x)
                matrix_of_squares[w * y] = (y ? matrix_of_squares[w * (y - 1)] : 0) + pow;
            else if (!y)
                matrix_of_squares[x] = matrix_of_squares[x - 1] + pow;
            else
                matrix_of_squares[w * y + x] = pow
                        + matrix_of_squares[w * (y - 1) + x]
                        + matrix_of_squares[w * y + x - 1]
                        - matrix_of_squares[w * (y - 1) + x - 1];
        }

#ifdef DEBUG
    qDebug() << "Default image";
    print_matrix(img);
    qDebug() << "\nGray image";
    print_matrix(pixels, w);
    qDebug() << "\nIntegral matrix";
    print_matrix(integral_matrix, w);
    qDebug() << "\nMatrix of squares";
    print_matrix(matrix_of_squares, w);
    qDebug() << "\n\n";
#endif
}
