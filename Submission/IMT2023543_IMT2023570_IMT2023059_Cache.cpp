#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <bitset>
#include <sstream>
using namespace std;
class Cache {
    private:
        int ways;               // Number of ways
        int cache_size;         // Cache size in kilobytes
        int block_size;         // Block size in bytes
        double hit_rate;        // Hit Rate of Cache
        double miss_rate;       // Miss Rate of Cache
        long long int hits;     // Number of hits
        long long int misses;   // Number of misses
        int indices;            // Number of lines (indices)
        int index_bits;         // Number of index bits
        int offset_bits;        // Number of offset bits
        int tag_bits;           // Number of tag bits
        vector<vector<vector<string>>> cache_mem;   // Cache in the form `[Tag, Valid Bit, LRU]`
        vector<string> traces;                      // Vector of trace files

    public:

        Cache(int ways1, int cache_size1, int block_size1)
            : ways(ways1), cache_size(cache_size1), block_size(block_size1) {
            traces = {
                ".\\traces\\gcc.trace",
                ".\\traces\\gzip.trace",
                ".\\traces\\mcf.trace",
                ".\\traces\\swim.trace",
                ".\\traces\\twolf.trace"
            };

            hits = 0;
            misses = 0;
            hit_rate = 0;
            miss_rate = 0;

            // Calculate the values
            indices = (cache_size * 1024) / (block_size * ways);
            index_bits = log2(indices);
            offset_bits = log2(block_size);
            tag_bits = 32 - index_bits - offset_bits;
        }

        // Convert the hex string to an integer
        string hexTo32BitBinary(const string& hexStr) {
            unsigned int value;         // Takes the hex string as a hex value
            stringstream ss;
            ss << std::hex << hexStr;
            ss >> value;

            // Use `std::bitset` to convert the integer to a 32-bit binary string
            bitset<32> binary(value);

            // Return the binary string representation
            return binary.to_string();
        }

        // Function to calculate miss/hit rates
        void calculateRates() {
            int fileNo = 0;
            // File names to display
            vector<string> FileName = {"gcc.trace", "gzip.trace", "mcf.trace", "swim.trace", "twolf.trace"};
            
            for (const auto& trace_iter : traces) {
                hits = 0;
                misses = 0;
                cache_mem.clear();      // Clear cache for next file
                cache_mem.resize(ways, vector<vector<string>>(indices, vector<string>(3, "0")));    // Initialize the cache with all 0s
                ifstream inputFile(trace_iter); // Open the file

                // If can't open file
                if (!inputFile) {
                    cerr << "Error opening file: " << trace_iter << endl;
                    continue;
                }

                string line;                                                            // Iterator for line of file
                while (getline(inputFile, line)) {
                    line = line.substr(4, 8);                                           // Extract the address part
                    string address = hexTo32BitBinary(line);                            // Binary address string 

                    string Gtag = address.substr(0, tag_bits);                          // Slices the address to get tag
                    int Gindex = stoi(address.substr(tag_bits, index_bits), nullptr, 2);// Get the index of the address requested

                    bool hit = false;                                                   // To see if it is a cache hit or a miss

                    // Implementation of a cache hit 
                    for (size_t j = 0; j < ways; ++j) {
                        // Check of valid and had tag match, then hit
                        if (cache_mem[j][Gindex][0] == Gtag && cache_mem[j][Gindex][1] == "1") {
                            hits++;
                            hit = true;
                            cache_mem[j][Gindex][2] = "0";                              // Set LRU to 0 (recent)
                            for (size_t k = 0; k < ways; ++k) {                         // Increase the LRU value for all other blocks
                                if(k != j) {
                                    int temp = stoi(cache_mem[k][Gindex][2]);           // Change other LRUs
                                    temp++;
                                    cache_mem[k][Gindex][2] = to_string(temp);
                                }
                            }
                            break;
                        }
                    }

                    //Implementation of a cache miss 
                    if (!hit) {
                        misses++;                   
                        int max_lru = -1;                                               // Stores the greatest LRU
                        int max_lru_way = -1;                                           // Finding the way which will be replaced 
                        for (size_t i = 0; i < ways; ++i) {
                            int temp = stoi(cache_mem[i][Gindex][2]);
                            // Set the greatest LRU
                            if (temp > max_lru) {
                                max_lru = temp; 
                                max_lru_way = i;
                            }
                        }
                        cache_mem[max_lru_way][Gindex][0] = Gtag;                       // Replacement of the tag
                        cache_mem[max_lru_way][Gindex][1] = "1";                        // Set valid bit to 1
                        cache_mem[max_lru_way][Gindex][2] = "0";                        // Set LRU to 0 (recent)

                        // Increase the LRU of other blocks
                        for (size_t i = 0; i < ways; ++i) {                            
                            if (i != max_lru_way) {
                                int temp = stoi(cache_mem[i][Gindex][2]);
                                temp++;
                                cache_mem[i][Gindex][2] = to_string(temp);
                            }
                        }
                    }
                }
                inputFile.close();  // Clost the file
                
                double total_instructions_double = static_cast<double>(hits + misses); // Total number of instructions
                hit_rate = static_cast<double>(hits) / total_instructions_double;       
                miss_rate = static_cast<double>(misses) / total_instructions_double;
                double hm = static_cast<double>(hits) / static_cast<double>(misses) ;   // Hit/Miss ratio
                cout << "File name: " << FileName[fileNo] << endl;
                cout << "----------------------" << endl;
                cout << "Lines of cache: " << indices << endl;
                cout << "Total hits is: " << hits << endl;
                cout << "Total misses is: " << misses << endl;
                cout << "Hit Rate (as percent) of file is: " << hit_rate * 100 << endl;
                cout << "Miss Rate (as percent) of file is: " << miss_rate * 100 << endl;
                cout << "Hits\\Miss: " << hm << endl;
                cout << "----------------********************-----------------" << endl;
                fileNo++;
            }
        }
};

int main() {
    int ways;       // Number of ways
    int cache_size; // Cache size in kilobytes
    int block_size; // Block size in bytes

    cout << "Enter the number of ways: ";
    cin >> ways;
    cout << "Enter the cache size in kilobytes: ";
    cin >> cache_size;
    cout << "Enter the block size in bytes: ";
    cin >> block_size;
    cout << "----------------********************-----------------" << endl;

    // Initialize the cache
    Cache L1(ways, cache_size, block_size);
    L1.calculateRates();    // Caclulate and print rates
    cout << "----------------********************-----------------" << endl;

    return 0;
}
