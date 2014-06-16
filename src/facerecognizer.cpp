#include "facerecognizer.h"

#ifdef STORE_INTERMIDATE_IMAGES
#   define STORE_DEBUG_IMAGE(__img__, __name__) \
    (__img__)->save(QString() + INTERMIDATE_IMAGES_PATH + __name__ + ".jpg")
#else
#   define STORE_DEBUG_IMAGE(__img__, __name__)
#endif

enum fann_props_ids {
    FANN_ACT_FUNCS_ID = 1,
    FANN_ACT_STEEP_ID,
    FANN_LAYERS_ID,
    FANN_SCALE_MEAN_IN,
    FANN_SCALE_DEVIATION_IN,
    FANN_SCALE_NEW_MIN_IN,
    FANN_SCALE_FACTOR_IN,
    FANN_SCALE_MEAN_OUT,
    FANN_SCALE_DEVIATION_OUT,
    FANN_SCALE_NEW_MIN_OUT,
    FANN_SCALE_FACTOR_OUT
};

void FaceRecognizer::load_fann_db() {

}

void FaceRecognizer::store_fann_db() {
    fann *ann = neuro_network;
    QString query = "insert or replace into fann_params(name, value) values ";

    struct fann_layer *layer_it;
    struct fann_neuron *neuron_it, *first_neuron;
    fann_type *weights;
    struct fann_neuron **connected_neurons;
    unsigned int i = 0;

    DECLARE_SQL_CON(q);

#define Q_NO_COMMA_VAR(__var__, __name__, __val__) \
    ({ __var__ += QString() + "(" + __name__ + ", '" + QString::number(__val__) + "')"; })

#define Q_S_VAR(__var__, __name__, __val__) \
    ({ __var__ += QString() + ", ('" + __name__ + "', '" + __val__ + "')"; })

#define Q_VAR(__var__, __name__, __val__)   Q_S_VAR(__var__, __name__, QString::number(__val__))
#define Q_S(__name__, __val__)              Q_S_VAR(query, __name__, __val__)
#define Q(__name__, __val__)                Q_S(__name__, QString::number(__val__))

    /* Save network parameters */
    Q_NO_COMMA_VAR(query, "'num_layers'", (int)(ann->last_layer - ann->first_layer));
    Q("learning_rate", ann->learning_rate);
    Q("connection_rate", ann->connection_rate);
    Q("network_type", ann->network_type);

    Q("learning_momentum", ann->learning_momentum);
    Q("training_algorithm", ann->training_algorithm);
    Q("train_error_function", ann->train_error_function);
    Q("train_stop_function", ann->train_stop_function);
    Q("cascade_output_change_fraction", ann->cascade_output_change_fraction);
    Q("quickprop_decay", ann->quickprop_decay);
    Q("quickprop_mu", ann->quickprop_mu);
    Q("rprop_increase_factor", ann->rprop_increase_factor);
    Q("rprop_decrease_factor", ann->rprop_decrease_factor);
    Q("rprop_delta_min", ann->rprop_delta_min);
    Q("rprop_delta_max", ann->rprop_delta_max);
    Q("rprop_delta_zero", ann->rprop_delta_zero);
    Q("cascade_output_stagnation_epochs", ann->cascade_output_stagnation_epochs);
    Q("cascade_candidate_change_fraction", ann->cascade_candidate_change_fraction);
    Q("cascade_candidate_stagnation_epochs", ann->cascade_candidate_stagnation_epochs);
    Q("cascade_max_out_epochs", ann->cascade_max_out_epochs);
    Q("cascade_min_out_epochs", ann->cascade_min_out_epochs);
    Q("cascade_max_cand_epochs", ann->cascade_max_cand_epochs);
    Q("cascade_min_cand_epochs", ann->cascade_min_cand_epochs);
    Q("cascade_num_candidate_groups", ann->cascade_num_candidate_groups);

    Q("bit_fail_limit", ann->bit_fail_limit);
    Q("cascade_candidate_limit", ann->cascade_candidate_limit);
    Q("cascade_weight_multiplier", ann->cascade_weight_multiplier);

    Q("cascade_activation_functions_count", ann->cascade_activation_functions_count);

    q.exec("delete from fann_e_params");
    QString tmp = "insert into fann_e_params(param_type, value) values ";
    for(i = 0; i < ann->cascade_activation_functions_count; i++) {
        if (i)
            tmp += ", ";
        Q_NO_COMMA_VAR(tmp, FANN_ACT_FUNCS_ID, ann->cascade_activation_functions[i]);
    }
    if (i)
        q.exec(tmp);

    Q("cascade_activation_steepnesses_count", ann->cascade_activation_steepnesses_count);
    tmp = "insert into fann_e_params(param_type, value) values ";
    for(i = 0; i < ann->cascade_activation_steepnesses_count; i++) {
        if (i)
            tmp += ", ";
        Q_NO_COMMA_VAR(tmp, FANN_ACT_STEEP_ID, ann->cascade_activation_steepnesses[i]);
    }
    if (i)
        q.exec(tmp);

    tmp = "insert into fann_e_params(param_type, value) values ";
    i = 0;
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++) {
        if (i++)
            tmp += ", ";
        /* the number of neurons in the layers (in the last layer, there is always one too many neurons, because of an unused bias) */
        Q_NO_COMMA_VAR(tmp, FANN_LAYERS_ID, (int)(layer_it->last_neuron - layer_it->first_neuron));
    }
    if (i)
        q.exec(tmp);

