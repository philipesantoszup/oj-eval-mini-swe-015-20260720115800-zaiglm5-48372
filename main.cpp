#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <functional>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

using namespace std;

const string DATA_DIR = "store";
const int NUM_BUCKETS = 16; // Use 16 bucket files (well under 20 limit)

// Simple hash function for bucket selection
size_t get_bucket(const string& index) {
    hash<string> hasher;
    size_t h = hasher(index);
    return h % NUM_BUCKETS;
}

// Get line prefix for an index in a bucket file
string prefix(const string& index) {
    return "[" + index + "]";
}

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void ensure_data_dir() {
    mkdir(DATA_DIR.c_str(), 0755);
}

// Read all entries from a bucket file
void read_bucket(int bucket, map<string, set<int>>& data) {
    string filepath = DATA_DIR + "/" + to_string(bucket) + ".dat";
    if (!file_exists(filepath)) return;
    
    ifstream fin(filepath);
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        // Parse: [index] value
        if (line[0] == '[') {
            size_t end_bracket = line.find(']');
            if (end_bracket != string::npos) {
                string index = line.substr(1, end_bracket - 1);
                int value = stoi(line.substr(end_bracket + 1));
                data[index].insert(value);
            }
        }
    }
    fin.close();
}

// Write all entries to a bucket file
void write_bucket(int bucket, const map<string, set<int>>& data) {
    string filepath = DATA_DIR + "/" + to_string(bucket) + ".dat";
    ofstream fout(filepath);
    for (auto& p : data) {
        for (int v : p.second) {
            fout << "[" << p.first << "]" << v << "\n";
        }
    }
    fout.close();
}

// Read entries for a specific index from its bucket
set<int> read_index_values(const string& index) {
    set<int> values;
    int bucket = get_bucket(index);
    string filepath = DATA_DIR + "/" + to_string(bucket) + ".dat";
    if (!file_exists(filepath)) return values;
    
    ifstream fin(filepath);
    string line;
    string target_prefix = prefix(index);
    while (getline(fin, line)) {
        if (line.empty()) continue;
        if (line.compare(0, target_prefix.size(), target_prefix) == 0) {
            size_t end_bracket = line.find(']');
            if (end_bracket != string::npos) {
                int value = stoi(line.substr(end_bracket + 1));
                values.insert(value);
            }
        }
    }
    fin.close();
    return values;
}

// Update a specific index in its bucket
void update_index(const string& index, const set<int>& values) {
    int bucket = get_bucket(index);
    string filepath = DATA_DIR + "/" + to_string(bucket) + ".dat";
    string temp_path = DATA_DIR + "/" + to_string(bucket) + ".tmp";
    
    ifstream fin;
    ofstream fout(temp_path);
    
    // Write the updated values first
    for (int v : values) {
        fout << "[" << index << "]" << v << "\n";
    }
    
    string target_prefix = prefix(index);
    if (file_exists(filepath)) {
        fin.open(filepath);
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            // Copy entries not for this index
            if (line.compare(0, target_prefix.size(), target_prefix) != 0) {
                fout << line << "\n";
            }
        }
        fin.close();
    }
    
    fout.close();
    rename(temp_path.c_str(), filepath.c_str());
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    ensure_data_dir();
    
    int n;
    cin >> n;
    
    // Track which indices we've modified
    map<int, map<string, set<int>>> modified_buckets;
    
    for (int i = 0; i < n; i++) {
        string cmd, index;
        cin >> cmd >> index;
        int bucket = get_bucket(index);
        
        if (cmd == "insert") {
            int value;
            cin >> value;
            
            // Lazy load if not already loaded
            if (modified_buckets.find(bucket) == modified_buckets.end()) {
                read_bucket(bucket, modified_buckets[bucket]);
            }
            modified_buckets[bucket][index].insert(value);
        } else if (cmd == "delete") {
            int value;
            cin >> value;
            
            if (modified_buckets.find(bucket) == modified_buckets.end()) {
                read_bucket(bucket, modified_buckets[bucket]);
            }
            modified_buckets[bucket][index].erase(value);
            if (modified_buckets[bucket][index].empty()) {
                modified_buckets[bucket].erase(index);
            }
        } else if (cmd == "find") {
            set<int> values;
            if (modified_buckets.find(bucket) != modified_buckets.end() &&
                modified_buckets[bucket].find(index) != modified_buckets[bucket].end()) {
                values = modified_buckets[bucket][index];
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
    
    // Write modified buckets back
    for (auto& p : modified_buckets) {
        int bucket = p.first;
        map<string, set<int>> data = p.second;
        
        // Merge with non-modified entries from same bucket
        string filepath = DATA_DIR + "/" + to_string(bucket) + ".dat";
        if (file_exists(filepath)) {
            ifstream fin(filepath);
            string line;
            while (getline(fin, line)) {
                if (line.empty()) continue;
                if (line[0] == '[') {
                    size_t end_bracket = line.find(']');
                    if (end_bracket != string::npos) {
                        string idx = line.substr(1, end_bracket - 1);
                        // Only add if we haven't modified this index
                        if (data.find(idx) == data.end()) {
                            int value = stoi(line.substr(end_bracket + 1));
                            data[idx].insert(value);
                        }
                    }
                }
            }
            fin.close();
        }
        
        write_bucket(bucket, data);
    }
    
    return 0;
}
