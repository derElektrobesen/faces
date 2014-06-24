#include "facerecognizer.h"
#include <float.h>

#ifdef STORE_INTERMIDATE_IMAGES
#   define STORE_DEBUG_IMAGE(__img__, __name__) \
    (__img__)->save(QString() + INTERMIDATE_IMAGES_PATH + __name__ + ".jpg")
#else
#   define STORE_DEBUG_IMAGE(__img__, __name__)
#endif

enum fann_props_ids {
    FANN_PROP_num_layers = 1,
    FANN_PROP_learning_rate,
    FANN_PROP_connection_rate,
    FANN_PROP_network_type,
    FANN_PROP_learning_momentum,
    FANN_PROP_training_algorithm,
    FANN_PROP_train_error_function,
    FANN_PROP_train_stop_function,
    FANN_PROP_cascade_output_change_fraction,
    FANN_PROP_quickprop_decay,
    FANN_PROP_quickprop_mu,
    FANN_PROP_rprop_increase_factor,
    FANN_PROP_rprop_decrease_factor,
    FANN_PROP_rprop_delta_min,
    FANN_PROP_rprop_delta_max,
    FANN_PROP_rprop_delta_zero,
    FANN_PROP_cascade_output_stagnation_epochs,
    FANN_PROP_cascade_candidate_change_fraction,
    FANN_PROP_cascade_candidate_stagnation_epochs,
    FANN_PROP_cascade_max_out_epochs,
    FANN_PROP_cascade_min_out_epochs,
    FANN_PROP_cascade_max_cand_epochs,
    FANN_PROP_cascade_min_cand_epochs,
    FANN_PROP_cascade_num_candidate_groups,
    FANN_PROP_bit_fail_limit,
    FANN_PROP_cascade_candidate_limit,
    FANN_PROP_cascade_weight_multiplier,
    FANN_PROP_scale_included,
    FANN_PROP_cascade_activation_functions_count,
    FANN_PROP_cascade_activation_functions,
    FANN_PROP_cascade_activation_steepnesses_count,
    FANN_PROP_cascade_activation_steepnesses,
    FANN_PROP_layers_sizes,

    FANN_PROP_scales,
    FANN_PROP_scale_mean_in,
    FANN_PROP_scale_deviation_in,
    FANN_PROP_scale_new_min_in,
    FANN_PROP_scale_factor_in,
    FANN_PROP_scale_mean_out,
    FANN_PROP_scale_deviation_out,
    FANN_PROP_scale_new_min_out,
    FANN_PROP_scale_factor_out
};

