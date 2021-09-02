### Simone Valdre' - 25/08/2021. Distributed under GPL-3.0-or-later licence

all: classes test.out ELossGUI
	

classes:
	$(MAKE) -C modules all

test.out: test.cpp modules/ELoss.o modules/Spline.o modules/ecorr_veda.o modules/vedaloss.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm -lgfortran

ELossGUI: main/ELossGUI.cpp modules/ELossDict.cxx modules/ELoss.o modules/Spline.o modules/ecorr_veda.o modules/vedaloss.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm -lgfortran `root-config --cflags --glibs`
	cp -p modules/ELossDict_rdict.pcm .

clean:
	rm -f *.out modules/*.o *.pcm modules/*.pcm ELossGUI
