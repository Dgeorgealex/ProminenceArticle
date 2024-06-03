#ifndef TESTING_UTILS_H
#define TESTING_UTILS_H

#include <bits/stdc++.h>
using namespace std;


const int HGT_TILE_SIZE = 1201;
const int H_MAX = 9000;
const int MIN_LAT = 24;
const int MIN_LON = 72;
const int NR_TILES_LIN = 5;
const int NR_TILES_COL = 5;
const string data_directory = ".";
const string check_file = ".";
const double EPS = 1e-4;

int16_t in_buf[HGT_TILE_SIZE * HGT_TILE_SIZE];

const int dx[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

bool isOnEdge(int x, int y) {
    return x == 0 || y == 0 || x == NR_TILES_LIN * HGT_TILE_SIZE - 1 || y == NR_TILES_COL * HGT_TILE_SIZE - 1;
}

bool isInside(int x, int y) {
    return x >= 0 && y >= 0 && x < NR_TILES_LIN * HGT_TILE_SIZE && y < NR_TILES_COL * HGT_TILE_SIZE;
}

uint16_t swapByteOrderLittleEndian(uint16_t x) {
    return (x >> 8) | (x << 8);
}


string getFileName(int lat, int lon) {
    char buf[100];
    string filename = data_directory + "%c%02d%c%03d.hgt";
    snprintf(buf, sizeof(buf), filename.c_str(),
             (lat >= 0) ? 'N' : 'S',
             abs(static_cast<int>(lat)),
             (lon >= 0) ? 'E' : 'W',
             abs(static_cast<int>(lon)));
    filename = buf;

    return filename;
}


bool readTileFromFile(int16_t tile[][HGT_TILE_SIZE], int lat, int lon) {
    string data_path = getFileName(lat, lon);
    FILE *input_file = fopen(data_path.c_str(), "rb");
    if (input_file == nullptr)
        exit(-1);

    int num_elevation = HGT_TILE_SIZE * HGT_TILE_SIZE;

    fread(in_buf, sizeof(int16_t), num_elevation, input_file);

    int maximum = 0, missing = 0;
    for (int i = 0; i < HGT_TILE_SIZE; i++)
        for (int j = 0; j < HGT_TILE_SIZE; j++)
            tile[i][j] = (int16_t) swapByteOrderLittleEndian(in_buf[i * HGT_TILE_SIZE + j]);
    string down_data_path = getFileName(lat + 1, lon);
    string right_data_path = getFileName(lat, lon + 1);
    FILE *down_input_file = fopen(down_data_path.c_str(), "rb");
    FILE *right_input_file = fopen(right_data_path.c_str(), "rb");

    if (down_input_file != nullptr) {
        fread(in_buf, sizeof(int16_t), num_elevation, down_input_file);
        for (int j = 0; j < HGT_TILE_SIZE; j++)
            tile[HGT_TILE_SIZE - 1][j] = (int16_t) swapByteOrderLittleEndian(in_buf[j]);
    }

    if (right_input_file != nullptr) {
        fread(in_buf, sizeof(int16_t), num_elevation, right_input_file);
        for (int i = 0; i < HGT_TILE_SIZE; i++)
            tile[i][HGT_TILE_SIZE - 1] = (int16_t) swapByteOrderLittleEndian(in_buf[i * HGT_TILE_SIZE]);
    }


    tile[0][0] = tile[HGT_TILE_SIZE - 1][HGT_TILE_SIZE - 1] = tile[0][HGT_TILE_SIZE - 1] = tile[HGT_TILE_SIZE -
                                                                                                1][0] = -1;
    for (int i = 0; i < HGT_TILE_SIZE; i++)
        for (int j = 0; j < HGT_TILE_SIZE; j++) {
            if (tile[i][j] < 0) {
                missing++;
                tile[i][j] = 0;
            }
            maximum = max(maximum, (int) tile[i][j]);
        }

    cout << data_path << '\n';
    cout << maximum << '\n';
    cout << missing << "\n";

    fclose(input_file);
    return true;
}


bool compare_doubles(double a, double b){
    return fabs(a - b) <= EPS;
}
bool compare_maps(pair<double, double> a, pair<double, double> b){
    return compare_doubles(a.first, b.first) && compare_doubles(a.second, b.second);
}

#endif //TESTING_UTILS_H