fann *FaceRecognizer::load_fann_db(int name_id) {
    DECLARE_SQL_CON(q);
    const QString name_str = QString::number(name_id);
    fann *ann = NULL;
    bool scale_included = false;

    q.exec("select value from fann_params where net_id = " + name_str +
           " and prop_id = " + QString::number(FANN_PROP_num_layers));
    q.next();
    ann = fann_allocate_structure(q.value(1).toString().toInt());
    if (!ann)
        return NULL;
    q.finish();

    q.exec("select value from fann_e_params where net_id = " + name_str +
           " and prop_id = " + QString::number(FANN_PROP_layers_sizes) +
           " order by id");

    for(auto layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
    {
        q.next();
        uint layer_size = q.value(0).toString().toUInt();
        /* we do not allocate room here, but we make sure that
         * last_neuron - first_neuron is the number of neurons */
        layer_it->first_neuron = NULL;
        layer_it->last_neuron = layer_it->first_neuron + layer_size;
        ann->total_neurons += layer_size;
    }
    q.finish();
    ann->num_input = ann->first_layer->last_neuron - ann->first_layer->first_neuron - 1;
    ann->num_output = ((ann->last_layer - 1)->last_neuron - (ann->last_layer - 1)->first_neuron);

    q.exec("select prop_id, value from fann_params where net_id = " + name_str +
           " and prop_id < " + QString::number(FANN_PROP_scales) + " order by prop_id");

#define QQ(__var__, __f__, __conv__) \
    case FANN_PROP_##__var__: ann->__var__ = (__conv__)(q.value(1).toString().__f__()); break;
#define QF(__var__) QQ(__var__, toFloat, float)
#define QU(__var__) QQ(__var__, toUInt, uint)
#define QE(__var__, __enum__) QQ(__var__, toUInt, __enum__)

    while (q.next()) {
        switch (q.value(0).toInt()) {
            case FANN_PROP_num_layers:
                break;
            QF(learning_rate)
            QF(connection_rate)
            QF(learning_momentum)
            QF(cascade_output_change_fraction)
            QF(quickprop_decay)
            QF(quickprop_mu)
            QF(rprop_increase_factor)
            QF(rprop_decrease_factor)
            QF(rprop_delta_min)
            QF(rprop_delta_max)
            QF(rprop_delta_zero)
            QF(cascade_candidate_change_fraction)
            QF(bit_fail_limit)
            QF(cascade_candidate_limit)
            QF(cascade_weight_multiplier)
            QU(cascade_output_stagnation_epochs)
            QU(cascade_candidate_stagnation_epochs)
            QU(cascade_max_out_epochs)
            QU(cascade_min_out_epochs)
            QU(cascade_max_cand_epochs)
            QU(cascade_min_cand_epochs)
            QU(cascade_num_candidate_groups)
            QU(cascade_activation_functions_count)
            QU(cascade_activation_steepnesses_count)
            QE(network_type, fann_nettype_enum)
            QE(training_algorithm, fann_train_enum)
            QE(train_error_function, fann_errorfunc_enum)
            QE(train_stop_function, fann_stopfunc_enum)
            case FANN_PROP_scale_included:
                scale_included = q.value(1).toString().toInt();
                break;
            default:
                qDebug() << "Incorrect option:" << q.value(0).toInt();
                break;
        }
    }

    q.finish();
    q.prepare("select prop_id, value from fann_e_params where net_id = ? and prop_id in (?, ?) order by id");
    q.bindValue(0, name_id);
    q.bindValue(1, FANN_PROP_cascade_activation_functions);
    q.bindValue(2, FANN_PROP_cascade_activation_steepnesses);
    q.exec();

    /* TODO: Check returned values */
    ann->cascade_activation_functions =
        (enum fann_activationfunc_enum *)realloc(ann->cascade_activation_functions,
        ann->cascade_activation_functions_count * sizeof(enum fann_activationfunc_enum));
    ann->cascade_activation_steepnesses =
        (fann_type *)realloc(ann->cascade_activation_steepnesses,
        ann->cascade_activation_steepnesses_count * sizeof(fann_type));

    int i = 0, j = 0;
    while (q.next()) {
        switch (q.value(0).toInt()) {
            case FANN_PROP_cascade_activation_functions:
                ann->cascade_activation_functions[i++] = (fann_activationfunc_enum)q.value(1).toString().toUInt();
                break;
            case FANN_PROP_cascade_activation_steepnesses:
                ann->cascade_activation_steepnesses[j++] = q.value(1).toString().toFloat();
                break;
            default:
                break;
        }
    }

    if (ann->network_type == FANN_NETTYPE_LAYER)
        ann->num_output--;

#define SCALE_LOAD( what, where ) case FANN_PROP_##what##_##where: {            \
        float val = q.value(1).toString().toFloat();                            \
        int *index = indexes + q.value(0).toInt() - FANN_PROP_scales - 1;       \
        ann->what##_##where[*index++] = val;                                    \
        break;                                                                  \
}

    int indexes[8];
    bzero(&indexes, sizeof(indexes));
    if (scale_included) {
        q.prepare("select prop_id, value from fann_e_params where net_id = ? and prop_id > ? order by id");
        q.bindValue(0, name_id);
        q.bindValue(1, FANN_PROP_scales);
        q.exec();
        while (q.next()) {
            switch (q.value(0).toInt()) {
                SCALE_LOAD( scale_mean, in );
                SCALE_LOAD( scale_deviation, in );
                SCALE_LOAD( scale_new_min, in );
                SCALE_LOAD( scale_factor, in );

                SCALE_LOAD( scale_mean,	out );
                SCALE_LOAD( scale_deviation, out );
                SCALE_LOAD( scale_new_min, out );
                SCALE_LOAD( scale_factor, out );

                default:
                    break;
            }
        }
    }

#undef SCALE_LOAD
#undef QQ
#undef QU
#undef QF
#undef QE

    fann_allocate_neurons(ann);
    if (ann->errno_f == FANN_E_CANT_ALLOCATE_MEM) {
        fann_destroy(ann);
        return NULL;
    }

    q.exec("select num_inputs, activation_function, activation_steepness from neurons where net_id = " +
           name_str + " order by id");

    auto last_neuron = (ann->last_layer - 1)->last_neuron;
    for (auto neuron_it = ann->first_layer->first_neuron; neuron_it != last_neuron; neuron_it++) {
        if (q.next()) {
            neuron_it->activation_function = (enum fann_activationfunc_enum)q.value(1).toInt();
            neuron_it->first_con = ann->total_connections;
            ann->total_connections += q.value(0).toInt();
            neuron_it->last_con = ann->total_connections;
            neuron_it->activation_steepness = q.value(3).toFloat();
        } else {
            qDebug() << "Incorrect number of neurons";
            fann_destroy(ann);
            return NULL;
        }
    }
    q.finish();

    fann_allocate_connections(ann);
    if(ann->errno_f == FANN_E_CANT_ALLOCATE_MEM) {
        fann_destroy(ann);
        return NULL;
    }

    auto connected_neurons = ann->connections;
    auto weights = ann->weights;
    auto first_neuron = ann->first_layer->first_neuron;

    q.exec("select connected_to_neuron, weight from connections where net_id = " + name_str + " order by id");

    for (uint i = 0; i < ann->total_connections; i++) {
        if (!q.next()) {
            qDebug() << "Incorrect number of connections";
            fann_destroy(ann);
            return NULL;
        }
        weights[i] = q.value(1).toFloat();
        connected_neurons[i] = first_neuron + q.value(0).toInt();
    }
    q.finish();

    return ann;
}

