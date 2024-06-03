#ifndef ALGORITHM_TILES_H
#define ALGORITHM_TILES_H

#include <bits/stdc++.h>
#include "area.h"
#include "utils.h"
#include "forest_node.h"
#include "merge_tiles_utils.h"

using namespace std;

struct Tile {
    vector<Area> areas;
    vector<pair<int, int>> up, down, left, right;
    vector<pair<pair<int, int>, int>> edges;

    Tile() = default;

    Tile(const vector<Area> &tile_areas, const vector<pair<pair<int, int>, int>> &tile_edges, int16_t (*tile)[1201],
         int (*viz)[1201], int lat, int lon);

    [[maybe_unused]] void printInfo();

    [[maybe_unused]] void exportInfo();

    void uniteLeft(Tile &otr);

    void uniteDown(Tile &otr);

    void runAlgorithm();

    void printAllPeaks();

    void uniteSameKind(int a, int b, int h, vector<ForestNode> &forest, bool type,
                       vector<pair<pair<int, int>, int>> &new_edge);

    void
    uniteDifferentKind(int a, int b, int h, vector<ForestNode> &forest, vector<pair<pair<int, int>, int>> &new_edge);

    void updateSaddle(int id, int h, bool true_saddle);

    void repairPrintTruePeaks(vector<pair<pair<int, int>, int>> &new_edges);
};

Tile::Tile(const vector<Area> &tile_areas, const vector<pair<pair<int, int>, int>> &tile_edges, int16_t (*tile)[1201],
           int (*viz)[1201], int lat, int lon) {

    unordered_map<int, int> transform;
    up.resize(HGT_TILE_SIZE);
    down.resize(HGT_TILE_SIZE);
    right.resize(HGT_TILE_SIZE);
    left.resize(HGT_TILE_SIZE);

    int cnt = 0;
    for (auto area: tile_areas)
        if (area.is_edge || (area.is_peak && !area.true_saddle)) {
            transform[area.id] = cnt;
            Area newArea(area);
            newArea.id = cnt;
            newArea.lat = lat;
            newArea.lon = lon;
            areas.push_back(newArea);
            cnt++;
        }

    for (int i = 0; i < HGT_TILE_SIZE; i++) {
        up[i] = make_pair(tile[0][i], transform[viz[0][i]]);
        down[i] = make_pair(tile[HGT_TILE_SIZE - 1][i], transform[viz[HGT_TILE_SIZE - 1][i]]);
        left[i] = make_pair(tile[i][0], transform[viz[i][0]]);
        right[i] = make_pair(tile[i][HGT_TILE_SIZE - 1], transform[viz[i][HGT_TILE_SIZE - 1]]);
    }
    unordered_set<int> visited;
    for (auto e: tile_edges) {
        assert(transform.count(e.first.first) && transform.count(e.first.second));
        edges.push_back({{transform[e.first.first], transform[e.first.second]}, e.second});
        visited.insert(e.first.first);
        visited.insert(e.first.second);

        auto ee = edges.back();
        assert(areas[ee.first.first].height >= ee.second && areas[ee.first.second].height >= ee.second);
    }
}

void Tile::uniteLeft(Tile &otr) {
    /// we are the assumption of seamless
    for (int i = 0; i < right.size(); i++)
        assert(right[i].first == otr.left[i].first);

    unordered_set<int> m, m_otr;
    unordered_map<int, int> t, t_otr;

    vector<Area> new_areas;
    for (int i = 0; i < right.size(); i++) {
        m.insert(right[i].second);
        m_otr.insert(otr.left[i].second);
    }

    addNewAreasOutsideUnion(areas, new_areas, m, t);
    addNewAreasOutsideUnion(otr.areas, new_areas, m_otr, t_otr);

    int new_ids = uniteSides(right, otr.left, t, t_otr, (int) new_areas.size());
    for (int i = 0; i < new_ids; i++) {
        new_areas.emplace_back();
        new_areas.back().id = -1;
    }

    addNewAreasInsideUnion(right, areas, new_areas, t);
    addNewAreasInsideUnion(otr.left, otr.areas, new_areas, t_otr);

    areas = new_areas;
    normVector(up, t, areas);
    normVector(down, t, areas);
    normVector(left, t, areas);
    normVector(otr.up, t_otr, areas);
    normVector(otr.down, t_otr, areas);
    normVector(otr.right, t_otr, areas);

    for (auto &e: edges) {
        e.first.first = t[e.first.first];
        e.first.second = t[e.first.second];
        assert(areas[e.first.first].height >= e.second && areas[e.first.second].height >= e.second);
    }
    for (auto &e: otr.edges) {
        e.first.first = t_otr[e.first.first];
        e.first.second = t_otr[e.first.second];
        assert(areas[e.first.first].height >= e.second && areas[e.first.second].height >= e.second);
    }

    edges.insert(edges.end(), otr.edges.begin(), otr.edges.end());
    up.insert(up.end(), otr.up.begin(), otr.up.end());
    down.insert(down.end(), otr.down.begin(), otr.down.end());
    right = otr.right;

    otr.areas.clear();
    otr.up.clear();
    otr.down.clear();
    otr.left.clear();
    otr.right.clear();
    otr.edges.clear();

    assert(check_if_tree((int) areas.size(), edges));
    runAlgorithm();
}