#define SCALE_SAVE( what, where, type) ({                                       \
        QString tmp = "insert into fann_e_params(param_type, value) values ";   \
        uint i;                                                                 \
        for (i = 0; i < ann->num_##where##put; i++) {                           \
            if (i)                                                              \
                tmp += ", ";                                                    \
            Q_NO_COMMA_VAR(tmp, type, ann->what##_##where[i]);                  \
        }                                                                       \
        if (i)                                                                  \
            q.exec(tmp);                                                        \
})

    if(ann->scale_mean_in != NULL) {
        Q_S("scale_included", "1");
        SCALE_SAVE( scale_mean, in, FANN_SCALE_MEAN_IN );
        SCALE_SAVE( scale_deviation, in, FANN_SCALE_DEVIATION_IN );
        SCALE_SAVE( scale_new_min, in, FANN_SCALE_NEW_MIN_IN );
        SCALE_SAVE( scale_factor, in, FANN_SCALE_FACTOR_IN );

        SCALE_SAVE( scale_mean,	out, FANN_SCALE_MEAN_OUT );
        SCALE_SAVE( scale_deviation, out, FANN_SCALE_DEVIATION_OUT );
        SCALE_SAVE( scale_new_min, out, FANN_SCALE_NEW_MIN_OUT );
        SCALE_SAVE( scale_factor, out, FANN_SCALE_FACTOR_OUT );
    } else
        Q_S("scale_included", "0");
    q.exec("delete from fann_params");
    q.exec(query);

#undef SCALE_SAVE
#undef Q_NO_COMMA_VAR
#undef Q_S_VAR
#undef Q_VAR
#undef Q_S
#undef Q

    q.exec("delete from neurons");

    i = 0;
    query = "insert into neurons(num_inputs, activation_function, asctivation_steepness) values ";
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
        for(neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron; neuron_it++) {
            if (i++)
                query += ", ";
            query += "(" + QString::number(neuron_it->last_con - neuron_it->first_con) + ", " +
                    QString::number(neuron_it->activation_function) + ", " +
                    QString::number(neuron_it->activation_steepness) + ")";
        }
    if (i)
        q.exec(query);

    connected_neurons = ann->connections;
    weights = ann->weights;
    first_neuron = ann->first_layer->first_neuron;

    /* Now save all the connections.
     * We only need to save the source and the weight,
     * since the destination is given by the order.
     *
     * The weight is not saved binary due to differences
     * in binary definition of floating point numbers.
     * Especially an iPAQ does not use the same binary
     * representation as an i386 machine.
     */

    q.exec("delete from connections");

    i = 0;
    query = "insert into connections(connected_to_neuron, weight) values ";
    for(i = 0; i < ann->total_connections; i++) {
        if (i++)
            query += ", ";
        query += "(" + QString::number((int)(connected_neurons[i] - first_neuron)) + ", " + QString::number(weights[i]) + ")";
    }
    if (i)
        q.exec(query);
}