fann *FaceRecognizer::create_new_neuronet(const QString &name) {
    fann *net = fann_create_standard(3,
                    DEFAULT_FACE_RECT_SIZE * DEFAULT_FACE_RECT_SIZE,
                    DEFAULT_FACE_RECT_SIZE * 4,
                    1);
    if (!net) {
        qDebug() << "fann_create_standard failure";
        return NULL;
    }
    fann_set_activation_function_hidden(net, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(net, FANN_SIGMOID_SYMMETRIC);

    DECLARE_SQL_CON(q);

    int id = -1;
    q.exec("select id from names where name = '" + name + "'");
    if (q.next())
        id = q.value(0).toInt();
    else {
        q.finish();
        q.prepare("insert into names(name) values (?)");
        q.bindValue(0, name);
        q.exec();
        id = q.lastInsertId().toInt();
    }
    q.finish();

    q.prepare("insert into neuro_networks(name_id) values (?)");
    q.bindValue(0, id);
    q.exec();

    nets[name] = qMakePair(false, net);
    people[name] = q.lastInsertId().toInt();

    q.finish();

    return net;
}

void FaceRecognizer::store_fann_db(const fann *ann, int name_id) {
    QString query = "replace into fann_params(net_id, prop_id, value) values ";

    struct fann_layer *layer_it;
    struct fann_neuron *neuron_it, *first_neuron;
    fann_type *weights;
    struct fann_neuron **connected_neurons;
    unsigned int i = 0;

    DECLARE_SQL_CON(q);
    q.exec("begin transaction");
    QString name_id_s = QString::number(name_id);

#define Q(__var__) \
    ({ QString() + "(" + name_id_s + ", " + QString::number(FANN_PROP_##__var__) + \
            ", '" + QString::number(ann->__var__) + "')"; })
#define QV(__var__, __val__) \
    ({ QString() + "(" + name_id_s + ", " + QString::number(FANN_PROP_##__var__) + \
            ", '" + QString::number(__val__) + "')"; })
#define QQ(__var__) ({ query += ", " + Q(__var__); })

    /* Save network parameters */
    query += QV(num_layers, (int)(ann->last_layer - ann->first_layer));
    QQ(learning_rate);
    QQ(connection_rate);
    QQ(network_type);
    QQ(learning_momentum);
    QQ(training_algorithm);
    QQ(train_error_function);
    QQ(train_stop_function);
    QQ(cascade_output_change_fraction);
    QQ(quickprop_decay);
    QQ(quickprop_mu);
    QQ(rprop_increase_factor);
    QQ(rprop_decrease_factor);
    QQ(rprop_delta_min);
    QQ(rprop_delta_max);
    QQ(rprop_delta_zero);
    QQ(cascade_output_stagnation_epochs);
    QQ(cascade_candidate_change_fraction);
    QQ(cascade_candidate_stagnation_epochs);
    QQ(cascade_max_out_epochs);
    QQ(cascade_min_out_epochs);
    QQ(cascade_max_cand_epochs);
    QQ(cascade_min_cand_epochs);
    QQ(cascade_num_candidate_groups);
    QQ(bit_fail_limit);
    QQ(cascade_candidate_limit);
    QQ(cascade_weight_multiplier);
    QQ(cascade_activation_functions_count);
    QQ(cascade_activation_steepnesses_count);

    query += ", " + QV(scale_included, ann->scale_mean_in ? 0 : 1);
    q.exec(query);

#define QUERY_OVERFLOW(__i__, __q__, __new_q__) ({ \
        if (__i__ >= SQL_MAX_INSERT) { \
            q.exec(query); \
            __i__ = 0; \
            __q__ = __new_q__; \
        } \
})

    QString req_head = "replace into fann_e_params(net_id, prop_id, value) values ";
    query = req_head;
    for(i = 0; i < ann->cascade_activation_functions_count; i++) {
        if (i)
            query += ", ";
        query += QV(cascade_activation_functions, ann->cascade_activation_functions[i]);
        QUERY_OVERFLOW(i, query, req_head);
    }
    if (i)
        q.exec(query);

    query = req_head;
    for(i = 0; i < ann->cascade_activation_steepnesses_count; i++) {
        if (i)
            query += ", ";
        query += QV(cascade_activation_steepnesses, ann->cascade_activation_steepnesses[i]);
        QUERY_OVERFLOW(i, query, req_head);
    }
    if (i)
        q.exec(query);

    i = 0;
    query = req_head;
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++) {
        if (i++)
            query += ", ";
        /* the number of neurons in the layers (in the last layer, there is always one too many neurons, because of an unused bias) */
        query += QV(layers_sizes, (int)(layer_it->last_neuron - layer_it->first_neuron));
        QUERY_OVERFLOW(i, query, req_head);
    }
    if (i)
        q.exec(query);

#define SCALE_SAVE( what, where ) ({                                            \
        QString tmp = req_head;                                                 \
        uint i;                                                                 \
        for (i = 0; i < ann->num_##where##put; i++) {                           \
            if (i)                                                              \
                tmp += ", ";                                                    \
            tmp += QV(what##_##where, ann->what##_##where[i]);                  \
            QUERY_OVERFLOW(i, tmp, req_head);                                   \
        }                                                                       \
        if (i)                                                                  \
            q.exec(tmp);                                                        \
})

    if(ann->scale_mean_in != NULL) {
        SCALE_SAVE( scale_mean, in );
        SCALE_SAVE( scale_deviation, in );
        SCALE_SAVE( scale_new_min, in );
        SCALE_SAVE( scale_factor, in );

        SCALE_SAVE( scale_mean,	out );
        SCALE_SAVE( scale_deviation, out );
        SCALE_SAVE( scale_new_min, out );
        SCALE_SAVE( scale_factor, out );
    }

