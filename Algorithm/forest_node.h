#ifndef ALGORITHM_FOREST_NODE_H
#define ALGORITHM_FOREST_NODE_H

#include <bits/stdc++.h>
#include "area.h"

using namespace std;

struct ForestNode {
    int parent;
    int sz;
    int height;
    int highest_id;
    bool is_edge;

    ForestNode() = default;

    explicit ForestNode(const Area &area) {
        this->parent = area.id;
        this->sz = 1;
        this->height = area.height;
        this->highest_id = area.id;
        this->is_edge = area.is_edge;
    }

    static int findParent(int x, vector<ForestNode> &forest);

    static int setUnion(int a, int b, vector<ForestNode> &forest);

    ~ForestNode() = default;
};

int ForestNode::setUnion(int a, int b, vector<ForestNode> &forest) {
    if (forest[a].sz < forest[b].sz)
        swap(a, b);

    forest[b].parent = a;
    forest[a].sz += forest[b].sz;
    if (forest[b].height > forest[a].height ||
        (forest[b].height == forest[a].height && !forest[a].is_edge && forest[b].is_edge)) {  //consistent id
        forest[a].highest_id = forest[b].highest_id;
        forest[a].height = forest[b].height;
    }

    forest[a].is_edge = forest[a].is_edge || forest[b].is_edge;
    return a;
}

int ForestNode::findParent(int x, vector<ForestNode> &forest) {
    if (forest[x].parent != x) forest[x].parent = findParent(forest[x].parent, forest);
    return forest[x].parent;
}

#endif