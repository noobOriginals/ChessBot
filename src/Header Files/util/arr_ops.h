void swap(int &a, int &b) {
    int buffer = a;
    a = b;
    b = buffer;
}

int find(int item, int *arr, int begin, int end) {
    int m;
    while (begin <= end) {
        m = (begin + end) / 2;
        if (arr[m] == item) {
            return m;
        }
        if (arr[m] > item) {
            end = m - 1;
        }
        else {
            begin = m + 1;
        }
    }
    return -1;
}

int find(unsigned int item, unsigned int *arr, int begin, int end) {
    int m;
    while (begin <= end) {
        m = (begin + end) / 2;
        if (arr[m] == item) {
            return m;
        }
        if (arr[m] > item) {
            end = m - 1;
        }
        else {
            begin = m + 1;
        }
    }
    return -1;
}

void quickSort(int *arr, int begin, int end) {
    if (begin >= end) {
        return;
    }
    int p, i;
    p = end;
    i = begin;
    for (int j = i; j < p; j++) {
        if (arr[j] < arr[p]) {
            swap(arr[i], arr[j]);
            i++;
        }
    }
    swap(arr[i], arr[p]);
    quickSort(arr, i + 1, end);
    quickSort(arr, begin, i - 1);
}