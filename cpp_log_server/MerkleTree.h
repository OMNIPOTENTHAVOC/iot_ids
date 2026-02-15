#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

class MerkleTree {
public:
    // Helper: Calculate SHA-256 of a string and return hex format
    static std::string sha256(const std::string str) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, str.c_str(), str.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    // Core Logic: Build the tree from a list of log entries
    static std::string compute_root(std::vector<std::string> leaves) {
        if (leaves.empty()) return "";
        
        std::vector<std::string> current_level;

        // Step 1: Hash all leaves
        for (const auto& leaf : leaves) {
            current_level.push_back(sha256(leaf));
        }

        // Step 2: Combine hashes up the tree
        while (current_level.size() > 1) {
            std::vector<std::string> next_level;
            
            for (size_t i = 0; i < current_level.size(); i += 2) {
                if (i + 1 < current_level.size()) {
                    // Combine two nodes: Hash(Left + Right)
                    std::string combined = current_level[i] + current_level[i+1];
                    next_level.push_back(sha256(combined));
                } else {
                    // Odd number of nodes? Duplicate the last one (common strategy)
                    std::string combined = current_level[i] + current_level[i];
                    next_level.push_back(sha256(combined));
                }
            }
            current_level = next_level;
        }

        return current_level[0]; // The Merkle Root
    }
};

#endif
