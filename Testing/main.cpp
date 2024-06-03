#include <bits/stdc++.h>
#include "utils.h"
#include "area.h"
#include "forest_node.h"

using namespace std;

int16_t small_tile[HGT_TILE_SIZE][HGT_TILE_SIZE];
int16_t tile[HGT_TILE_SIZE * NR_TILES_LIN][HGT_TILE_SIZE * NR_TILES_COL];
int viz[HGT_TILE_SIZE * NR_TILES_LIN][HGT_TILE_SIZE * NR_TILES_COL];

vector<Area> areas;
vector<vector<int>> graph;
vector<int> order[H_MAX];
vector<ForestNode> forest;
vector<pair<pair<int, int>, int>> edges;

Area fill(int i, int j, int id) {
    bool is_edge = false;
    bool is_peak = true;

    queue<pair<int, int>> q;
    q.emplace(i, j);
    viz[i][j] = id;

    while (!q.empty()) {
        auto u = q.front();

        is_edge = is_edge || isOnEdge(u.first, u.second);

        q.pop();

        for (int d = 0; d < 8; d++) {
            pair<int, int> v;
            v.first = u.first + dx[d];
            v.second = u.second + dy[d];


            if (!isInside(v.first, v.second))
                continue;

            if (tile[v.first][v.second] > tile[u.first][u.second]) {
                is_peak = false;
                continue;
            }

            if (viz[v.first][v.second] == -1 && tile[v.first][v.second] == tile[u.first][u.second]) {
                viz[v.first][v.second] = id;
                q.push(v);
            }
        }
    }

    return {id, make_pair(i, j), tile[i][j], is_edge, is_peak};
}

void fillFlatAreas() {
    areas.clear();
    memset(viz, -1, sizeof(viz));
    for (int i = 0; i < NR_TILES_LIN * HGT_TILE_SIZE; i++)
        for (int j = 0; j < NR_TILES_COL * HGT_TILE_SIZE; j++)
            if (viz[i][j] == -1 && tile[i][j] != -1)
                areas.push_back(fill(i, j, (int) areas.size()));
}

void connectEdges() {
    graph.resize(areas.size());
    for (auto &a: graph)
        a.clear();

    for (int i = 0; i < NR_TILES_LIN * HGT_TILE_SIZE; i++)
        for (int j = 0; j < NR_TILES_COL * HGT_TILE_SIZE; j++)
            for (int d = 0; d < 8; d++) {
                int x = i + dx[d], y = j + dy[d];
                if (!isInside(x, y))
                    continue;

                if (viz[x][y] != viz[i][j])
                    graph[viz[i][j]].push_back(viz[x][y]);
            }
}

void sortEdgesInitializeTree() {
    for (auto &h: order)
        h.clear();

    for (auto it: areas)
        order[it.height].push_back(it.id);

    forest.resize(areas.size());
    for (auto it: areas)
        forest[it.id] = ForestNode(it);
}

void updateSaddle(int id, int h, bool true_saddle) {
    if (id == -1 || !areas[id].is_peak)
        return;

    if (areas[id].saddle_height == -1) {
        areas[id].saddle_height = h;
        areas[id].true_saddle = true_saddle;
    }
}

int uniteSameKind(int a, int b, int h, bool type) {
    int ta = ForestNode::findParent(a, forest), tb = ForestNode::findParent(b, forest);

    if (ta == tb)
        return ta;

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
        edges.push_back({{forest[ta].highest_id, forest[tb].highest_id}, h});
    }
    return ForestNode::setUnion(ta, tb, forest);
}

