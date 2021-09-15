### Simone Valdre' - 25/08/2021. Distributed under GPL-3.0-or-later licence

all: classes ELossGUI UpdateTables
	

classes:
	$(MAKE) -C modules all

ELossGUI: main/ELossGUI.cpp modules/ELossDict.cxx modules/ELoss.o modules/Spline.o modules/ecorr_veda.o modules/vedaloss.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm -lgfortran `root-config --cflags --glibs`
	cp -p modules/ELossDict_rdict.pcm .

UpdateTables: main/UpdateTables.cpp modules/ELoss.o modules/Spline.o modules/ecorr_veda.o modules/vedaloss.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm -lgfortran

clean:
	rm -f *.out modules/*.o *.pcm modules/*.pcm modules/*.cxx ELossGUI