FaceRecognizer::~FaceRecognizer() {
    if (neural_network_changed && neuro_network)
        store_fann_db();
}

FaceRecognizer::FaceRecognizer(QObject *parent) :
    QObject(parent),
    neuro_network(NULL),
    neural_network_changed(false)
{
    data.load();
    load_fann_db();

#ifdef STORE_INTERMIDATE_IMAGES
    QDir d(INTERMIDATE_IMAGES_PATH);
    if (!d.exists())
        QDir().mkdir(INTERMIDATE_IMAGES_PATH);
#endif
}

#ifdef STORE_INTERMIDATE_IMAGES
inline static void store_converted_image(const QVector< uchar > &data, const char *name) {
    int s = qFloor(qSqrt(data.size()));
    QImage image(&data[0], s, s, QImage::Format_Indexed8);
    STORE_DEBUG_IMAGE(&image, name);
}
#else
#   define store_converted_image(data, name)
#endif

void FaceRecognizer::recognize(const QImage &in_img, QImage &out_img) {
    QVector< QRect > faces;
    if (in_img.isNull()) {
        qDebug() << "Null image given";
        return;
    }

    out = &out_img;
    QImage gray_image;
    prepare_image(&in_img, &gray_image);
    recognize_face(DEFAULT_RECO_FACTOR, faces);
    qDebug() << faces;

#ifdef DEBUG
    for (auto r = faces.begin(); r != faces.end(); r++)
        draw_rect(*r, out, Qt::red);
#endif

    QImage face;
    choose_rect(faces, &face, &gray_image);

    QVector< uchar > out_pixels;
    add_dobeshi_conversion(pixels, out_pixels);

    store_converted_image(out_pixels, "dobeshi_1");
}

#if 0
inline void FaceRecognizer::dobeshi_conv_impl(const uchar *i_data, int len, uchar *o_data) {
    /* in_data is a square matrix */
#define SQRT_3  1.732050808
#define SQRT_2  1.414213562
    static float filter_matrix[] = {
#ifdef USE_LOW_LEVEL_CONVERSION
        (1.0f + SQRT_3) / (4.0f * SQRT_2),
        (3.0f + SQRT_3) / (4.0f * SQRT_2),
        (3.0f - SQRT_3) / (4.0f * SQRT_2),
        (1.0f - SQRT_3) / (4.0f * SQRT_2)
#elif defined USE_HIGH_LEVEL_CONVERSION
        (1.0f - SQRT_3) / (4.0f * SQRT_2),
       -(3.0f - SQRT_3) / (4.0f * SQRT_2),
        (3.0f + SQRT_3) / (4.0f * SQRT_2),
       -(1.0f + SQRT_3) / (4.0f * SQRT_2)
#endif
    };
#undef SQRT_2
#undef SQRT_3

    for (int i = 0; i < len; i += 2, o_data++) {
        float s = 0;
        for (int j = 0; j < st_arr_len_s(filter_matrix); j++)
            s += i_data[(i + j) % len] * filter_matrix[j];
        *o_data = (uchar)qCeil(s);
    }
}

