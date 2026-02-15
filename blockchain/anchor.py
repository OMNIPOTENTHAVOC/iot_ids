import sys
# A simple mock script to complete the loop without needing a full blockchain node running right now
def mock_anchor(root_hash):
    print(f"[Blockchain-Script] 🔗 Received Root: {root_hash}")
    print(f"[Blockchain-Script] ✅ Mock Transaction confirmed on ledger.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 anchor.py <root_hash>")
    else:
        mock_anchor(sys.argv[1])
