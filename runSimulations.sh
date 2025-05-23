#!/bin/bash

# Author: Colby Thompson
# Date: 01/02/2025
# Purpose: Executing this script will generate N 1800GeV di-gluino R-Hadron samples given the current R-Hadron energy deposit model in SimG4Core.
#          The script will then pass the file through the reconstruction chain, and ultimately through the current NTuplizer.
#          The script will also create CSVs of the R-Hadron energy deposits during simulation, and also a CSV of the R-Hadron vertices from the GEN-SIM AOD ROOT file.

# CONTROL CENTER ----------------------

mass=1800
events=1
cmEnergy="13TeV"
prefix="test_v1.00"
gensim=true
eventdisplay=true # Set to true to create CSVs of the R-Hadron energy deposits during simulation for the purpose of an event display

# -------------------------------------

# cd into correct directory
cd Demo/SpikedRHadronAnalyzer

# Create data directory if it does not already exist
dir_name=$prefix"_M"$mass"_"$cmEnergy"_pythia8"
echo "All files will be saved in $dir_name"

if [ ! -d "data/$dir_name" ]; then
    mkdir -p data/$dir_name
    echo "creating data/${dir_name}"
fi

# gen-sim output files
genSimRoot=$prefix"_gensimM"$mass"_"$events"Events.root"

if $gensim; then

    if [ ! -f data/$dir_name/$genSimRoot ]; then
        echo "Starting step 0: GEN-SIM"
        cmsRun simulate_rhadron_decays.py maxEvents=$events outputFile=data/$dir_name/$genSimRoot
        echo "Step 0 completed"
    fi

fi


if $eventdisplay; then

    if [ ! -f data/$dir_name/$prefix-eventdisplay.csv ]; then
        echo "Now analyzing the data"
        echo "Creating CSV from EDMAnalyzer over GEN-SIM"
        cmsRun python/SpikedRHadronAnalyzer_cfg.py inputFiles=file:data/$dir_name/$genSimRoot outputFile=data/$dir_name/$prefix-eventdisplay.csv
    fi

fi