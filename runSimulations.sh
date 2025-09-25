#!/bin/bash

# Author: Colby Thompson
# Date: 01/02/2025
# Purpose: Executing this script will generate N 1800GeV di-gluino R-Hadron samples given the current R-Hadron energy deposit model in SimG4Core.
#          The script will then pass the file through the reconstruction chain, and ultimately through the current NTuplizer.
#          The script will also create CSVs of the R-Hadron energy deposits during simulation, and also a CSV of the R-Hadron vertices from the GEN-SIM AOD ROOT file.

# CONTROL CENTER ----------------------

mass=500
events=2
flavor="stop" # gluino or stop
cmEnergy="13TeV"
prefix="~t->t_or_b_andNeutralino_25ns_quickTest"
gensim=true
eventdisplay=false # Set to true to create CSVs of the R-Hadron energy deposits during simulation for the purpose of an event display

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
        
        if [ "$flavor" = "gluino" ]; then
            echo "Generating $events gluino R-hadrons events with mass $mass GeV"
            cmsRun simulate_gluinoRhadron_decays.py maxEvents=$events mass=$mass outputFile=data/$dir_name/$genSimRoot > "data/$dir_name/terminalOutput.log"
        elif [ "$flavor" = "stop" ]; then
            echo "Generating $events stop R-hadrons events with mass $mass GeV"
            cmsRun simulate_stopRhadron_decays.py maxEvents=$events mass=$mass outputFile=data/$dir_name/$genSimRoot > "data/$dir_name/terminalOutput.log"
        else
            echo "Invalid flavor specified. Please use 'gluino' or 'stop'."
            exit 1
        fi

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