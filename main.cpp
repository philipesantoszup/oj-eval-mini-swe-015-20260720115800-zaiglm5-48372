#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <sys/stat.h>

using namespace std;

const int MAX_INDEX_LEN = 65;
const string DATA_FILE = "kvstore.dat";

// Fixed-size index for memory efficiency
struct Entry {
    char index[MAX_INDEX_LEN];
    int value;
    
    bool operator<(const Entry& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
    
    bool operator==(const Entry& other) const {
        return strcmp(index, other.index) == 0 && value == other.value;
    }
};

vector<Entry> entries;

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    // Read existing entries
    if (file_exists(DATA_FILE)) {
        FILE* fin = fopen(DATA_FILE.c_str(), "r");
        char index[MAX_INDEX_LEN];
        int value;
        while (fscanf(fin, "%64s %d", index, &value) == 2) {
            Entry e;
            strcpy(e.index, index);
            e.value = value;
            entries.push_back(e);
        }
        fclose(fin);
    }
    
    // Sort for binary search
    sort(entries.begin(), entries.end());
    
    int n;
    scanf("%d", &n);
    
    char cmd[10];
    char index[MAX_INDEX_LEN];
    int value;
    
    for (int i = 0; i < n; i++) {
        scanf("%s", cmd);
        
        if (cmd[0] == 'i') { // insert
            scanf("%s %d", index, &value);
            Entry target;
            strcpy(target.index, index);
            target.value = value;
            auto it = lower_bound(entries.begin(), entries.end(), target);
            if (it == entries.end() || strcmp(it->index, index) != 0 || it->value != value) {
                entries.insert(it, target);
            }
        } else if (cmd[0] == 'd') { // delete
            scanf("%s %d", index, &value);
            Entry target;
            strcpy(target.index, index);
            target.value = value;
            auto it = lower_bound(entries.begin(), entries.end(), target);
            if (it != entries.end() && strcmp(it->index, index) == 0 && it->value == value) {
                entries.erase(it);
            }
        } else { // find
            scanf("%s", index);
            Entry target;
            strcpy(target.index, index);
            target.value = -2147483647; // INT_MIN equivalent
            
            auto lower = lower_bound(entries.begin(), entries.end(), target);
            
            // Collect and output matching values
            bool first = true;
            for (auto it = lower; it != entries.end() && strcmp(it->index, index) == 0; ++it) {
                if (!first) putchar(' ');
                printf("%d", it->value);
                first = false;
            }
            if (first) {
                puts("null");
            } else {
                putchar('\n');
            }
        }
    }
    
    // Save entries
    FILE* fout = fopen(DATA_FILE.c_str(), "w");
    for (auto& e : entries) {
        fprintf(fout, "%s %d\n", e.index, e.value);
    }
    fclose(fout);
    
    return 0;
}
