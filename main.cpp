#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <sys/stat.h>

using namespace std;

const string DATA_FILE = "kvstore.dat";

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Compact storage: sort by index, then by value
struct Entry {
    string index;
    int value;
    bool operator<(const Entry& other) const {
        if (index != other.index) return index < other.index;
        return value < other.value;
    }
    bool operator==(const Entry& other) const {
        return index == other.index && value == other.value;
    }
};

vector<Entry> entries;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    // Read existing entries
    if (file_exists(DATA_FILE)) {
        ifstream fin(DATA_FILE);
        string index;
        int value;
        while (fin >> index >> value) {
            entries.push_back({index, value});
        }
        fin.close();
    }
    
    // Sort for binary search
    sort(entries.begin(), entries.end());
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string cmd;
        cin >> cmd;
        
        if (cmd == "insert") {
            string index;
            int value;
            cin >> index >> value;
            // Check if already exists
            auto it = lower_bound(entries.begin(), entries.end(), Entry{index, value});
            if (it == entries.end() || !(it->index == index && it->value == value)) {
                entries.insert(it, {index, value});
            }
        } else if (cmd == "delete") {
            string index;
            int value;
            cin >> index >> value;
            auto it = lower_bound(entries.begin(), entries.end(), Entry{index, value});
            if (it != entries.end() && it->index == index && it->value == value) {
                entries.erase(it);
            }
        } else if (cmd == "find") {
            string index;
            cin >> index;
            
            auto lower = lower_bound(entries.begin(), entries.end(), Entry{index, INT_MIN});
            
            // Collect matching values
            vector<int> values;
            for (auto it = lower; it != entries.end() && it->index == index; ++it) {
                values.push_back(it->value);
            }
            
            if (values.empty()) {
                cout << "null\n";
            } else {
                for (size_t j = 0; j < values.size(); j++) {
                    if (j > 0) cout << " ";
                    cout << values[j];
                }
                cout << "\n";
            }
        }
    }
    
    // Save entries
    ofstream fout(DATA_FILE);
    for (auto& e : entries) {
        fout << e.index << " " << e.value << "\n";
    }
    fout.close();
    
    return 0;
}