void Tile::uniteDown(Tile &otr) {
    /// we are the assumption of seamless
    for (int i = 0; i < down.size(); i++)
        assert(down[i].first == otr.up[i].first);

    unordered_set<int> m, m_otr;
    unordered_map<int, int> t, t_otr;

    vector<Area> new_areas;
    for (int i = 0; i < down.size(); i++) {
        m.insert(down[i].second);
        m_otr.insert(otr.up[i].second);
    }

    addNewAreasOutsideUnion(areas, new_areas, m, t);
    addNewAreasOutsideUnion(otr.areas, new_areas, m_otr, t_otr);

    int new_ids = uniteSides(down, otr.up, t, t_otr, (int) new_areas.size());
    for (int i = 0; i < new_ids; i++) {
        new_areas.emplace_back();
        new_areas.back().id = -1;
    }

    addNewAreasInsideUnion(down, areas, new_areas, t);
    addNewAreasInsideUnion(otr.up, otr.areas, new_areas, t_otr);

    areas = new_areas;
    normVector(left, t, areas);
    normVector(up, t, areas);
    normVector(right, t, areas);
    normVector(otr.left, t_otr, areas);
    normVector(otr.down, t_otr, areas);
    normVector(otr.right, t_otr, areas);

    for (auto &e: edges) {
        e.first.first = t[e.first.first];
        e.first.second = t[e.first.second];
        assert(areas[e.first.first].height >= e.second && areas[e.first.second].height >= e.second);
    }
    for (auto &e: otr.edges) {
        e.first.first = t_otr[e.first.first];
        e.first.second = t_otr[e.first.second];
        assert(areas[e.first.first].height >= e.second && areas[e.first.second].height >= e.second);
    }

    edges.insert(edges.end(), otr.edges.begin(), otr.edges.end());
    left.insert(left.end(), otr.left.begin(), otr.left.end());
    right.insert(right.end(), otr.right.begin(), otr.right.end());
    down = otr.down;

    otr.areas.clear();
    otr.up.clear();
    otr.down.clear();
    otr.left.clear();
    otr.right.clear();
    otr.edges.clear();

    assert(check_if_tree((int) areas.size(), edges));
    runAlgorithm();
}

void Tile::runAlgorithm() {
    vector<ForestNode> f((int) areas.size());
    vector<vector<pair<int, int>>> o(H_MAX);
    vector<pair<pair<int, int>, int>> new_edges;
    for (auto e: edges)
        o[e.second].push_back(e.first);

    int cnt = 0;
    for (auto it: areas) {
        if (it.id != cnt)
            exit(-1);
        cnt++;
        f[it.id] = ForestNode(it);
    }

    for (int h = H_MAX - 1; h >= 0; h--) {
        if (o[h].empty())
            continue;

        /// inside
        for (auto e: o[h]) {
            int ta = ForestNode::findParent(e.first, f), tb = ForestNode::findParent(e.second, f);
            if (!f[ta].is_edge && !f[tb].is_edge)
                uniteSameKind(ta, tb, h, f, true, new_edges);

            else if (f[ta].is_edge && f[tb].is_edge)
                uniteSameKind(ta, tb, h, f, false, new_edges);
        }

        /// mixed
        for (auto e: o[h]) {
            int ta = ForestNode::findParent(e.first, f), tb = ForestNode::findParent(e.second, f);
            if (!f[ta].is_edge && f[tb].is_edge)
                uniteDifferentKind(ta, tb, h, f, new_edges);
            else if (f[ta].is_edge && !f[tb].is_edge)
                uniteDifferentKind(tb, ta, h, f, new_edges);
        }

        /// same again
        for (auto e: o[h]) {
            int ta = ForestNode::findParent(e.first, f), tb = ForestNode::findParent(e.second, f);
            if (f[ta].is_edge && f[tb].is_edge)
                uniteSameKind(ta, tb, h, f, false, new_edges);
        }
    }

    int tp = ForestNode::findParent(0, f);
    for (auto area: areas)
        assert(tp == ForestNode::findParent(area.id, f));

    repairPrintTruePeaks(new_edges);
}

