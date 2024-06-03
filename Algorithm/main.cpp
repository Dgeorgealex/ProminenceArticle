#include <bits/stdc++.h>
#include "utils.h"
#include "tiles.h"
#include "area.h"
#include "forest_node.h"

using namespace std;

int tile_id;
int16_t tile[HGT_TILE_SIZE][HGT_TILE_SIZE];
int viz[HGT_TILE_SIZE][HGT_TILE_SIZE];

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
    for (int i = 0; i < HGT_TILE_SIZE; i++)
        for (int j = 0; j < HGT_TILE_SIZE; j++)
            if (viz[i][j] == -1 && tile[i][j] != -1)
                areas.push_back(fill(i, j, (int) areas.size()));
}

void connectEdges() {
    graph.resize(areas.size());
    for (auto &a: graph)
        a.clear();

    for (int i = 0; i < HGT_TILE_SIZE; i++)
        for (int j = 0; j < HGT_TILE_SIZE; j++)
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

Tile div_et_imp(int minLat, int maxLat, int minLon, int maxLon) {
    if (minLat == maxLat && minLon == maxLon) {
        tile_id++;
        cout << minLat << ' ' << minLon << ' ' << tile_id << '\n';
        readTileFromFile(tile, minLat, minLon);
        fillFlatAreas();
        connectEdges();
        sortEdgesInitializeTree();
        mainAlgorithm();
        writeCorrectOnesSimple(areas, minLat, minLon);
        int bad = countBadOnes(areas);
        int peak_count = countPeaks(areas);
        if(maximum_bad < bad){
            maximum_bad = bad;
            maximum_lat = minLat;
            maximum_lon = minLon;
        }
        total_bad += bad;
        exportInfo(minLat, minLon, peak_count, bad);
        cout << "Bad ones for tile = " << bad;
        cout << "\n-----------------------------------------------------\n\n";
        return {areas, edges, tile, viz, minLat, minLon};
    }

    int mijLat = (minLat + maxLat) / 2;
    int mijLon = (minLon + maxLon) / 2;

    if (minLat != maxLat && minLon != maxLon) {
        Tile t1 = div_et_imp(minLat, mijLat, minLon, mijLon);
        Tile t2 = div_et_imp(minLat, mijLat, mijLon + 1, maxLon);
        cout << "Uniting top from 4 areas:\n";
        cout << minLat << ' ' << mijLat << ' ' << minLon << ' ' << mijLon << '\n';
        cout << minLat << ' ' << mijLat << ' ' << mijLon + 1 << ' ' << maxLon << "\n\n";
        t1.uniteLeft(t2);

        Tile t3 = div_et_imp(mijLat + 1, maxLat, minLon, mijLon);
        Tile t4 = div_et_imp(mijLat + 1, maxLat, mijLon + 1, maxLon);
        cout << "Uniting bottom from 4 areas:\n";
        cout << mijLat + 1 << ' ' << maxLat << ' ' << minLon << ' ' << mijLon << '\n';
        cout << mijLat + 1 << ' ' << maxLat << ' ' << mijLon + 1 << ' ' << maxLon << "\n\n";
        t3.uniteLeft(t4);

        cout << "Uniting top to bottom from 4 areas:\n";
        cout << minLat << ' ' << maxLat << ' ' << minLon << ' ' << mijLon << '\n';
        cout << minLat << ' ' << maxLat << ' ' << mijLon + 1 << ' ' << maxLon << "\n\n";
        t1.uniteDown(t3);

        return t1;
    }

    if (minLat == maxLat) {
        Tile t1 = div_et_imp(minLat, maxLat, minLon, mijLon);
        Tile t2 = div_et_imp(minLat, maxLat, mijLon + 1, maxLon);
        cout << "Uniting left to right:\n";
        cout << minLat << ' ' << maxLat << ' ' << minLon << ' ' << mijLon << '\n';
        cout << minLat << ' ' << maxLat << ' ' << mijLon + 1 << ' ' << maxLon << "\n\n";
        t1.uniteLeft(t2);
        return t1;
    }

    Tile t1 = div_et_imp(minLat, mijLat, minLon, maxLon);
    Tile t2 = div_et_imp(mijLat + 1, maxLat, minLon, maxLon);
    cout << "Uniting up do down:\n";
    cout << minLat << ' ' << mijLat << ' ' << minLon << ' ' << maxLon << '\n';
    cout << mijLat + 1 << ' ' << maxLat << ' ' << minLon << ' ' << maxLon << "\n\n";
    t1.uniteDown(t2);
    return t1;
}

int main() {
    auto start = chrono::steady_clock::now();

    Tile last = div_et_imp(MIN_LAT, MAX_LAT, MIN_LON, MAX_LON);
    final_bad_ones = countBadOnes(last.areas);
    last.printAllPeaks();

    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start);
    cout << "Elapsed time: " << elapsed.count() << " seconds" << '\n';
    cout << "Maximum bad = " << maximum_bad << '\n';
    cout << "Maximum_len = " << maximum_lon << '\n';
    cout << "Maximum_lat = " << maximum_lat << '\n';
    cout << "Total number of peaks = " << total_peaks << '\n';
    cout << "Total number of bad = " << total_bad << '\n';
    cout << "Final bad peaks = " << final_bad_ones << '\n';
    return 0;
}
