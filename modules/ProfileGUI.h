/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 24/11/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#ifndef _PROFILEGUI
#define _PROFILEGUI

#include <RQ_OBJECT.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TSpline.h>
#include <TH2.h>
#include <TPaveText.h>
#include <TLatex.h>
#include <TLine.h>
#include <TArrow.h>
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

#define WINDOWX 1600
#define WINDOWY 820
#define NLABS 4
#define NARWS 2

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;

class MyMainFrame {
	RQ_OBJECT("MyMainFrame")
private:
	TGMainFrame  *fMain;
	TRootEmbeddedCanvas *fEcanvas;
	TH1D *hD, *hE;
	TSpline3 *sD, *sE;
	TGComboBox *cbmode, *cbin, *cbstrat, *cbform, *cbstate;
	TGNumberEntry *nein, *nepre, *netem, *nerho;
	TGTextEntry *tepro, *tetar;
	TGTextButton *tbcalc, *tbsave;
	TColor *red, *green, *gray, *yello, *black, *white;
	TPaveText *pl[NLABS];
	TArrow *tar[NARWS];
	TLine *tldin;
	
	ELoss el;
	bool up, upPro, fPro, upTar, fTar;
	int proZ, proA, mid;
	std::vector< double > vX, vD, vE;
public:
	MyMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
	virtual ~MyMainFrame();
	
	void ModeSwitch(int);
	void FormSwitch();
	void ProMod();
	void ProApply();
	void AbsMod();
	void AbsApply();
	void GasSwitch(int);
	void SetOutOfDate();
	void Draw();
};

#endif