void Tile::updateSaddle(int id, int h, bool true_saddle) {
    if (id == -1 || !areas[id].is_peak)
        return;

    areas[id].true_saddle = true_saddle;
    areas[id].saddle_height = max(areas[id].saddle_height, h);
}

void Tile::uniteSameKind(int ta, int tb, int h, vector<ForestNode> &forest, bool type,
                         vector<pair<pair<int, int>, int>> &new_edge) {
    if (ta == tb)
        return;

    if (forest[ta].height == forest[tb].height) {
        updateSaddle(forest[ta].highest_id, h, type);
        updateSaddle(forest[tb].highest_id, h, type);
    } else {
        if (forest[ta].height > forest[tb].height)
            swap(ta, tb);

        updateSaddle(forest[ta].highest_id, h, type);
    }

    if (!type) {
        areas[forest[ta].highest_id].true_saddle = false;
        areas[forest[tb].highest_id].true_saddle = false;
        new_edge.push_back({{forest[ta].highest_id, forest[tb].highest_id}, h});
    }
    ForestNode::setUnion(ta, tb, forest);
}

void Tile::uniteDifferentKind(int ta, int tb, int h, vector<ForestNode> &forest,
                              vector<pair<pair<int, int>, int>> &new_edges) {
    if (ta == tb)
        return;

    if (forest[ta].height < forest[tb].height)
        updateSaddle(forest[ta].highest_id, h, true);
    else if (forest[ta].height == forest[tb].height) {
        updateSaddle(forest[ta].highest_id, h, true);
        updateSaddle(forest[tb].highest_id, h, false);
    } else {
        updateSaddle(forest[tb].highest_id, h, false);
        areas[forest[ta].highest_id].true_saddle = false;
        areas[forest[tb].highest_id].true_saddle = false;
        new_edges.push_back({{forest[ta].highest_id, forest[tb].highest_id}, h});
    }

    ForestNode::setUnion(ta, tb, forest);
}


void Tile::repairPrintTruePeaks(vector<pair<pair<int, int>, int>> &new_edges) {
    for (auto area: areas)
        if (area.is_peak && area.true_saddle)
            writeAnswerForArea(area, false);

    areas.erase(remove_if(areas.begin(), areas.end(), [&](Area area) { return !(area.is_edge || (area.is_peak && !area.true_saddle)); }), areas.end());

    unordered_map<int, int> transform;
    int cnt = 0;
    for (auto &area: areas) {
        transform[area.id] = cnt;
        area.id = cnt;
        cnt++;
    }

    for (auto &e: new_edges) {
        e.first.first = transform[e.first.first];
        e.first.second = transform[e.first.second];
    }

    edges = new_edges;

    for (auto &it: up) it.second = transform[it.second];
    for (auto &it: down) it.second = transform[it.second];
    for (auto &it: left) it.second = transform[it.second];
    for (auto &it: right) it.second = transform[it.second];

    assert(check_if_tree(areas.size(), edges));
}

void Tile::printAllPeaks() {
    for (auto area: areas)
        if (area.is_peak)
            writeAnswerForArea(area, true);
}

[[maybe_unused]] void Tile::printInfo() {
    cout << "Info for tile:\n";
    cout << areas.size() << '\n';
    cout << up.size() << '\n';
    cout << down.size() << '\n';
    cout << left.size() << '\n';
    cout << right.size() << '\n';
    cout << edges.size() << '\n';
    cout << '\n';
}

#endif