int uniteDifferent(int a, int b, int h) {
    int ta = ForestNode::findParent(a, forest), tb = ForestNode::findParent(b, forest);

    if (ta == tb)
        return ta;

    if (forest[ta].height < forest[tb].height)
        updateSaddle(forest[ta].highest_id, h, true);
    else if (forest[ta].height == forest[tb].height) {
        updateSaddle(forest[ta].highest_id, h, true);
        updateSaddle(forest[tb].highest_id, h, false);
    } else {
        updateSaddle(forest[tb].highest_id, h, false);
        areas[forest[ta].highest_id].true_saddle = false;
        areas[forest[tb].highest_id].true_saddle = false;
        edges.push_back({{forest[ta].highest_id, forest[tb].highest_id}, h});
    }
    return ForestNode::setUnion(ta, tb, forest);
}


void mainAlgorithm() {
    edges.clear();
    for (int h = H_MAX - 1; h >= 0; h--) {
        if (order[h].empty())
            continue;

        for (auto area: order[h]) {
            stack<int> inside, outside;
            for (auto it: graph[area]) {
                if (areas[it].height < h)
                    continue;

                int p_it = ForestNode::findParent(it, forest);

                if (forest[p_it].is_edge)
                    outside.push(p_it);
                else
                    inside.push(p_it);
            }
            int p_area = ForestNode::findParent(area, forest);
            if (forest[p_area].is_edge)
                outside.push(p_area);
            else
                inside.push(p_area);

            while (inside.size() > 1) {
                int a = inside.top();
                inside.pop();
                int b = inside.top();
                inside.pop();
                int aux = uniteSameKind(a, b, h, true);
                inside.push(aux);
            }

            while (outside.size() > 1) {
                int a = outside.top();
                outside.pop();
                int b = outside.top();
                outside.pop();
                int aux = uniteSameKind(a, b, h, false);
                outside.push(aux);
            }

            if (!outside.empty() && !inside.empty()) {
                int a = inside.top();
                int b = outside.top();
                uniteDifferent(a, b, h);
            }
        }
    }
}

int main() {
    for (int i = 0; i < NR_TILES_LIN; i++)
        for (int j = 0; j < NR_TILES_COL; j++) {
            readTileFromFile(small_tile, MIN_LAT + i, MIN_LON + j);

            for (int ii = 0; ii < HGT_TILE_SIZE; ii++)
                for (int jj = 0; jj < HGT_TILE_SIZE; jj++)
                    tile[i * HGT_TILE_SIZE + ii][j * HGT_TILE_SIZE + jj] = small_tile[ii][jj];
        }

    fillFlatAreas();
    connectEdges();
    sortEdgesInitializeTree();
    mainAlgorithm();

    vector<pair<pair<double, double>, int>> true_results, false_results;
    for (auto area: areas)
        if (area.is_peak) {
            int prom = 0;
            if (area.saddle_height == -1)
                prom = area.height;
            else
                prom = area.height - area.saddle_height;

            auto [i, j] = area.representative;

            int lat = i / HGT_TILE_SIZE, lon = j / HGT_TILE_SIZE, r_lat = i % HGT_TILE_SIZE, r_lon = j % HGT_TILE_SIZE;

            double x = MIN_LAT + lat + 1 - (double) r_lat / HGT_TILE_SIZE;
            double y = MIN_LON + lon + (double) r_lon / HGT_TILE_SIZE;

            true_results.push_back({{x, y}, prom});
        }

    ifstream fin(check_file.c_str());

    double x, y;
    int prom;
    while (fin >> x >> y >> prom) {
        false_results.push_back({{x, y}, prom});
    }

    if (true_results.size() != false_results.size()) {
        cout << "Not same size";
        exit(-1);
    }


    sort(true_results.begin(), true_results.end());
    sort(false_results.begin(), false_results.end());

    int cnt = 0;
    for(int i=0;i<true_results.size(); i++)
        if(!compare_maps(true_results[i].first, false_results[i].first) ||
           true_results[i].second != false_results[i].second)
            cnt++;

    cout << "\n\nBase algorithm DONE\n\n";

    if(cnt != 0)
        cout << "Something was wrong";
    else
        cout << "All is good";
    return 0;
}