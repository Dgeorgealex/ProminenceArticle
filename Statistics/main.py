import statistics

info_file = "."


def read_data():
    data = []
    with open(info_file) as file:
        for line in file:
            t = tuple(map(int, line.split()))
            percentage = t[3] / t[2] * 100
            t = t + (percentage,)
            data.append(t)

    print(f"Number of samples: {len(data)}")
    return data


def calculate_statistics_with_index(data, e):
    element_with_index = [(i, item[e]) for i, item in enumerate(data)]
    elements = [item[1] for item in element_with_index]

    minimum = min(elements)
    maximum = max(elements)

    sorted_elements = sorted(elements)
    n = len(elements)
    med = n // 2 - (n + 1) % 2
    median = sorted_elements[med]

    min_index = elements.index(minimum)
    max_index = elements.index(maximum)
    median_index = elements.index(median)
    avg = sum(elements) / len(elements)
    q = [statistics.quantiles(elements, n=4)[0], statistics.quantiles(elements, n=4)[2]]
    return (minimum, min_index), (maximum, max_index), avg, (median, median_index), q


def main():
    data = read_data()
    peaks_data = calculate_statistics_with_index(data, 2)
    print(peaks_data)
    bad_peaks_data = calculate_statistics_with_index(data, 3)
    print(bad_peaks_data)
    percentage_data = calculate_statistics_with_index(data, 4)
    print(percentage_data)
    for e in percentage_data:
        if type(e) == tuple:
            print(data[e[1]][3])


if __name__ == "__main__":
    main()
