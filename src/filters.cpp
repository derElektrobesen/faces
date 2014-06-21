#include "facerecognizer.h"

QVector<double> pconv(const QVector<double> &data, const QVector<double> &cl,
                      const QVector<double> &ch, int delta = 0)
{
    QVector<double> out;

    for (int k = 0; k < data.length(); k += 2) {
        double sL = 0;
        double sH = 0;

        for (int i = 0; i < cl.length(); ++i) {
            int ind = (k + i - delta) % data.length();
            if (ind < 0)
                ind += data.length();

            sL += data[ind] * cl[i];
            sH += data[ind] * ch[i];
        }

        out.append(sL);
        out.append(sH);
    }

    return out;
}

QVector<double> get_high_coefs(const QVector<double> &cl)
{
    QVector<double> result;
    int sign = 0;

    for (int i = cl.length() - 1; i >= 0; i--) {
        int mult = (sign++ % 2) == 0 ? 1 : -1;
        result.push_back(cl[i] * mult);
    }

    return result;
}

QVector<double> get_inverted_coeffs(const QVector<double> &cl,
                                    const QVector<double> &ch, bool low)
{
    QVector<double> result;
    int dec = (low == true) ? 2 : 1;

    for (int k = 0; k < cl.length(); k += 2) {
        int ind = k - dec;
        if (ind < 0)
            ind += cl.length();

        result.push_back(cl[ind]);
        result.push_back(ch[ind]);
    }

    return result;
}

QVector<QVector<double> > dwt2(const QVector<QVector<double> >&image,
                               const QVector<double> &cl)
{
    QVector<QVector<double> > imageT(image);
    QVector<double> ch = get_high_coefs(cl);
    int width = image.size(), height = image.size();

    for (int i = 0; i < height; ++i)
        imageT[i] = pconv(imageT[i], cl, ch);

    for (int j = 0; j < width; ++j) {
        QVector<double> column;
        for (int i = 0; i < height; ++i)
            column.push_back(imageT[i][j]);

        column = pconv(column, cl, ch);

        for (int i = 0; i < height; ++i)
            imageT[i][j] = column[i];
    }

    QVector<QVector<double> > result(imageT);
#if 1
    for (int i = 0, k = 0; i < height; i += 2, k++)
        for (int j = 0, l = 0; j < width; j += 2, l++) {
            result[k][l] = imageT[i][j];
            result[k + height / 2][l] = imageT[i + 1][j];
            result[k][l + width / 2] = imageT[i][j + 1];
            result[k + height / 2][l + width / 2] = imageT[i + 1][j + 1];
        }
#else
    for (int i = 0, k = 0; i < height; i += 2, k++) {
        for (int j = 0, l = 0; j < width; j += 2, l++) {
            result[k][l] = image[i + 1][j]; // 1
//            result[k][l] = image[i][j]; // 2
//            result[k][l] = image[i][j + 1]; // 3
//            result[k][l] = image[i+1][j+1]; // 4

        }
        result[k].resize(width / 2);
    }
    result.resize(width / 2);
#endif

    return result;
}

QVector<QVector<double> > invert_dwt2(const QVector<QVector<double> >&data,
                                      const QVector<double> &cl)
{
    int width = data.size(), height = data.size();
    QVector<QVector<double> > imageT = data;
    for (int i = 0, k = 0; i < height; i += 2, k++)
        for (int j = 0, l = 0; j < width; j += 2, l++) {
            imageT[i][j] = data[k][l];
            imageT[i + 1][j] = data[k + height / 2][l];
            imageT[i][j + 1] = data[k][l + width / 2];
            imageT[i + 1][j + 1] = data[k + height / 2][l + width / 2];
        }

    auto ch = get_high_coefs(cl);
    auto i_cl = get_inverted_coeffs(cl, ch, true);
    auto i_ch = get_inverted_coeffs(cl, ch, false);
    QVector<QVector<double> > image(imageT);

    for (int j = 0; j < width; ++j) {
        QVector<double> column;
        for (int i = 0; i < height; ++i)
            column.push_back(image[i][j]);

        column = pconv(column, i_cl, i_ch, i_cl.length() - 2);

        for (int i = 0; i < height; ++i)
            image[i][j] = column[i];
    }

    for (int i = 0; i < height; ++i)
        image[i] = pconv(image[i], i_cl, i_ch, i_cl.length() - 2);

    return image;
}

void save_img(const QVector<double> &data, const char *name) {
    int w = qSqrt(data.size());
    QVector<QRgb> image;
    for (int i = 0; i < data.size(); ++i) {
        uchar r = data[i] * UCHAR_MAX;
        image.push_back(qRgb(r,r,r));
    }

    QImage img((uchar *)image.data(), w, w, QImage::Format_RGB32);

    img.save(name);
}

bool FaceRecognizer::set_filter(const QImage *img, QVector< uchar > &out, reco_filter_type_t f_type) {
    double sq_2 = qSqrt(2);

    double d2[] = { 1.0, 1.0 };
    double d4[] = { 0.6830127, 1.1830127, 0.3169873, -0.1830127 };
    double d6[] = { 0.47046721, 1.14111692, 0.650365, -0.19093442, -0.12083221, 0.0498175 };
    double d10[] = { 0.22641898, 0.85394354, 1.02432694, 0.19576696, -0.34265671,
                     -0.04560113, 0.10970265, -0.00882680, -0.01779187, 0.00471742793 };

    double *ptr = NULL;
    int count = 0;
    switch (f_type) {
    case HOAR_FILTER: ptr = d2; count = st_arr_len(d2); break;
    case D4_FILTER: ptr = d4; count = st_arr_len(d4); break;
    case D6_FILTER: ptr = d6; count = st_arr_len(d6); break;
    case D10_FILTER: ptr = d10; count = st_arr_len(d10); break;
    default:
        qDebug() << "Unimplemented filter type given";
        return false;
    }

    QVector<double> cl;
    cl.resize(count);
    double *data_ptr = cl.data();

    for (int i = 0; i < count; i++)
        *data_ptr++ = *ptr++ / sq_2;

    QVector< QVector< double > > img_data;
    int w = img->width();
    int h = img->height();

    for (int i = 0; i < h; i++) {
        QVector< double > tmp;
        for (int j = 0; j < w; j++)
            tmp.push_back((img->pixel(j, i) & 0xFF) * 1.0 / UCHAR_MAX);
        img_data.push_back(tmp);
    }

    auto res = dwt2(img_data, cl);
    out.clear();
    out.reserve(res.length() * res.length());

    for (int i = 0; i < res.length(); ++i)
        for (int j = 0; j < res[i].length(); ++j)
            out.push_back(static_cast< uchar >(res[i][j] * UCHAR_MAX));
    return true;
}
