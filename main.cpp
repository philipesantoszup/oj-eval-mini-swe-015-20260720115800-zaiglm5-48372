#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <sys/stat.h>

using namespace std;

const string DATA_FILE = "kvstore.dat";

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    // Read all existing data
    map<string, set<int>> data;
    
    if (file_exists(DATA_FILE)) {
        ifstream fin(DATA_FILE);
        string index;
        int value;
        while (fin >> index >> value) {
            data[index].insert(value);
        }
        fin.close();
    }
    
    // Process commands
    for (int i = 0; i < n; i++) {
        string cmd;
        cin >> cmd;
        
        if (cmd == "insert") {
            string index;
            int value;
            cin >> index >> value;
            data[index].insert(value);
        } else if (cmd == "delete") {
            string index;
            int value;
            cin >> index >> value;
            if (data.count(index)) {
                data[index].erase(value);
                if (data[index].empty()) {
                    data.erase(index);
                }
            }
        } else if (cmd == "find") {
            string index;
            cin >> index;
            if (data.count(index) && !data[index].empty()) {
                bool first = true;
                for (int v : data[index]) {
                    if (!first) cout << " ";
                    cout << v;
                    first = false;
                }
                cout << "\n";
            } else {
                cout << "null\n";
            }
        }
    }
    
    // Save all data back to file
    ofstream fout(DATA_FILE);
    for (auto& p : data) {
        for (int v : p.second) {
            fout << p.first << " " << v << "\n";
        }
    }
    fout.close();
    
    return 0;
}
