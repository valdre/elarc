### Simone Valdre' - 25/08/2021. Distributed under GPL-3.0-or-later licence

all: ELoss.o ELossDict.cxx
	

.cpp.o:
	g++ -Wall -Wextra -c -o $@ $<

# ROOT DICTIONARIES
ELossDict.cxx: ELossGUI.h ELossLinkDef.h
	rootcling -f $@ -c $^
