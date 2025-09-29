#!/bin/bash

# Author: Colby Thompson
# Date: 01/02/2025
# Purpose: Executing this script will generate N 1800GeV di-gluino R-Hadron samples given the current R-Hadron energy deposit model in SimG4Core.
#          The script will then pass the file through the reconstruction chain, and ultimately through the current NTuplizer.
#          The script will also create CSVs of the R-Hadron energy deposits during simulation, and also a CSV of the R-Hadron vertices from the GEN-SIM AOD ROOT file.

# CONTROL CENTER ----------------------

mass=1800
events=1
flavor="gluino" # gluino or stop
cmEnergy="13TeV"
#prefix="stop2t_or_b_andNeutralino_25ns_quickTest"
prefix="g2qq_andNeutralino_50ns_run3"
gensim=true
reco=true
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

# digi-L1-digi2ray output files
digiRawRoot="digirawM"$mass"_"$events"Events.root"
digiRawOut="digirawM"$mass"_"$events"Events.out"

# reco output files
hltRoot="hltM"$mass"_"$events"Events.root"
hltOut="hltM"$mass"_"$events"Events.out"
recoRoot="recoM"$mass"_"$events"Events.root"
recoOut="recoM"$mass"_"$events"Events.out"

if $gensim; then

    if [ ! -f data/$dir_name/$genSimRoot ]; then
        echo "Starting step 0: GEN-SIM"
        
        if [ "$flavor" = "gluino" ]; then
            echo "Generating $events gluino R-hadrons events with mass $mass GeV"
            cmsRun simulate_gluinoRhadron_decays_Run3.py maxEvents=$events mass=$mass outputFile=data/$dir_name/$genSimRoot > "data/$dir_name/terminalOutput.log"
        elif [ "$flavor" = "stop" ]; then
            echo "Generating $events stop R-hadrons events with mass $mass GeV"
            cmsRun simulate_stopRhadron_decays_Run3.py maxEvents=$events mass=$mass outputFile=data/$dir_name/$genSimRoot > "data/$dir_name/terminalOutput.log"
        else
            echo "Invalid flavor specified. Please use 'gluino' or 'stop'."
            exit 1
        fi

        echo "Step 0 completed"
    fi

fi

if $reco; then

    if [ ! -f data/$dir_name/$digiRawRoot ]; then
        echo "Starting step 1: DIGI-L1-DIGI2RAW"
        cmsDriver.py --filein file:data/$dir_name/$genSimRoot \
            --fileout file:data/$dir_name/$digiRawRoot\
            --mc \
            --eventcontent RAWSIM \
            --datatier GEN-SIM-RAW \
            --conditions 140X_mcRun3_2024_realistic_v11 \
            --step DIGI,L1,DIGI2RAW \
            --python_filename data/$dir_name/step1_cfg.py \
            --era Run3_2024 \
            -n -1 >& data/$dir_name/$digiRawOut
        echo "Step 1 completed"
    fi

    if [ ! -f data/$dir_name/$recoRoot ]; then
        echo "Starting step 2: RAW2DIGI-L1Reco-RECO"
        cmsDriver.py --filein file:data/$dir_name/$digiRawRoot \
            --fileout file:data/$dir_name/$recoRoot \
            --mc \
            --eventcontent FEVTDEBUGHLT \
            --datatier AODSIM \
            --conditions 140X_mcRun3_2024_realistic_v11 \
            --step RAW2DIGI,L1Reco,RECO \
            --python_filename data/$dir_name/step2_cfg.py \
            --era Run3_2024 \
            -n -1 >& data/$dir_name/$recoOut
        echo "Step 2 completed"
    fi
    
fi


if $eventdisplay; then

    if [ ! -f data/$dir_name/$prefix-eventdisplay.csv ]; then
        echo "Now analyzing the data"
        echo "Creating CSV from EDMAnalyzer over GEN-SIM"
        cmsRun python/SpikedRHadronAnalyzer_cfg.py inputFiles=file:data/$dir_name/$genSimRoot outputFile=data/$dir_name/$prefix-eventdisplay.csv
    fi

fi