#ifndef HAARCASCADE_H
#define HAARCASCADE_H

#include <QObject>
#include "databaseengine.h"
#include "main.h"

/**
 * \brief The HaarCascade class
 *
 * Class will download from database Haarcascade tables and will
 * store them. Only one object of this class must be created.
 */
class HaarCascade : public QObject
{
    Q_OBJECT

    struct stage {
        double threshold;
        stage *parent;
    };

    struct node {
        int feature;
        stage *node_stage;
        double threshold;
        double left_val;
        double right_val;
    };

    struct rect {
        int feature;
        int x;
        int y;
        int width;
        int height;
        float weight;
    };

public:
    explicit HaarCascade(QObject *parent = 0);
    ~HaarCascade();

    bool load();

signals:

public slots:

protected:
    bool init_memory();
    bool load_data();

private:
    int window_min_w, window_min_h; /**< Contains a start window size.                      */

    int *features;                  /**< Contains a `tilted` column of `features` table.    */
    stage *stages;
    node *nodes;
    rect *rects;
};

#endif // HAARCASCADE_H
