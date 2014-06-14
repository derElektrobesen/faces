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

public:
    struct rect {
        int x;
        int y;
        int width;
        int height;
        float weight;
    };

    struct node {
        int tilted;
        double threshold;
        double left_val;
        double right_val;

        int rects_count;
        rect *rects_ptr;
    };

    struct stage {
        double threshold;
        stage *parent;

        int nodes_count;
        node *nodes_ptr;
    };

public:
    explicit HaarCascade(QObject *parent = 0);
    ~HaarCascade();

    bool load();

    inline const stage *get_stages(int *count) const { *count = n_stages; return this->stages; }
    inline void get_window(int *min_w, int *min_h) const { *min_w = window_min_w, *min_h = window_min_h; }

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

    int n_stages;
};

#endif // HAARCASCADE_H
