/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 21/09/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#ifndef _ELOSSGUI
#define _ELOSSGUI

#include <RQ_OBJECT.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGButton.h>
#include <TColor.h>
#include <TVirtualX.h>
#include <TApplication.h>
#include <TROOT.h>
#include <sys/time.h>
#include "ELoss.h"

#define WINDOWX 600
#define WINDOWY 800

class TGWindow;
class TGMainFrame;

class MyMainFrame {
	RQ_OBJECT("MyMainFrame")
private:
	TGMainFrame  *fMain;
	TGComboBox *cbmode, *cbform, *cbstate, *cbin1, *cbin2;
	TGLabel *lin1, *lin2, *lout[2][4];
	TGNumberEntry *nein1, *nein2, *nepre, *netem, *nerho;
	TGTextButton *tbcalc;
	TGTextEntry *tepro, *tetar;
	TColor *red, *green, *gray, *yello, *black, *white;
	
	bool up, upPro, fPro, upTar, fTar;
	
	ELoss el;
	int proZ, proA, mid;
public:
	MyMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
	virtual ~MyMainFrame();
	
	void ModeSwitch(int);
	void FormSwitch();
	void GasSwitch(int);
	void ProMod();
	void ProApply();
	void AbsMod();
	void AbsApply();
	void SetOutOfDate();
	void Calculate();
};

#endif
