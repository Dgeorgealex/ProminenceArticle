#ifndef ALGORITHM_UTILS_H
#define ALGORITHM_UTILS_H

#include <bits/stdc++.h>
#include "area.h"

using namespace std;

const int HGT_TILE_SIZE = 1201;
const int MIN_LAT = 24;
const int MAX_LAT = 39;
const int MIN_LON = 72;
const int MAX_LON = 107;
const string data_directory = ".";
const string answer_directory = ".";
const string info_file = ".";
[[maybe_unused]] const string check_file = ".";
const int H_MAX = 8850;

int maximum_bad, maximum_lat, maximum_lon, final_bad_ones;
long long total_bad;
long long total_peaks;

int16_t in_buf[HGT_TILE_SIZE * HGT_TILE_SIZE];

const int dx[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

bool isOnEdge(int x, int y) {
    return x == 0 || y == 0 || x == HGT_TILE_SIZE - 1 || y == HGT_TILE_SIZE - 1;
}

bool isInside(int x, int y) {
    return x >= 0 && y >= 0 && x < HGT_TILE_SIZE && y < HGT_TILE_SIZE;
}

string getOutputFileName(int lat, int lon) {
    char buf[100];
    string filename = answer_directory + "%c%02d%c%03d.txt";
    snprintf(buf, sizeof(buf), filename.c_str(),
             (lat >= 0) ? 'N' : 'S',
             abs(static_cast<int>(lat)),
             (lon >= 0) ? 'E' : 'W',
             abs(static_cast<int>(lon)));
    filename = buf;
    return filename;
    // return check_file; // - this is for check purposes
}

void writeCorrectOnesSimple(const vector<Area> &areas, int lat, int lon) {
    string filename = getOutputFileName(lat, lon);
    FILE *output_file = fopen(filename.c_str(), "w");
    // FILE *output_file = fopen(filename.c_str(), "a"); // - this is for check purposes

    for (auto area: areas) {
        if (area.is_peak && area.is_edge && area.true_saddle) {
            cout << "Found correct peak on edge = incorrect";
            exit(-1);
        }
        if (area.true_saddle && !area.is_peak) {
            cout << "Found true saddle for non peak = incorrect";
            exit(-1);
        }
        if (area.is_peak && area.true_saddle) {
            total_peaks++;
            auto [x, y] = area.representative;
            double true_lat = lat + 1 - (double) x / HGT_TILE_SIZE;
            double true_lon = lon + (double) y / HGT_TILE_SIZE;
            fprintf(output_file, "%.6f %.6f %d\n", true_lat, true_lon, area.height - area.saddle_height);
        }
    }
    // fprintf(output_file, "HERE ARE THE CORRECT ONES DONE\n"); // - debug purposes
    fclose(output_file);
}

void writeAnswerForArea(const Area &area, bool ign) {
    total_peaks++;
    string filename = getOutputFileName(area.lat, area.lon);
    FILE *output_file = fopen(filename.c_str(), "a");

    if (output_file == nullptr) {
        cout << filename << '\n';
        return;
    }
    if (ign) {
        auto [x, y] = area.representative;
        double true_lat = area.lat + 1 - (double) x / HGT_TILE_SIZE;
        double true_lon = area.lon + (double) y / HGT_TILE_SIZE;
        int prom;
        if (area.saddle_height == -1)
            prom = area.height;
        else
            prom = area.height - area.saddle_height;

        fprintf(output_file, "%.6f %.6f %d\n", true_lat, true_lon, prom);
        fclose(output_file);
        return;
    }
    if (area.is_peak && area.is_edge && area.true_saddle) {
        cout << "Found correct peak on edge = incorrect";
        exit(-1);
    }
    if (area.true_saddle && !area.is_peak) {
        cout << "Found true saddle for non peak = incorrect";
        exit(-1);
    }
    if (area.is_peak && area.true_saddle) {
        auto [x, y] = area.representative;
        double true_lat = area.lat + 1 - (double) x / HGT_TILE_SIZE;
        double true_lon = area.lon + (double) y / HGT_TILE_SIZE;
        fprintf(output_file, "%.6f %.6f %d\n", true_lat, true_lon, area.height - area.saddle_height);
    }
    fclose(output_file);
}

int countBadOnes(const vector<Area> &areas) {
    int cnt = 0;
    for (auto area: areas)
        if (area.is_peak && !area.true_saddle)
            cnt++;

    return cnt;
}

int countPeaks(const vector<Area> &areas) {
    int cnt = 0;
    for (auto area: areas)
        if (area.is_peak)
            cnt++;

    return cnt;
}

void exportInfo(int minLat, int minLon, int peak_count, int bad){
    FILE *output_file = fopen(info_file.c_str(), "a");
    fprintf(output_file, "%d %d %d %d\n", minLat, minLon, peak_count, bad);
    fclose(output_file);
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
    fclose(down_input_file);
    fclose(right_input_file);
    return true;
}

#endif //ALGORITHM_UTILS_H