void FaceRecognizer::add_dobeshi_conversion(const QVector< uchar > &in_data, QVector< uchar > &out_data) {
    int w = (int)(qSqrt(in_data.size())); /* 2^i x 2^i [px] image */
    QVector< uchar > tmp(w), tmp1(in_data.size() / 2);
    const uchar *row = &in_data[0];
    uchar *col = &tmp[0];
    uchar *tmp1_ptr = &tmp1[0];

    out_data.resize(in_data.size() / 4);
    uchar *out = &out_data[0];

    int w2 = w / 2;
    int w4 = w2 / 2;

    for (int i = 0; i < w; i++)
        dobeshi_conv_impl(row + w * i, w, tmp1_ptr + w2 * i);
    for (int i = 0; i < w2; i++) {
        for (int j = 0; j < w2; j++)
            col[j] = tmp1_ptr[w2 * j + i];
        dobeshi_conv_impl(col, w2, col + w2);
        for (int j = 0; j < w4; j++)
            out[w4 * j + i] = col[j + w2];
    }
}
#else
inline void FaceRecognizer::dobeshi_conv_impl(const uchar *i_data, int len, uchar *o_data) {
    /* in_data is a square matrix */
#define SQRT_3  1.732050808
#define SQRT_2  1.414213562

    static float l_filter_matrix[] = {
        (1.0f + SQRT_3) / (4.0f * SQRT_2),
        (3.0f + SQRT_3) / (4.0f * SQRT_2),
        (3.0f - SQRT_3) / (4.0f * SQRT_2),
        (1.0f - SQRT_3) / (4.0f * SQRT_2)
    };
    static float h_filter_matrix[] = {
        (1.0f - SQRT_3) / (4.0f * SQRT_2),
       -(3.0f - SQRT_3) / (4.0f * SQRT_2),
        (3.0f + SQRT_3) / (4.0f * SQRT_2),
       -(1.0f + SQRT_3) / (4.0f * SQRT_2)
    };
    Q_ASSERT(sizeof(l_filter_matrix) == sizeof(h_filter_matrix));

#undef SQRT_2
#undef SQRT_3

    for (int i = 0; i < len; i += 2, o_data++) {
        float sl = 0, sh = 0;
        for (int j = 0; j < st_arr_len_s(l_filter_matrix); j++) {
            sl += i_data[(i + j) % len] * l_filter_matrix[j];
            sh += i_data[(i + j) % len] * h_filter_matrix[j];
        }
        *o_data = (uchar)qCeil(sl);
        *++o_data = (uchar)qCeil(sh);
    }
}

void FaceRecognizer::add_dobeshi_conversion(const QVector< uchar > &in_data, QVector< uchar > &out_data) {
    int w = (int)(qSqrt(in_data.size())); /* 2^i x 2^i [px] image */
    QVector< uchar > buf(in_data.size());
    out_data.resize(in_data.size());

    const uchar *in = &in_data[0];
    uchar *tmp = &buf[0];
    uchar *out = &out_data[0];

    for (int i = 0; i < w; i++)
        dobeshi_conv_impl(in + w * i, w, out + w * i);
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < w; j++)
            tmp[j] = out[w * j + i];
        dobeshi_conv_impl(tmp, w, tmp + w);
        for (int j = 0; j < w; j++)
            out[w * j + i] = tmp[w + j];
    }

    int w2 = w / 2;
    int w4 = w / 4;
    for (int i = 0; i < w4; i++)
        for (int j = 0; j < w4; j++) {
            tmp[i * w2 + j] = out[2 * i * w + j * 2];
            tmp[(i + 1) * w2 + j] = out[2 * (i + 1) * w + j * 2];
            tmp[w2 * w + i * w2 + j] = out[2 * i * w + (j + 1) * 2];
            tmp[w2 * w + (i + 1) * w2 + j] = out[2 * (i + 1) * w + (j + 1) * 2];
        }
    out_data = buf;
}
#endif

/*
 * Only one face per image expected.
 * TODO
 */
