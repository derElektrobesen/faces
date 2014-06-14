 #include "haarcascade.h"

HaarCascade::HaarCascade(QObject *parent) :
    QObject(parent),
    features(NULL),
    stages(NULL),
    nodes(NULL),
    rects(NULL),
    n_stages(0)
{}

HaarCascade::~HaarCascade() {
    if (features) delete[] features;
    if (stages) delete[] stages;
    if (nodes) delete[] nodes;
    if (rects) delete[] rects;
}

bool HaarCascade::init_memory() {
    DECLARE_SQL_CON(q);
    int count;

#define READ_COUNT(__table__) \
        if (!q.exec("select count(*) from " __table__)) \
            return false; \
        q.next(); \
        count = q.value(0).toInt(); \
        q.finish(); \
        if (!count) return false;

    READ_COUNT("features")
    features = new int [count];
    bzero(features, sizeof(*features) * count);

    READ_COUNT("stages")
    stages = new stage [count];
    n_stages = count;
    bzero(stages, sizeof(*stages) * count);

    READ_COUNT("nodes")
    nodes = new node [count];
    bzero(nodes, sizeof(*nodes) * count);

    READ_COUNT("rects")
    rects = new rect [count];
    bzero(rects, sizeof(*rects) * count);

#undef READ_COUNT

    return true;
}

bool HaarCascade::load_data() {
    DECLARE_SQL_CON(q);
    int index = 1;
    QMap< int, int > l_features;
    QMap< int, int > l_stages;
    QMap< int, int > l_nodes;

#define EXEC(query) \
        if (!index) return false; /* No data loaded */ \
        index = 0; \
        if (!q.exec(query)) return false;

    EXEC("select id, tilted from features")
    while (q.next()) {
        l_features.insert(q.value(0).toInt(), index);
        features[index++] = q.value(1).toInt();
    }
    q.finish();

    EXEC("select width, height from sizes")
    q.next();   /* TODO: Convert to multiple sizes usage */
    window_min_w = q.value(0).toInt();
    window_min_h = q.value(1).toInt();
    index = 1;
    q.finish();

    EXEC("select id, threshold, parent_stage from stages order by id")
    while (q.next()) {
        l_stages.insert(q.value(0).toInt(), index);

        int parent = q.value(2).toInt();
        if (parent != -1)
            stages[index].parent = stages + l_stages[parent];
        else
            stages[index].parent = NULL;

        stages[index].nodes_count = 0;
        stages[index].nodes_ptr = NULL;

        stages[index++].threshold = q.value(1).toDouble();
    }
    q.finish();

    EXEC("select feature_id, stage_id, threshold, left_val, right_val from nodes order by id");
    while (q.next()) {
        stage *stage_ptr = stages + l_stages[q.value(1).toInt()];

        nodes[index].tilted = features[ l_features[q.value(0).toInt()] ];
        nodes[index].threshold = q.value(2).toDouble();
        nodes[index].left_val = q.value(3).toDouble();
        nodes[index].right_val = q.value(4).toDouble();

        nodes[index].rects_ptr = NULL;
        nodes[index].rects_count = 0;

        if (!stage_ptr->nodes_ptr)
            stage_ptr->nodes_ptr = nodes + index;
        stage_ptr->nodes_count++;

        l_nodes.insert(q.value(0).toInt(), index);

        index++;
    }
    q.finish();

    EXEC("select feature_id, x, y, width, height, weight from rects");
    while (q.next()) {
        node *node_ptr = nodes + l_nodes[q.value(0).toInt()];
        rects[index].x = q.value(1).toInt();
        rects[index].y = q.value(2).toInt();
        rects[index].width = q.value(3).toInt();
        rects[index].height = q.value(4).toInt();
        rects[index].weight = q.value(5).toFloat();

        if (!node_ptr->rects_ptr)
            node_ptr->rects_ptr = rects + index;
        node_ptr->rects_count++;

        index++;
    }
    q.finish();

#undef EXEC
    return true;
}

bool HaarCascade::load() {
    if (!init_memory())
        return false;
    return load_data();
}
