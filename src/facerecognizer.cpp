#include "facerecognizer.h"

#ifdef STORE_INTERMIDATE_IMAGES
#   define STORE_DEBUG_IMAGE(__img__, __name__) \
    (__img__)->save(QString() + INTERMIDATE_IMAGES_PATH + __name__ + ".jpg")
#else
#   define STORE_DEBUG_IMAGE(__img__, __name__)
#endif

void FaceRecognizer::load_fann_db() {

}

void FaceRecognizer::store_fann_db() {
    fann *ann = neuro_network;


    struct fann_layer *layer_it;
    int calculated_decimal_point = 0;
    struct fann_neuron *neuron_it, *first_neuron;
    fann_type *weights;
    struct fann_neuron **connected_neurons;
    unsigned int i = 0;

#ifndef FIXEDFANN
    /* variabels for use when saving floats as fixed point variabels */
    unsigned int decimal_point = 0;
    unsigned int fixed_multiplier = 0;
    fann_type max_possible_value = 0;
    unsigned int bits_used_for_max = 0;
    fann_type current_max_value = 0;
#endif

#ifndef FIXEDFANN
    if(save_as_fixed)
    {
        /* save the version information */
        fprintf(conf, FANN_FIX_VERSION "\n");
    }
    else
    {
        /* save the version information */
        fprintf(conf, FANN_FLO_VERSION "\n");
    }
#else
    /* save the version information */
    fprintf(conf, FANN_FIX_VERSION "\n");
#endif

#ifndef FIXEDFANN
    if(save_as_fixed)
    {
        /* calculate the maximal possible shift value */

        for(layer_it = ann->first_layer + 1; layer_it != ann->last_layer; layer_it++)
        {
            for(neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron; neuron_it++)
            {
                /* look at all connections to each neurons, and see how high a value we can get */
                current_max_value = 0;
                for(i = neuron_it->first_con; i != neuron_it->last_con; i++)
                {
                    current_max_value += fann_abs(ann->weights[i]);
                }

                if(current_max_value > max_possible_value)
                {
                    max_possible_value = current_max_value;
                }
            }
        }

        for(bits_used_for_max = 0; max_possible_value >= 1; bits_used_for_max++)
        {
            max_possible_value /= 2.0;
        }

        /* The maximum number of bits we shift the fix point, is the number
         * of bits in a integer, minus one for the sign, one for the minus
         * in stepwise, and minus the bits used for the maximum.
         * This is devided by two, to allow multiplication of two fixed
         * point numbers.
         */
        calculated_decimal_point = (sizeof(int) * 8 - 2 - bits_used_for_max) / 2;

        if(calculated_decimal_point < 0)
        {
            decimal_point = 0;
        }
        else
        {
            decimal_point = calculated_decimal_point;
        }

        fixed_multiplier = 1 << decimal_point;

#ifdef DEBUG
        printf("calculated_decimal_point=%d, decimal_point=%u, bits_used_for_max=%u\n",
               calculated_decimal_point, decimal_point, bits_used_for_max);
#endif

        /* save the decimal_point on a seperate line */
        fprintf(conf, "decimal_point=%u\n", decimal_point);
    }
#else
    /* save the decimal_point on a seperate line */
    fprintf(conf, "decimal_point=%u\n", ann->decimal_point);

#endif

    /* Save network parameters */
    fprintf(conf, "num_layers=%d\n", (int)(ann->last_layer - ann->first_layer));
    fprintf(conf, "learning_rate=%f\n", ann->learning_rate);
    fprintf(conf, "connection_rate=%f\n", ann->connection_rate);
    fprintf(conf, "network_type=%u\n", ann->network_type);

    fprintf(conf, "learning_momentum=%f\n", ann->learning_momentum);
    fprintf(conf, "training_algorithm=%u\n", ann->training_algorithm);
    fprintf(conf, "train_error_function=%u\n", ann->train_error_function);
    fprintf(conf, "train_stop_function=%u\n", ann->train_stop_function);
    fprintf(conf, "cascade_output_change_fraction=%f\n", ann->cascade_output_change_fraction);
    fprintf(conf, "quickprop_decay=%f\n", ann->quickprop_decay);
    fprintf(conf, "quickprop_mu=%f\n", ann->quickprop_mu);
    fprintf(conf, "rprop_increase_factor=%f\n", ann->rprop_increase_factor);
    fprintf(conf, "rprop_decrease_factor=%f\n", ann->rprop_decrease_factor);
    fprintf(conf, "rprop_delta_min=%f\n", ann->rprop_delta_min);
    fprintf(conf, "rprop_delta_max=%f\n", ann->rprop_delta_max);
    fprintf(conf, "rprop_delta_zero=%f\n", ann->rprop_delta_zero);
    fprintf(conf, "cascade_output_stagnation_epochs=%u\n", ann->cascade_output_stagnation_epochs);
    fprintf(conf, "cascade_candidate_change_fraction=%f\n", ann->cascade_candidate_change_fraction);
    fprintf(conf, "cascade_candidate_stagnation_epochs=%u\n", ann->cascade_candidate_stagnation_epochs);
    fprintf(conf, "cascade_max_out_epochs=%u\n", ann->cascade_max_out_epochs);
    fprintf(conf, "cascade_min_out_epochs=%u\n", ann->cascade_min_out_epochs);
    fprintf(conf, "cascade_max_cand_epochs=%u\n", ann->cascade_max_cand_epochs);
    fprintf(conf, "cascade_min_cand_epochs=%u\n", ann->cascade_min_cand_epochs);
    fprintf(conf, "cascade_num_candidate_groups=%u\n", ann->cascade_num_candidate_groups);

#ifndef FIXEDFANN
    if(save_as_fixed)
    {
        fprintf(conf, "bit_fail_limit=%u\n", (int) floor((ann->bit_fail_limit * fixed_multiplier) + 0.5));
        fprintf(conf, "cascade_candidate_limit=%u\n", (int) floor((ann->cascade_candidate_limit * fixed_multiplier) + 0.5));
        fprintf(conf, "cascade_weight_multiplier=%u\n", (int) floor((ann->cascade_weight_multiplier * fixed_multiplier) + 0.5));
    }
    else
#endif
    {
        fprintf(conf, "bit_fail_limit="FANNPRINTF"\n", ann->bit_fail_limit);
        fprintf(conf, "cascade_candidate_limit="FANNPRINTF"\n", ann->cascade_candidate_limit);
        fprintf(conf, "cascade_weight_multiplier="FANNPRINTF"\n", ann->cascade_weight_multiplier);
    }

    fprintf(conf, "cascade_activation_functions_count=%u\n", ann->cascade_activation_functions_count);
    fprintf(conf, "cascade_activation_functions=");
    for(i = 0; i < ann->cascade_activation_functions_count; i++)
        fprintf(conf, "%u ", ann->cascade_activation_functions[i]);
    fprintf(conf, "\n");

    fprintf(conf, "cascade_activation_steepnesses_count=%u\n", ann->cascade_activation_steepnesses_count);
    fprintf(conf, "cascade_activation_steepnesses=");
    for(i = 0; i < ann->cascade_activation_steepnesses_count; i++)
    {
#ifndef FIXEDFANN
        if(save_as_fixed)
            fprintf(conf, "%u ", (int) floor((ann->cascade_activation_steepnesses[i] * fixed_multiplier) + 0.5));
        else
#endif
            fprintf(conf, FANNPRINTF" ", ann->cascade_activation_steepnesses[i]);
    }
    fprintf(conf, "\n");

    fprintf(conf, "layer_sizes=");
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
    {
        /* the number of neurons in the layers (in the last layer, there is always one too many neurons, because of an unused bias) */
        fprintf(conf, "%d ", (int)(layer_it->last_neuron - layer_it->first_neuron));
    }
    fprintf(conf, "\n");

#ifndef FIXEDFANN
    /* 2.1 */
    #define SCALE_SAVE( what, where )										\
        fprintf( conf, #what "_" #where "=" );								\
        for( i = 0; i < ann->num_##where##put; i++ )						\
            fprintf( conf, "%f ", ann->what##_##where[ i ] );				\
        fprintf( conf, "\n" );

    if(!save_as_fixed)
    {
        if(ann->scale_mean_in != NULL)
        {
            fprintf(conf, "scale_included=1\n");
            SCALE_SAVE( scale_mean,			in )
            SCALE_SAVE( scale_deviation,	in )
            SCALE_SAVE( scale_new_min,		in )
            SCALE_SAVE( scale_factor,		in )

            SCALE_SAVE( scale_mean,			out )
            SCALE_SAVE( scale_deviation,	out )
            SCALE_SAVE( scale_new_min,		out )
            SCALE_SAVE( scale_factor,		out )
        }
        else
            fprintf(conf, "scale_included=0\n");
    }
#undef SCALE_SAVE
#endif

    /* 2.0 */
    fprintf(conf, "neurons (num_inputs, activation_function, activation_steepness)=");
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
    {
        /* the neurons */
        for(neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron; neuron_it++)
        {
#ifndef FIXEDFANN
            if(save_as_fixed)
            {
                fprintf(conf, "(%u, %u, %u) ", neuron_it->last_con - neuron_it->first_con,
                        neuron_it->activation_function,
                        (int) floor((neuron_it->activation_steepness * fixed_multiplier) + 0.5));
            }
            else
            {
                fprintf(conf, "(%u, %u, " FANNPRINTF ") ", neuron_it->last_con - neuron_it->first_con,
                        neuron_it->activation_function, neuron_it->activation_steepness);
            }
#else
            fprintf(conf, "(%u, %u, " FANNPRINTF ") ", neuron_it->last_con - neuron_it->first_con,
                    neuron_it->activation_function, neuron_it->activation_steepness);
#endif
        }
    }
    fprintf(conf, "\n");

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
    fprintf(conf, "connections (connected_to_neuron, weight)=");
    for(i = 0; i < ann->total_connections; i++)
    {
#ifndef FIXEDFANN
        if(save_as_fixed)
        {
            /* save the connection "(source weight) " */
            fprintf(conf, "(%d, %d) ",
                    (int)(connected_neurons[i] - first_neuron),
                    (int) floor((weights[i] * fixed_multiplier) + 0.5));
        }
        else
        {
            /* save the connection "(source weight) " */
            fprintf(conf, "(%d, " FANNPRINTF ") ", (int)(connected_neurons[i] - first_neuron), weights[i]);
        }
#else
        /* save the connection "(source weight) " */
        fprintf(conf, "(%d, " FANNPRINTF ") ", (int)(connected_neurons[i] - first_neuron), weights[i]);
#endif

    }
    fprintf(conf, "\n");

    return calculated_decimal_point;
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
