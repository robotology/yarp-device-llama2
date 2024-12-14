#!/bin/bash

# Assicurati di essere nella directory principale del repository
echo "Inizializzazione e aggiornamento del submodule llama.cpp..."

# Inizializza e aggiorna il submodule
git submodule init
git submodule update

# Spostati nella directory del submodule
cd llama.cpp

# Checkout al commit desiderato
git checkout 167a515651a4b065a16225ffc69564c5674f3d0f

# Torna alla root del repository principale
cd ..

echo "Submodule aggiornato al commit desiderato."
