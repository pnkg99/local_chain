#!/bin/bash

echo "🔧 Pokrećem test_chain.py..."
python3 /opt/local_chain/test_chain.py start

echo "🔧 Pokrećem inery-gui-build iz Package direktorijuma (u pozadini)..."
cd /opt/Package
nohup ./inery-gui-build &

# Sačekaj da se `nodine` pokrene
echo "⌛ Čekam da se pokrene nodine..."
sleep 5

# Proveri da li `nodine` radi
while pgrep nodine > /dev/null; do
    sleep 5  # ponovo proveri svakih 5 sekundi
done

echo "❌ Proces nodine je ugašen. Kontejner se zatvara."
exit 1  # Kada nodine padne, kontejner se automatski gasi