#undef SCALE_SAVE
#undef Q
#undef QQ
#undef QV

    q.exec("delete from neurons where net_id = " + name_id_s);

    i = 0;
    req_head = query = "insert into neurons(net_id, num_inputs, activation_function, activation_steepness) values ";
    for(layer_it = ann->first_layer; layer_it != ann->last_layer; layer_it++)
        for(neuron_it = layer_it->first_neuron; neuron_it != layer_it->last_neuron; neuron_it++) {
            if (i++)
                query += ", ";
            query += "(" + name_id_s + ", " + QString::number(neuron_it->last_con - neuron_it->first_con) + ", " +
                    QString::number(neuron_it->activation_function) + ", " +
                    QString::number(neuron_it->activation_steepness) + ")";
            QUERY_OVERFLOW(i, query, req_head);
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

    i = 0;
    q.exec("delete from connections where net_id = " + name_id_s);
    req_head = query = "insert into connections(net_id, connected_to_neuron, weight) values ";
    qDebug() << ann->total_connections;
    for(i = 0; i < ann->total_connections; i++) {
        if (i++)
            query += ", ";
        query += "(" + name_id_s + ", " +
                QString::number((int)(connected_neurons[i] - first_neuron)) + ", " + QString::number(weights[i]) + ")";
        QUERY_OVERFLOW(i, query, req_head);
    }
    if (i)
        q.exec(query);
    q.exec("commit");
}

