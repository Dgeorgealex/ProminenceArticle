#ifndef ALGORITHM_MERGE_TILES_UTILS_H
#define ALGORITHM_MERGE_TILES_UTILS_H
#define N_MAX 2000005

vector<int> v[N_MAX];
int group[N_MAX];

void dfs(int x, int k) {
    group[x] = k;
    for (auto it: v[x])
        if (group[it] == -1)
            dfs(it, k);
}

int uniteSides(const vector<pair<int, int>> &a, const vector<pair<int, int>> &b, unordered_map<int, int> &t,
               unordered_map<int, int> &t_otr, int id) {
    int cnt = 0;
    for (int i = 0; i < a.size(); i++) {
        if (t.count(a[i].second) == 0) {
            t[a[i].second] = cnt;
            cnt++;
        }
        if (t_otr.count(b[i].second) == 0) {
            t_otr[b[i].second] = cnt;
            cnt++;
        }
    }

    for (int i = 0; i < cnt; i++) {
        v[i].clear();
        group[i] = -1;
    }

    for (int i = 0; i < a.size(); i++) {
        int x = t[a[i].second], y = t_otr[b[i].second];
        v[x].push_back(y);
        v[y].push_back(x);
    }

    int k = 0;
    for (int i = 0; i < cnt; i++)
        if (group[i] == -1) {
            dfs(i, k);
            k++;
        }

    for (int i = 0; i < a.size(); i++) {
        int pa = t[a[i].second], pb = t_otr[b[i].second];
        if (pa < id)
            t[a[i].second] = id + group[pa];
        if (pb < id)
            t_otr[b[i].second] = id + group[pb];
    }

    return k;
}

void addNewAreasOutsideUnion(const vector<Area> &areas, vector<Area> &new_areas, const unordered_set<int> &m,
                             unordered_map<int, int> &t) {
    for (auto area: areas)
        if (!m.contains(area.id)) {
            Area new_area = Area(area);
            new_area.id = (int) new_areas.size();

            t[area.id] = new_area.id;

            new_areas.push_back(new_area);
        }
}

void addNewAreasInsideUnion(const vector<pair<int, int>> &v, const vector<Area> &areas, vector<Area> &new_areas,
                            const unordered_map<int, int> &t) {
    for (auto it: v) {
        int pos = t.find(it.second)->second;
        if (new_areas[pos].id == -1) {
            new_areas[pos] = areas[it.second];
            new_areas[pos].id = pos;
            new_areas[pos].is_edge = false;
        } else {
            ///change representative
            if (new_areas[pos].lat == areas[it.second].lat && new_areas[pos].lon == areas[it.second].lat) {
                auto new_r = new_areas[pos].representative;
                auto r = areas[it.second].representative;
                if (new_r.first > r.first || (new_r.first == r.first && new_r.second > r.second))
                    new_areas[pos].representative = areas[it.second].representative;
            } else if (new_areas[pos].lat == areas[it.second].lat) {
                auto new_r = new_areas[pos].representative;
                auto r = areas[it.second].representative;

                if (new_r.first > r.first) {
                    new_areas[pos].representative = areas[it.second].representative;
                    new_areas[pos].lon = areas[it.second].lon;
                } else if (new_areas[pos].lon > areas[it.second].lon) {
                    new_areas[pos].representative = areas[it.second].representative;
                    new_areas[pos].lon = areas[it.second].lon;
                }
            } else if (new_areas[pos].lon == areas[it.second].lon) {
                if (new_areas[pos].lat > areas[it.second].lat) {
                    new_areas[pos].representative = areas[it.second].representative;
                    new_areas[pos].lat = areas[it.second].lat;
                }
            } else {
                auto n_p = make_pair(new_areas[pos].lat, new_areas[pos].lon);
                auto p = make_pair(areas[it.second].lat, areas[it.second].lon);
                if (n_p > p) {
                    new_areas[pos].representative = areas[it.second].representative;
                    new_areas[pos].lat = areas[it.second].lat;
                    new_areas[pos].lon = areas[it.second].lon;
                }
            }

            new_areas[pos].saddle_height = max(new_areas[pos].saddle_height, areas[it.second].saddle_height);
            new_areas[pos].is_peak = new_areas[pos].is_peak && areas[it.second].is_peak;
        }
    }
}

void normVector(vector<pair<int, int>> &a, const unordered_map<int, int> &t, vector<Area> &areas) {
    for (auto &it: a) {
        it.second = t.find(it.second)->second;
        areas[it.second].is_edge = true;
    }
}

void dfs_tree(int x, vector<int> &viz, const vector<vector<int>> &e) {
    viz[x] = 1;
    for (auto it: e[x])
        if (viz[it] == 0)
            dfs_tree(it, viz, e);
}

bool check_if_tree(int n, const vector<pair<pair<int, int>, int>> &edges) {
    vector<int> viz(n);

    for (int i = 0; i < n; i++)
        viz[i] = 0;
    vector<vector<int>> e(n);
    for (auto it: edges) {
        auto [x, y] = it.first;
        e[x].push_back(y);
        e[y].push_back(x);
    }

    dfs_tree(1, viz, e);
    for (int i = 0; i < n; i++)
        if (viz[i] == 0)
            return false;

    return true;
}

#endif //ALGORITHM_MERGE_TILES_UTILS_H
