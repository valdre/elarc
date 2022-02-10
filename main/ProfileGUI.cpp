/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 25/11/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#include "ProfileGUI.h"

#define CANVASX 1200
#define CANVASY  800
#define BARX     400

#define NEUNIT 3
#define NTUNIT 5

#define MINSTEPE   0.005
#define MAXSTEPE   1.
#define RELSTEPE   0.05

static const char *mode_list[2] = {"Given Ein", "Given range"};
static const char *strat_list[4] = {"From range table", "50% Bragg peak", "Full energy loss", "Maximize Elost"};
static const char *form_list[NFORMULAS] = {"NIST", "SRIM", "Barbui", "Schwalm", "vedaloss"};
static const char *eunit[NEUNIT] = {"keV", "MeV", "MeV/u"};
static const char *tunit[NTUNIT] = {"nm", "um", "mm", "ug/cm2", "mg/cm2"};

double econv(const int &sel, double in, const double A = 1) {
	switch(sel) {
		case 0: in /= 1000.; break;
		case 2: in = ELoss::AMeV_to_MeV(in, A);
	}
	return in;
}

double tconv(const int &sel, double in, const double rho = 1) {
	switch(sel) {
		case 0: in /= 1000.; break;
		case 2: in *= 1000.; break;
		case 3: in /= 1000.; [[fallthrough]];
		case 4: in = ELoss::mgcm2_to_um(in, rho);
	}
	return in;
}

void printval(double v, const char *u, char *out) {
	char mod[11][10] = {"f", "p", "n", "u", "m", " ", "k", "M", "G", "T", "P"};
	
	int idx = ((int)(15.5 + floor(log10(v)))) / 3;
	if(idx < 0)  idx =  0;
	if(idx > 10) idx = 10;
	v *= pow(10., 15 - idx * 3);
	if(fabs(v) >= 100)   sprintf(out, "%6.1f %s%s", v, mod[idx], u);
	else if(fabs(v)>=10) sprintf(out, "%6.2f %s%s", v, mod[idx], u);
	else                 sprintf(out, "%6.3f %s%s", v, mod[idx], u);
	return;
}