void FaceRecognizer::store_neuro_networks() {
    for (auto it = nets.begin(); it != nets.end(); it++) {
        if (it.value().first) {
            /* Net changed */
#ifdef STORE_NNET_IN_F
            fann_save(it.value().second, (QString(WORK_DIR "/" NET_PREFIX) +
                                          QString::number(people[it.key()]) + ".net").toUtf8());
#else
            store_fann_db(it.value().second, people[it.key()]);
#endif
            it.value().first = false;
        }
    }
    qDebug() << "Neuro networks storred";
}

FaceRecognizer::~FaceRecognizer() {
    store_neuro_networks();
    for (auto it = nets.begin(); it != nets.end(); it++)
        fann_destroy(it.value().second);
}

void FaceRecognizer::load_people() {
    DECLARE_SQL_CON(q);
    q.exec("select nn.id, n.name from neuro_networks nn join names n on n.id = nn.name_id");

    while (q.next()) {
#ifdef STORE_NNET_IN_F
        fann *ann = fann_create_from_file((QString(WORK_DIR "/" NET_PREFIX) + q.value(0).toString() + ".net").toUtf8());
#else
        fann *ann = load_fann_db(q.value(0).toInt());
#endif
        if (!ann) {
            qDebug() << "FANN loading failure!";
            return;
        }
        nets[q.value(1).toString()] = qMakePair(false, ann);
        people[q.value(1).toString()] = q.value(0).toInt();
    }
}

FaceRecognizer::FaceRecognizer(QObject *parent) :
    QObject(parent)
{
    data.load();
    load_people();

#ifdef STORE_INTERMIDATE_IMAGES
    QDir d(INTERMIDATE_IMAGES_PATH);
    if (!d.exists())
        QDir().mkdir(INTERMIDATE_IMAGES_PATH);
#endif
}

#ifdef STORE_INTERMIDATE_IMAGES
inline static void store_converted_image_w(const QVector< uchar > &data, const char *name, int w, int h) {
    QImage image(data.data(), w, h, QImage::Format_Indexed8);
    STORE_DEBUG_IMAGE(&image, name);
}

inline static void store_converted_image(const QVector< uchar > &data, const char *name) {
    float wf = qSqrt(data.size());
    if ((float)qFloor(wf) != qSqrt(data.size())) {
        qDebug() << "Input vector is not square";
        return;
    }
    return store_converted_image_w(data, name, (int)wf, (int)wf);
}
#else
#   define store_converted_image_w(data, name, w, h)
#   define store_converted_image(data, name)
#endif