bool FaceRecognizer::choose_rect(const QVector< QRect > &recognized_faces, QImage *new_image, const QImage *gray_image) {
    if (!recognized_faces.size())
        return false;

    QRect last_rect = recognized_faces.first();

    for (auto r = recognized_faces.begin() + 1; r != recognized_faces.end(); r++) {
        if (last_rect.intersects(*r)) {
            last_rect = QRect((last_rect.left() + r->left()) / 2,
                              (last_rect.top() + r->top()) / 2,
                              (last_rect.width() + r->width()) / 2,
                              (last_rect.height() + r->height()) / 2);
        } else if (last_rect.width() < r->width())
            last_rect = *r;
    }

    draw_rect(last_rect, out, Qt::blue);

    *new_image = gray_image->copy(last_rect);
    STORE_DEBUG_IMAGE(new_image, "located_face");

    *new_image = new_image->scaled(DEFAULT_FACE_RECT_SIZE, DEFAULT_FACE_RECT_SIZE);
    STORE_DEBUG_IMAGE(new_image, "located_face_64");

    pixels.resize(new_image->byteCount() / sizeof(QRgb));
    uchar *pix_ptr = &pixels[0];
    QRgb *data = (QRgb *)new_image->bits();
    QRgb *end_ptr = (QRgb *)(new_image->bits() + new_image->byteCount());
    for (auto pix = data; pix != end_ptr; pix++, pix_ptr++)
        *pix_ptr = (uchar)(*pix & 0xff);

    return true;
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

void FaceRecognizer::recognize_face(double factor, QVector< QRect > &faces) {
    int d_scales[MAX_RECO_WND];
    double d_scale = 1.0;

    int img_w = in->width();
    int img_h = in->height();

    int n_stages, wnd_w, wnd_h;
    int c_wnd_w, c_wnd_h;
    auto stages = data.get_stages(&n_stages);
    data.get_window(&wnd_w, &wnd_h);

    faces.clear();

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

                if (stddev < 10)
                    continue;

                // Sliding window processing
                int f_failed = 0;
                for (auto stage = stages; stage < stages + n_stages; stage++) {
                    float sum_stage = 0;
                    for (auto feature = stage->nodes_ptr; feature < stage->nodes_ptr + stage->nodes_count; feature++) {
                        long sum_feature = 0;
                        for (auto rect = feature->rects_ptr; rect < feature->rects_ptr + feature->rects_count; rect++) {
                            sum_feature += f_sum_1(
                                    x1 + d_scales[rect->x],
                                    y1 + d_scales[rect->y],
                                    d_scales[rect->width],
                                    d_scales[rect->height]) * rect->weight;
                        } // rects

                        if (sum_feature * inv < feature->threshold * stddev)
                            sum_stage += feature->left_val;
                        else
                            sum_stage += feature->right_val;
                    } // features

                    sum_cascade += sum_stage;

                    if (sum_stage < stage->threshold) {
                        f_failed = 1;
                        break;
                    }
                } // stages

                if (!f_failed && stddev > 30.0)
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
}

void FaceRecognizer::convert_to_grayscale(QImage *img) {
    int w = img->width();
    int h = img->height();
    uchar gray;

    pixels.resize(w * h);
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            gray = qGray(img->pixel(i, j));
            pixels[j * w + i] = gray;
            img->setPixel(i, j, qRgb(gray, gray, gray));
        }
    STORE_DEBUG_IMAGE(img, "default_grayscaled");
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

void FaceRecognizer::prepare_image(const QImage *img, QImage *gray_img) {
    in = img;
    *gray_img = QImage(*img);
    convert_to_grayscale(gray_img);

    int w = img->width();
    int h = img->height();

    integral_matrix.resize(h * w);
    matrix_of_squares.resize(h * w);

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

void FaceRecognizer::draw_rect(const QRect &rect, QImage *img, QColor color) {
    QPainter painter(img);
    painter.setPen(color);
    painter.setBrush(Qt::NoBrush);

    painter.drawRect(rect);
}
