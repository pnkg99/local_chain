#!/bin/bash
set -e

# Podesite putanje prema potrebi
DATA_GENESIS="/opt/local_chain/Data/genesis"
TEST_CHAIN_SCRIPT="/opt/local_chain/test_chain.py"
HARD_REPLAY_SCRIPT="/opt/local_chain/Data/genesis/hard_replay.sh"
GUI_BINARY="/opt/local_chain/Packages/inery-gui-build"

# 1. Provjeri postoji li genesis datoteka
if [ ! -d "$DATA_GENESIS" ]; then
    echo "🔧 Nema genesis datoteke. Pokrećem test_chain.py za inicijalizaciju nodine..."
    python3 "$TEST_CHAIN_SCRIPT" start
else
    # 2. Ako genesis postoji, ali nodine nije pokrenut, radimo hard replay
    #    (Pod "nodine proces" ovdje pretpostavljamo da se može pronaći s `pgrep -f nodine`
    #     ili da je to zapravo 'inery-gui-build'; prilagodite prema vašem procesu.)
    if ! pgrep -f "nodine" > /dev/null 2>&1; then
        echo "🔧 Genesis postoji, ali nodine nije aktivan. Pokrećem hard replay..."
        bash "$HARD_REPLAY_SCRIPT"
    fi
fi

# 3. Pokreni web GUI (u pozadini)

if ! pgrep -f "inery-gui-build" > /dev/null 2>&1; then
    echo "🔧 Pokrećem inery-gui-build (web GUI) u pozadini..."
    cd /opt/local_chain/Packages
    nohup "$GUI_BINARY" &
fi

echo "⌛ Čekamo babu da donese ustipke..."
sleep 2

echo "🔎 Pratim inery-gui-build proces. Kontejner ostaje aktivan dok inery-gui-build radi..."

while pgrep -f "inery-gui-build" > /dev/null 2>&1; do
    sleep 5
done

# Ako padne nodine, gasi se kontejner.
echo "❌ Proces nodine je ugašen. Kontejner se zatvara."
exit 1
