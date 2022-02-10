### Simone Valdre' - 25/11/2021. Distributed under GPL-3.0-or-later licence

all: classes ELossGUI ProfileGUI UpdateTables
	

classes:
	$(MAKE) -C modules all

ELossGUI: main/ELossGUI.cpp modules/ELossDict.cxx modules/ELoss.o modules/Spline.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm `root-config --cflags --glibs`
	cp -p modules/ELossDict_rdict.pcm .

ProfileGUI: main/ProfileGUI.cpp modules/ProfileDict.cxx modules/ELoss.o modules/Spline.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm `root-config --cflags --glibs`
	cp -p modules/ProfileDict_rdict.pcm .

UpdateTables: main/UpdateTables.cpp modules/ELoss.o modules/Spline.o
	g++ -Wall -Wextra -Imodules -o $@ $^ -lm

clean:
	rm -f *.out modules/*.o *.pcm modules/*.pcm modules/*.cxx ELossGUI ProfileGUI UpdateTables db/vedasac2coe_C*
