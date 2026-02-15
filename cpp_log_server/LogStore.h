#ifndef LOG_STORE_H
#define LOG_STORE_H

#include <vector>
#include <string>
#include "MerkleTree.h"

class LogStore {
private:
    std::vector<std::string> current_batch;
    const size_t BATCH_SIZE = 5; // Small batch size for testing
    int batch_count = 0;

public:
    void add_log(const std::string& log_entry) {
        current_batch.push_back(log_entry);
        
        // Check if batch is full
        if (current_batch.size() >= BATCH_SIZE) {
            finalize_batch();
        }
    }

    void finalize_batch() {
        std::cout << "\n[LogStore] Batch " << ++batch_count << " Full (" << BATCH_SIZE << " logs)." << std::endl;
        
        // Calculate Merkle Root
        std::string root = MerkleTree::compute_root(current_batch);
        
        std::cout << "[Merkle Root] " << root << std::endl;
        
        // Execute the python script. 
        // NOTE: Adjust the path "../anchor.py" depending on where you put the file!
        std::string cmd = "python3 ../../blockchain/anchor.py " + root;
        int result = std::system(cmd.c_str());

        // Clear for next batch
        current_batch.clear();
    }
};

#endif
