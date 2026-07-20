#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <tuple>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

const string DATA_FILE = "kvstore.dat";
const string TEMP_FILE = "kvstore.tmp";

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Read entries for a specific index from file
set<int> read_index_values(const string& target_index) {
    set<int> values;
    if (!file_exists(DATA_FILE)) return values;
    
    ifstream fin(DATA_FILE);
    string index;
    int value;
    while (fin >> index >> value) {
        if (index == target_index) {
            values.insert(value);
        }
    }
    fin.close();
    return values;
}

// Write all entries, updating one index
void write_with_update(const string& target_index, const set<int>& new_values) {
    ifstream fin;
    ofstream fout(TEMP_FILE);
    
    // First, write the updated index
    for (int v : new_values) {
        fout << target_index << " " << v << "\n";
    }
    
    // Copy other entries
    if (file_exists(DATA_FILE)) {
        fin.open(DATA_FILE);
        string index;
        int value;
        while (fin >> index >> value) {
            if (index != target_index) {
                fout << index << " " << value << "\n";
            }
        }
        fin.close();
    }
    
    fout.close();
    
    // Atomic replace
    rename(TEMP_FILE.c_str(), DATA_FILE.c_str());
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    // Track which indices we've modified (to avoid re-reading)
    map<string, set<int>> modified;
    
    for (int i = 0; i < n; i++) {
        string cmd, index;
        cin >> cmd >> index;
        
        if (cmd == "insert") {
            int value;
            cin >> value;
            
            // Get current values (from memory or file)
            if (modified.find(index) == modified.end()) {
                modified[index] = read_index_values(index);
            }
            modified[index].insert(value);
        } else if (cmd == "delete") {
            int value;
            cin >> value;
            
            // Get current values (from memory or file)
            if (modified.find(index) == modified.end()) {
                modified[index] = read_index_values(index);
            }
            modified[index].erase(value);
        } else if (cmd == "find") {
            set<int> values;
            if (modified.find(index) != modified.end()) {
                values = modified[index];
            } else {
                values = read_index_values(index);
            }
            
            if (values.empty()) {
                cout << "null\n";
            } else {
                bool first = true;
                for (int v : values) {
                    if (!first) cout << " ";
                    cout << v;
                    first = false;
                }
                cout << "\n";
            }
        }
    }
    
    // Write all modified indices back
    if (!modified.empty()) {
        // We need to merge with existing data
        // Build a complete view
        map<string, set<int>> all_data;
        
        // Read unmodified entries from file
        if (file_exists(DATA_FILE)) {
            ifstream fin(DATA_FILE);
            string index;
            int value;
            while (fin >> index >> value) {
                if (modified.find(index) == modified.end()) {
                    all_data[index].insert(value);
                }
            }
            fin.close();
        }
        
        // Add modified entries
        for (auto& p : modified) {
            all_data[p.first] = p.second;
        }
        
        // Write back
        ofstream fout(DATA_FILE);
        for (auto& p : all_data) {
            for (int v : p.second) {
                fout << p.first << " " << v << "\n";
            }
        }
        fout.close();
    }
    
    return 0;
}