MyMainFrame::MyMainFrame(const TGWindow *p, UInt_t w, UInt_t h) {
	FontStruct_t font16 = gClient->GetFontByName("-*-arial-regular-r-*-*-16-*-*-*-*-*-iso8859-1");
	TGGC* GC16 = new TGGC(TGLabel::GetDefaultGC());
	GC16->SetFont(gVirtualX->GetFontHandle(font16));
	
	FontStruct_t font16b = gClient->GetFontByName("-*-arial-bold-r-*-*-16-*-*-*-*-*-iso8859-1");
	TGGC* GC16b = new TGGC(TGLabel::GetDefaultGC());
	GC16b->SetFont(gVirtualX->GetFontHandle(font16b));
	
	yello = gROOT->GetColor(kYellow);
	red   = gROOT->GetColor(kRed);
	green = gROOT->GetColor(kGreen);
	gray  = gROOT->GetColor(kGray + 1);
	black = gROOT->GetColor(kBlack);
	white = gROOT->GetColor(kWhite);
	
	// Create a main frame
	fMain = new TGMainFrame(p, w, h);
	
	//* hlay starts
	TGHorizontalFrame *hlay = new TGHorizontalFrame(fMain, CANVASX + BARX, CANVASY);
	
	// Create canvas widget
	fEcanvas = new TRootEmbeddedCanvas("Ecanvas", hlay, CANVASX, CANVASY);
// 	fEcanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MyMainFrame",this,"HandleMyCanvas(Int_t,Int_t,Int_t,TObject*)"); 
	hlay->AddFrame(fEcanvas, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
	
	//* vlay starts
	TGVerticalFrame *vlay = new TGVerticalFrame(hlay, BARX, CANVASY);
	
	//BEGIN SYSTEM (PROJECTILE, TARGET AND ENERGY LOSS FORMULA)
	TGHorizontalFrame *hfs1 = new TGHorizontalFrame(vlay);
	TGLabel *lform = new TGLabel(hfs1, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hfs1->AddFrame(lform, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbform = new TGComboBox(hfs1);
	for(unsigned j = 0; j < NFORMULAS; j++) cbform->AddEntry(form_list[j], j);
	cbform->Select(1);
	cbform->Resize(200, 30);
	cbform->Connect("Selected(Int_t)", "MyMainFrame", this, "SetOutOfDate()");
	hfs1->AddFrame(cbform, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs1, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs2 = new TGHorizontalFrame(vlay);
	TGLabel *lpro = new TGLabel(hfs2, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hfs2->AddFrame(lpro, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	tepro = new TGTextEntry(hfs2, new TGTextBuffer(), -1, GC16->GetGC(), font16);
	tepro->Resize(200, 30);
	tepro->Connect("TextChanged(const char *)", "MyMainFrame", this, "ProMod()");
	tepro->Connect("TabPressed()", "MyMainFrame", this, "ProApply()");
	tepro->Connect("ReturnPressed()", "MyMainFrame", this, "ProApply()");
	hfs2->AddFrame(tepro, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs2, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs3 = new TGHorizontalFrame(vlay);
	TGLabel *ltar = new TGLabel(hfs3, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hfs3->AddFrame(ltar, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	tetar = new TGTextEntry(hfs3, new TGTextBuffer(), -1, GC16->GetGC(), font16);
	tetar->Resize(200, 30);
	tetar->Connect("TextChanged(const char *)", "MyMainFrame", this, "AbsMod()");
	tetar->Connect("TabPressed()", "MyMainFrame", this, "AbsApply()");
	tetar->Connect("ReturnPressed()", "MyMainFrame", this, "AbsApply()");
	hfs3->AddFrame(tetar, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs3, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs4 = new TGHorizontalFrame(vlay);
	TGLabel *lstate = new TGLabel(hfs4, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hfs4->AddFrame(lstate, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbstate = new TGComboBox(hfs4);
	cbstate->AddEntry("solid", 0); cbstate->AddEntry("gas", 1);
	cbstate->Select(0);
	cbstate->Resize(200, 30);
	cbstate->Connect("Selected(Int_t)", "MyMainFrame", this, "GasSwitch(Int_t)");
	hfs4->AddFrame(cbstate, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs4, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs5 = new TGHorizontalFrame(vlay);
	TGLabel *lpre = new TGLabel(hfs5, "AAAAAA", GC16b->GetGC(), font16b);
	hfs5->AddFrame(lpre, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nepre = new TGNumberEntry(hfs5, 50, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	nepre->Resize(80, 30);
	nepre->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hfs5->AddFrame(nepre, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lpreu = new TGLabel(hfs5, "AAAAAAAA", GC16b->GetGC(), font16b);
	hfs5->AddFrame(lpreu, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs5, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs6 = new TGHorizontalFrame(vlay);
	TGLabel *ltem = new TGLabel(hfs6, "AAAAAA", GC16b->GetGC(), font16b);
	hfs6->AddFrame(ltem, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	netem = new TGNumberEntry(hfs6, 300, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	netem->Resize(80, 30);
	netem->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hfs6->AddFrame(netem, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *ltemu = new TGLabel(hfs6, "AAAAAAAA", GC16b->GetGC(), font16b);
	hfs6->AddFrame(ltemu, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs6, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	
	TGHorizontalFrame *hfs7 = new TGHorizontalFrame(vlay);
	TGLabel *lrho = new TGLabel(hfs7, "AAAAAA", GC16b->GetGC(), font16b);
	hfs7->AddFrame(lrho, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nerho = new TGNumberEntry(hfs7, 1000, 5, -1, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
	nerho->Resize(80, 30);
	nerho->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hfs7->AddFrame(nerho, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lrhou = new TGLabel(hfs7, "AAAAAAAA", GC16b->GetGC(), font16b);
	hfs7->AddFrame(lrhou, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfs7, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//END SYSTEM
	
	//BEGIN INPUT
	TGLabel *lmode = new TGLabel(vlay, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	vlay->AddFrame(lmode, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbmode = new TGComboBox(vlay);
	for(unsigned j = 0; j < 2; j++) cbmode->AddEntry(mode_list[j], j);
	cbmode->Select(0);
	cbmode->Resize(400, 30);
	cbmode->Connect("Selected(Int_t)", "MyMainFrame", this, "ModeSwitch(Int_t)");
	vlay->AddFrame(cbmode, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGHorizontalFrame *hfi1 = new TGHorizontalFrame(vlay);
	nein = new TGNumberEntry(hfi1, 0, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	nein->Resize(190, 30);
	nein->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hfi1->AddFrame(nein, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbin = new TGComboBox(hfi1);
	for(unsigned j = 0; j < NEUNIT; j++) cbin->AddEntry(eunit[j], j);
	cbin->Select(1);
	cbin->Resize(190, 30);
	cbin->Connect("Selected(Int_t)", "MyMainFrame", this, "SetOutOfDate()");
	hfi1->AddFrame(cbin, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfi1, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	TGLabel *lstrat = new TGLabel(vlay, "AAAAAAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	vlay->AddFrame(lstrat, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbstrat = new TGComboBox(vlay);
	for(unsigned j = 0; j < 4; j++) cbstrat->AddEntry(strat_list[j], j);
	cbstrat->Select(0);
	cbstrat->Resize(400, 30);
	cbstrat->Connect("Selected(Int_t)", "MyMainFrame", this, "SetOutOfDate()");
	vlay->AddFrame(cbstrat, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	//END INPUT
	
	//BEGIN BUTTONS
	tbcalc = new TGTextButton(vlay, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	tbcalc->SetFont(font16b);
	tbcalc->Connect("Clicked()", "MyMainFrame", this, "Draw()");
	vlay->AddFrame(tbcalc, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	tbsave = new TGTextButton(vlay, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	tbsave->SetFont(font16b);
// 	tbsave->Connect("Clicked()", "MyMainFrame", this, "Save()");
	vlay->AddFrame(tbsave, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGTextButton *tbexit = new TGTextButton(vlay, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "gApplication->Terminate(0)");
	tbexit->SetFont(font16b);
	vlay->AddFrame(tbexit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	//END BUTTONS
	
	hlay->AddFrame(vlay, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	//* vlay ends
	
	fMain->AddFrame(hlay, new TGLayoutHints(kLHintsExpandY|kLHintsExpandX));
	// hlay ends
	
	// Set a name to the main frame
	fMain->SetWindowName("Energy loss profile plotting utility");
	
	// Map all subwindows of main frame
	fMain->MapSubwindows();
	
	// Initialize the layout algorithm
	fMain->Resize(fMain->GetDefaultSize());
	
	// Map main frame
	fMain->MapWindow();
	
	lmode->SetText("Input mode:");
	lstrat->SetText("Punch-through strategy:");
	lform->SetText("Formula:");
	lpro->SetText("Projectile:");
	ltar->SetText("Absorber:");
	lstate->SetText("State:");
	lpre->SetText("  P =");   lpreu->SetText("mbar  ");
	ltem->SetText("  T =");   ltemu->SetText("K     ");
	lrho->SetText("rho =");   lrhou->SetText("mg/cm3");
	
	tbcalc->SetText("&Draw profile");
	tbsave->SetText("&Save (not implemented)");
	tbexit->SetText("&Exit");
	
	hD = hE = nullptr;
	sD = sE = nullptr;
	for(int j = 0; j < NLABS; j++) {
		pl[j] = nullptr;
	}
	for(int j = 0; j < NARWS; j++) {
		tar[j] = nullptr;
	}
	tldin = nullptr;
	
	up = true;
	mid = -1;
	
	ModeSwitch(cbmode->GetSelected());
	GasSwitch(cbstate->GetSelected());
	ProMod(); AbsMod();
}

MyMainFrame::~MyMainFrame() {
	fMain->Cleanup();
	delete fMain;
}

// void MyMainFrame::HandleMyCanvas(Int_t event,Int_t x,Int_t y,TObject *selected) {
// 	
// 	return;
// }

void MyMainFrame::ModeSwitch(int mode) {
	nein->SetNumber(0);
	cbin->RemoveAll();
	if(mode) {
		for(unsigned j = 0; j < NTUNIT; j++) cbin->AddEntry(tunit[j], j);
		cbin->Select(1);
		cbstrat->SetEnabled(kTRUE);
	}
	else {
		for(unsigned j = 0; j < NEUNIT; j++) cbin->AddEntry(eunit[j], j);
		cbin->Select(1);
		cbstrat->SetEnabled(kFALSE);
	}
	SetOutOfDate();
	return;
}

void MyMainFrame::GasSwitch(int state) {
	switch(state) {
		case 0:
			nepre->SetState(kFALSE);
			netem->SetState(kFALSE);
			nerho->SetState(kTRUE);
			break;
		case 1:
			nepre->SetState(kTRUE);
			netem->SetState(kTRUE);
			nerho->SetState(kFALSE);
	}
	SetOutOfDate();
	return;
}

void MyMainFrame::ProMod() {
	tepro->ChangeBackground(yello->GetPixel());
	fPro  = false;
	upPro = false;
	SetOutOfDate();
	return;
}

void MyMainFrame::ProApply() {
	std::vector< int > Z, W;
	std::vector< double > A;
	
	upPro = true;
	if(el.ParseFormula(tepro->GetText(), Z, W, A) < 0) goto bad;
	if(Z.size() != 1) goto bad;
	proZ = Z[0];
	proA = (int)(A[0] + 0.5);
	if(proZ < 1 || proZ > 118) goto bad;
	if(proA < 1 || proA > 300) goto bad;
	
	tepro->ChangeBackground(white->GetPixel());
	fPro = true;
	return;
	
	bad:
	tepro->ChangeBackground(red->GetPixel());
	fPro = false;
	return;
}

void MyMainFrame::AbsMod() {
	tetar->ChangeBackground(yello->GetPixel());
	fTar  = false;
	upTar = false;
	SetOutOfDate();
	return;
}

void MyMainFrame::AbsApply() {
	upTar = true;
	if((mid = el.AddAbsorber(tetar->GetText())) < 0) goto bad;
	
	bool isgas;
	double rho, P, T;
	if(el.GetAbsParam(mid, isgas, rho, P, T) < 0) goto bad;
	if(isgas) {
		cbstate->Select(1);
		if(P > 0) nepre->SetNumber(P);
		if(T > 0) netem->SetNumber(T);
		if(rho > 0) nerho->SetNumber(rho);
	}
	else {
		cbstate->Select(0);
		if(rho > 0) nerho->SetNumber(rho);
	}
	tetar->ChangeBackground(white->GetPixel());
	fTar = true;
	return;
	
	bad:
	tetar->ChangeBackground(red->GetPixel());
	fTar = false;
	return;
}

void MyMainFrame::Draw() {
	if(!upPro) ProApply();
	if(!fPro) return;
	
	if(!upTar) AbsApply();
	if(!fTar) return;
	
	double dedx, hx, x, dx, xmax, dmax, din;
	const int forsel[NFORMULAS] = {NIST, SRIM, BARBUI, SCHWALM, VEDALOSS};
	int form = forsel[cbform->GetSelected()];
	int mode = cbmode->GetSelected();
	int strat = cbstrat->GetSelected();
	TCanvas *fCanvas = fEcanvas->GetCanvas();
	
	bool isgas = false;
	if(cbstate->GetSelected() == 1) isgas = true;
	double P = nepre->GetNumber();
	double T = netem->GetNumber();
	double rho = nerho->GetNumber();
	el.SetAbsParam(mid, isgas, rho, P, T);
	if(el.GetAbsParam(mid, isgas, rho, P, T) < 0) goto bad;
	if(isgas && (rho > 0)) nerho->SetNumber(rho);
	
	
	//START OF CALCULATION (TIME MEASUREMENT)
	struct timeval t0, t1, td;
	gettimeofday(&t0, NULL);
	double E, thk, ept;
	if(mode) {
		thk = nein->GetNumber();
		ept = el.Compute(THK_TO_EPT, proZ, proA, mid, form, thk);
		if(ept < 0) goto bad;
		E = 1.1 * ept;
	}
	else {
		ept = E = nein->GetNumber();
		thk = el.Compute(EPT_TO_THK, proZ, proA, mid, form, ept);
		if(thk < 0) goto bad;
	}
	hx = thk * 1.1;
	
	//Mode 0 and mode 1 common part: profile calculation
	dedx = el.Getdedx(proZ, proA, mid, form, E);
	vX.clear(); vD.clear(); vE.clear();
	vX.push_back(0);
	vD.push_back(dedx);
	vE.push_back(E);
	x = 0; dx = 0;
	double ea, e2, d2;
	for(;;) {
		ea = E/(double)proA;
		e2 = (1 - RELSTEPE) * ea;
		if(ea - e2 < MINSTEPE) e2 = ea - MINSTEPE;
		else if(ea - e2 > MAXSTEPE) e2 = ea - MAXSTEPE;
		if(e2 < EMIN) {
			e2 = 0;
			d2 = 0;
			dx += el.mgcm2_to_um(E/dedx, rho);
		}
		else {
			e2 *= (double)proA;
			printf(UP);
			d2 = el.Getdedx(proZ, proA, mid, form, e2);
			if(d2 < 0) goto bad;
			dx += el.mgcm2_to_um(0.5 * (1./dedx + 1./d2) * (E - e2), rho);
		}
		if(dx > 0.01 * x) {
			x += dx;
			dx = 0;
		}
		
		dedx = d2;
		E    = e2;
		vX.push_back(x + dx);
		vD.push_back(d2);
		vE.push_back(e2);
		if(E < EMIN) break;
	}
	x += dx;
	if(sD != nullptr) sD->Delete();
	if(sE != nullptr) sE->Delete();
	sD = new TSpline3("dE/dx spline", vX.data(), vD.data(), vX.size());
	sE = new TSpline3("Residual energy spline", vX.data(), vE.data(), vX.size());
	
	double Xoff, Ein, Ept_max, Ept_50, Ept_end;
	//Bragg peak search
	double x5[5], d5[5];
	x5[0] = 0;   d5[0] = sD->Eval(0);
	x5[4] = x;   d5[4] = sD->Eval(x);
	x5[2] = x/2; d5[2] = sD->Eval(x/2);
	if(d5[2] < d5[0] || d5[2] < d5[4]) {
		printf(RED "Profile " NRM " Bad profile! [1]\n");
		goto bad;
	}
	xmax = -1; dmax = -1;
	for(;;) {
		x5[1] = (x5[0] + x5[2]) / 2.;
		d5[1] = sD->Eval(x5[1]);
		x5[3] = (x5[4] + x5[2]) / 2.;
		d5[3] = sD->Eval(x5[3]);
		int imax = 0;
		for(int i = 1; i < 4; i++) {
			if(d5[i] > d5[imax]) imax = i;
		}
		if(imax < 1) {
			printf(RED "Profile " NRM " Bad profile! [2]\n");
			goto bad;
		}
		xmax = x5[imax];
		dmax = d5[imax];
		if((dmax - 0.5 * (d5[imax-1] + d5[imax+1])) / dmax < 0.01) break;
		
		x5[0] = x5[imax - 1]; d5[0] = d5[imax - 1];
		x5[4] = x5[imax + 1]; d5[4] = d5[imax + 1];
		x5[2] = x5[imax];     d5[2] = d5[imax];
	}
	if(xmax < 0 || dmax < 0) {
		printf(RED "Profile " NRM " Bad profile! [3]\n");
		goto bad;
	}
	
	Xoff = 0;
	if(mode == 0) {
		Ein = ept;
		goto draw;
	}
	Ein = -1;
	//Search of maximum energy release (I use only the first 3 elements of x5 and d5!)
	x5[0] = xmax - thk;
	if(x5[0] < 0) x5[0] = 0;
	x5[2] = x - thk;
	if(x5[2] <= x5[0]) {
		printf(RED "Profile " NRM " Bad profile! [4]\n");
		goto bad;
	}
	for(;;) {
		x5[1] = (x5[0] + x5[2]) / 2.;
		d5[1] = sD->Eval(x5[1] + thk) - sD->Eval(x5[1]);
		if(fabs(d5[1]) / dmax < 0.001) break;
		if(d5[1] > 0) x5[0] = x5[1];
		else          x5[2] = x5[1];
	}
	if(strat == 3) {
		Ein  = sE->Eval(x5[1]);
		Xoff = x5[1];
	}
	Ept_max = sE->Eval(x5[1]) - sE->Eval(x5[1] + thk);
	
	//Search of p-t energy (50% of Bragg peak) (I use only the first 3 elements of x5 and d5!)
	x5[0] = xmax - thk;
	if(x5[0] < 0) x5[0] = 0;
	x5[2] = x - thk;
	for(;;) {
		x5[1] = (x5[0] + x5[2]) / 2.;
		d5[1] = sD->Eval(x5[1] + thk) - 0.5 * dmax;
		if(fabs(d5[1]) / dmax < 0.001) break;
		if(d5[1] > 0) x5[0] = x5[1];
		else          x5[2] = x5[1];
	}
	if(strat == 1) {
		Ein  = sE->Eval(x5[1]);
		Xoff = x5[1];
	}
	Ept_50 = sE->Eval(x5[1]) - sE->Eval(x5[1] + thk);
	
	//Search of p-t energy (end of range)
	if(strat == 2) {
		Ein  = sE->Eval(x - thk);
		Xoff = x - thk;
	}
	Ept_end = sE->Eval(x - thk);
	
	//Search of X offset using range table (I use only the first 3 elements of x5 and d5!)
	x5[0] = 0; x5[2] = xmax;
	for(;;) {
		x5[1] = (x5[0] + x5[2]) / 2.;
		d5[1] = sE->Eval(x5[1]) - ept;
		if(fabs(d5[1]) / ept < 0.001) break;
		if(d5[1] > 0) x5[0] = x5[1];
		else          x5[2] = x5[1];
	}
	if(strat == 0) {
		Ein  = ept;
		Xoff = x5[1];
	}
	if(Ein < 0) {
		printf(RED "Profile " NRM " Bad profile! [5]\n");
		goto bad;
	}
	printf(GRN "Profile " NRM " Punch through energy calculated with different range distribution cuts:\n");
	printf("           Value from range tables:   %9.3lf MeV\n", ept);
	printf("           Cut at 50%% of Bragg peak:  %9.3lf MeV\n", Ept_50);
	printf("           End of range distribution: %9.3lf MeV\n", Ept_end);
	printf("           Maximum released energy:   %9.3lf MeV\n", Ept_max);
	
	draw:
	gettimeofday(&t1, NULL);
	timersub(&t1, &t0, &td);
	printf(GRN "Profile " NRM " energy loss profiling took %2ld.%06ld s\n", td.tv_sec, td.tv_usec);
	//I don't directly plot TSplines (I tried!) because ROOT rasters them with a broad binning (not nice to see...)
	if(hD != nullptr) hD->Delete();
	if(hE != nullptr) hE->Delete();
	hD = new TH1D("hD", "", 10000, 0, hx);
	hE = new TH1D("hE", "", 10000, 0, hx);
	for(int j = 1; j <= hD->GetNbinsX(); j++) {
		double xx = hD->GetXaxis()->GetBinCenter(j);
		if(xx + Xoff > x) {
			hD->SetBinContent(j, 0);
			hE->SetBinContent(j, 0);
		}
		else {
			hD->SetBinContent(j, sD->Eval(xx + Xoff) / dmax);
			hE->SetBinContent(j, sE->Eval(xx + Xoff) / Ein);
		}
	}
	fCanvas->GetPad(0)->SetGridx(kTRUE);
	fCanvas->GetPad(0)->SetGridy(kTRUE);
	fCanvas->SetMargin(0.05, 0.01, 0.10, 0.01);
	fCanvas->cd();
	hD->SetStats(kFALSE);
	hD->GetXaxis()->SetTitle("Thickness [#mum]");
	hD->GetXaxis()->SetTitleSize(0.05);
	hD->GetXaxis()->SetTitleOffset(0.98);
	hD->GetXaxis()->SetLabelSize(0.05);
	hD->GetXaxis()->SetNdivisions(508);
	hD->GetYaxis()->SetTitleSize(0.05);
	hD->GetYaxis()->SetTitleOffset(0.83);
	hD->GetYaxis()->SetLabelSize(0.05);
	hD->GetYaxis()->SetNdivisions(508);
	hD->GetYaxis()->SetRangeUser(0,1.1);
	hD->SetLineWidth(2);
	hD->SetLineColor(kRed + 2);
	hD->SetFillColor(kRed);
	hD->SetFillStyle(3001);
	hD->Draw();
	hE->SetLineWidth(2);
	hE->SetLineColor(kGreen + 2);
	hE->SetFillColor(kGreen);
	hE->SetFillStyle(3003);
	hE->Draw("same");
	
	//Initial dE/dx line
	din = sD->Eval(Xoff);
	if(tldin) tldin->Delete();
	tldin = new TLine(0, din / dmax, hx, din / dmax);
	tldin->SetLineStyle(9);
	tldin->SetLineWidth(3);
	tldin->SetLineColor(kRed + 2);
	tldin->Draw();
	
	double posx, pos, pos1, pos2;
	char fmtstr[STRMAXL];
	
	//***** LABELS
	for(int j = 0; j < NLABS; j++) {
		if(pl[j] != nullptr) pl[j]->Delete();
	}
	//Eres label
	pos1 = sE->Eval(0.27 * hx + Xoff) / Ein;
	pos2 = sD->Eval(0.27 * hx + Xoff) / dmax;
	if(pos1 - 0.18 > pos2) pos = pos1 - 0.17;
	else {
		pos1 = sE->Eval(0.27 * hx + Xoff) / Ein;
		pos2 = sD->Eval(0.02 * hx + Xoff) / dmax;
		pos = ((pos1 > pos2) ? pos2 : pos1) - 0.17;
	}
	pl[0] = new TPaveText(0.02 * hx, pos,  0.27 * hx, pos + 0.16, "br");
	pl[0]->AddText("Residual energy");
	pl[0]->AddText("normalized to E_{in}");
	pl[0]->SetFillColor(kGreen - 10);
	
	//dE/dx label
	posx = (0.98 * (xmax - Xoff)) / hx;
	pos1 = sE->Eval((posx - 0.25) * hx + Xoff) / Ein;
	pos2 = sD->Eval(posx * hx + Xoff) / dmax;
	if(pos1 > pos2) {
		posx += 0.01;
		pos = pos1 + 0.02;
	}
	else if(pos2 > 0.83) {
		posx = (xmax + thk - Xoff) / (2. * hx);
		pos1 = sD->Eval(posx * hx + Xoff) / dmax;
		pos2 = sD->Eval((posx - 0.26) * hx + Xoff) / dmax;
		pos = ((pos1 > pos2) ? pos2 : pos1) - 0.17;
		posx -= 0.01;
	}
	else {
		posx -= 0.01;
		pos = pos2 + 0.01;
	}
	pl[1] = new TPaveText((posx - 0.25) * hx, pos, posx * hx, pos + 0.16, "br");
	pl[1]->AddText("Stopping power");
	pl[1]->AddText("n. Bragg Peak");
	pl[1]->SetFillColor(kRed - 10);
	
	printval(Ein * 1.e6, "eV", fmtstr);
	pl[2] = new TPaveText(0.02 * hx, 1.02, 0.16 * hx, 1.1, "");
	pl[2]->AddText(fmtstr);
	pl[2]->SetTextSize(0.04);
	pl[2]->SetTextColor(kGreen + 2);
	pl[2]->SetFillStyle(0);
	pl[2]->SetBorderSize(0);
	
	printval(dmax * 1.e6, "eV/(mg/cm^{2})", fmtstr);
	pl[3] = new TPaveText(0.65 * hx, 1.02, 0.95 * hx, 1.1, "");
	pl[3]->AddText(fmtstr);
	pl[3]->SetTextSize(0.04);
	pl[3]->SetTextColor(kRed + 2);
	pl[3]->SetFillStyle(0);
	pl[3]->SetBorderSize(0);
	
// 	printval(din * 1.e6, "#frac{eV}{(mg/cm2)}", fmtstr);
// 	pos = din / dmax - 0.09;
// 	if(pos < 0.01) pos = sD->Eval(0.4 * hx + Xoff) / dmax + 0.01;
// 	pl[2] = new TPaveLabel(0.02 * hx, pos, 0.4 * hx, pos + 0.08, fmtstr, "br");
// 	pl[2]->SetFillColor(kRed - 10);
	
	for(int j = 0; j < NLABS; j++) pl[j]->Draw();
	
	//***** ARROWS
	for(int j = 0; j < NARWS; j++) {
		if(tar[j] != nullptr) tar[j]->Delete();
	}
	
	tar[0] = new TArrow(0.022 * hx, 1.04, 0.002 * hx, 1.01, 0.01, "|>");
	tar[0]->SetLineWidth(3);
	tar[0]->SetLineColor(kGreen + 2);
	tar[0]->SetFillColor(kGreen + 2);
	tar[1] = new TArrow(xmax - Xoff, 1.04, xmax - Xoff, 1.01, 0.01, "|>");
	tar[1]->SetLineWidth(3);
	tar[1]->SetLineColor(kRed + 2);
	tar[1]->SetFillColor(kRed + 2);
	
	for(int j = 0; j < NARWS; j++) tar[j]->Draw();
	fCanvas->Update();
	
	gettimeofday(&t0, NULL);
	timersub(&t0, &t1, &td);
	printf(GRN "Profile " NRM "       profile drawing took %2ld.%06ld s\n", td.tv_sec, td.tv_usec);
	
	up = true;
	tbcalc->ChangeBackground(green->GetPixel());
	return;
	
	bad:
	up = true;
	tbcalc->ChangeBackground(red->GetPixel());
	return;
}

void MyMainFrame::SetOutOfDate() {
	if(!up) return;
	up = false;
	tbcalc->ChangeBackground(yello->GetPixel());
	tbsave->SetEnabled(kFALSE);
	return;
}

int main(int argc, char **argv) {
	TApplication Profile("App", &argc, argv);
	MyMainFrame *gui = new MyMainFrame(gClient->GetRoot(), WINDOWX, WINDOWY);
	Profile.Run();
	delete gui;
	return 0;
}
