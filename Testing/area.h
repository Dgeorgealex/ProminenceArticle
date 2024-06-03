#ifndef TESTING_AREA_H
#define TESTING_AREA_H

#include <bits/stdc++.h>
using namespace std;

struct Area {
    int id;
    pair<int, int> representative;
    int height;
    bool is_edge, is_peak;
    int saddle_height;
    bool true_saddle;
    int lat, lon;

    Area() = default;

    Area(int id, pair<int, int> rep, int16_t tile_h, bool is_edge, bool is_peak){
        this->id = id;
        this->representative = rep;
        this->height = (int) tile_h;
        this->is_edge = is_edge;
        this->is_peak = is_peak;
        this->saddle_height = -1;
        this->true_saddle = false;
        this->lat = 0;
        this->lon = 0;
    }
};

#endif //TESTING_AREA_H