bool FaceRecognizer::recognize(const QImage &in_img, QImage &out_img, QString &name) {
    QVector< QRect > faces;
    name = "Unknown";

    if (in_img.isNull()) {
        qDebug() << "Null image given";
        return false;
    }

    out = &out_img;
    QImage gray_image;
    prepare_image(&in_img, &gray_image);

    if (in_img.format() == QImage::Format_Indexed8)
        out_img = in_img.convertToFormat(QImage::Format_ARGB32);

    recognize_face(DEFAULT_RECO_FACTOR, faces);
    qDebug() << faces;

#ifdef DRAW_RED_RECTS
    for (auto r = faces.begin(); r != faces.end(); r++)
        draw_rect(*r, out, Qt::red);
#endif
    if (faces.size()) {
        QImage face;
        choose_rect(faces, &face, &gray_image);

#if 0
        /* Debug filters saving */
        QVector< uchar > out_pixels;
        set_filter(&face, out_pixels, HOAR_FILTER);
        store_converted_image(out_pixels, "dobeshi_1");
        set_filter(&face, out_pixels, D4_FILTER);
        store_converted_image(out_pixels, "dobeshi_2");
        set_filter(&face, out_pixels, D6_FILTER);
        store_converted_image(out_pixels, "dobeshi_3");
        set_filter(&face, out_pixels, D10_FILTER);
        store_converted_image(out_pixels, "dobeshi_4");
#endif
        set_filter(&face, last_recognized_face, D6_FILTER);
        name = neuro_search(last_recognized_face);

        return true;
    }
    return false;
}

QString FaceRecognizer::neuro_search(const QVector< uchar > &data) {
    static float f_data[DEFAULT_FACE_RECT_SIZE * DEFAULT_FACE_RECT_SIZE];
    QString name = "Unknown";
    if (st_arr_len(f_data) != data.size()) {
        qDebug() << "Something happens wrong: Incorrect input array length";
        return name;
    }

    const uchar *i_data = data.data();
    for (size_t i = 0; i < st_arr_len(f_data); i++)
        f_data[i] = 1.0 * i_data[i];

    QPair< QString, float > max = qMakePair(name, -FLT_MAX);
    for (auto it = nets.begin(); it != nets.end(); it++) {
        float *out = fann_run(it.value().second, f_data);
        qDebug() << it.key() << out[0];
        if (max.second < *out)
            max = qMakePair(it.key(), *out);
    }
    return max.first;
}

/*
 * Only one face per image expected.
 * TODO
 */
QRect FaceRecognizer::choose_rect(const QVector< QRect > &recognized_faces, QImage *new_image, const QImage *gray_image) {
    if (!recognized_faces.size())
        return QRect();

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

    return last_rect;
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

    if (img->depth() == 8) /* Already grayscaled */
        memcpy(pixels.data(), img->bits(), w * h);
    else
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

inline static bool update_neuronet(fann *ann, const uchar *data, size_t count) {
    static float f_data[DEFAULT_FACE_RECT_SIZE * DEFAULT_FACE_RECT_SIZE];
    if (st_arr_len(f_data) != count) {
        qDebug() << "Something happens wrong: Incorrect input array length";
        return false;
    }

    for (size_t i = 0; i < count; i++)
        f_data[i] = 1.0 * data[i];

    float results[] = { 1.0 };
    fann_train(ann, f_data, results);
    return true;
}

void FaceRecognizer::update_name(const QString &new_name) {
    if (!last_recognized_face.size())
        return;

    fann *ann = NULL;
    if (!people.contains(new_name))
        ann = create_new_neuronet(new_name);
    else
        ann = nets[new_name].second;
    if (update_neuronet(ann, last_recognized_face.data(), last_recognized_face.size()))
        nets[new_name].first = true;
}
