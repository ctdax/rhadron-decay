# rhadron-decay

A revised version of CMSSW's SimG4Core that includes R-hadron decay through Pythia8.

Pythia8ForDecays.cc handles the SLHA input and has functions to do the decays. RHadronPythiaDecayer.cc is what is called to do the decaying. Both need to be updated to work with CMSSW.

### Current status

- Compiles with warnings
- Pythia instance defined with normal pointer rather than unique pointer because Pythia8 could not be loaded into Pythia8ForDecays.h. This means that I had to implement a destructor in the .cc file rather than defining the default destructor in the .h file
- Pythia instance defined with default parameters rather than with an XML that was previously defined in ATLAS
- XML BuildFiles updated to link the new files

### Useful links

- **[SLHA Generation](https://ballanach.github.io/softsusy/)**