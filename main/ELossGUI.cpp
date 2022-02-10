/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 21/09/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#include "ELossGUI.h"

#define NEUNIT 3
#define NTUNIT 5

static const char *mode_list[NMODES] = {"From Ein and thickness to Elost and Eres", "From Eres and thickness to Ein and Elost", "From Elost and thickness to Ein and Eres", "From Ein and Eres to thickness", "From thickness to punch-through energy", "From punch-through energy to thickness"};
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
	
	//* vlay starts
	TGVerticalFrame *vlay = new TGVerticalFrame(fMain);
	
	//** hf01 starts
	TGHorizontalFrame *hf01 = new TGHorizontalFrame(vlay);
	TGLabel *lmode = new TGLabel(hf01, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf01->AddFrame(lmode, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbmode = new TGComboBox(hf01);
	for(unsigned j = 0; j < NMODES; j++) cbmode->AddEntry(mode_list[j], j);
	cbmode->Select(0);
	cbmode->Resize(500, 30);
	cbmode->Connect("Selected(Int_t)", "MyMainFrame", this, "ModeSwitch(Int_t)");
	hf01->AddFrame(cbmode, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf01, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf01 ends
	
	//** hf02 starts
	TGHorizontalFrame *hf02 = new TGHorizontalFrame(vlay);
	TGLabel *lpro = new TGLabel(hf02, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf02->AddFrame(lpro, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	tepro = new TGTextEntry(hf02, new TGTextBuffer(), -1, GC16->GetGC(), font16);
	tepro->Resize(150, 30);
	tepro->Connect("TextChanged(const char *)", "MyMainFrame", this, "ProMod()");
	tepro->Connect("TabPressed()", "MyMainFrame", this, "ProApply()");
	tepro->Connect("ReturnPressed()", "MyMainFrame", this, "ProApply()");
	hf02->AddFrame(tepro, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lform = new TGLabel(hf02, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf02->AddFrame(lform, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbform = new TGComboBox(hf02);
	for(unsigned j = 0; j < NFORMULAS; j++) cbform->AddEntry(form_list[j], j);
	cbform->Select(0);
	cbform->Resize(100, 30);
	cbform->Connect("Selected(Int_t)", "MyMainFrame", this, "FormSwitch()");
	hf02->AddFrame(cbform, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf02, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf02 ends
	
	//** hf03 starts
	TGHorizontalFrame *hf03 = new TGHorizontalFrame(vlay);
	lin1 = new TGLabel(hf03, "AAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf03->AddFrame(lin1, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nein1 = new TGNumberEntry(hf03, 0, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	nein1->Resize(80, 30);
	nein1->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hf03->AddFrame(nein1, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbin1 = new TGComboBox(hf03);
	for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
	cbin1->Select(1);
	cbin1->Resize(100, 30);
	cbin1->Connect("Selected(Int_t)", "MyMainFrame", this, "SetOutOfDate()");
	hf03->AddFrame(cbin1, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	lin2 = new TGLabel(hf03, "AAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf03->AddFrame(lin2, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nein2 = new TGNumberEntry(hf03, 0, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	nein2->Resize(80, 30);
	nein2->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hf03->AddFrame(nein2, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbin2 = new TGComboBox(hf03);
	for(unsigned j = 0; j < NTUNIT; j++) cbin2->AddEntry(tunit[j], j);
	cbin2->Select(1);
	cbin2->Resize(100, 30);
	cbin2->Connect("Selected(Int_t)", "MyMainFrame", this, "SetOutOfDate()");
	hf03->AddFrame(cbin2, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf03, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf03 ends
	
	//** hf05 starts
	TGHorizontalFrame *hf05 = new TGHorizontalFrame(vlay);
	TGLabel *ltar = new TGLabel(hf05, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf05->AddFrame(ltar, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	tetar = new TGTextEntry(hf05, new TGTextBuffer(), -1, GC16->GetGC(), font16);
	tetar->Resize(150, 30);
	tetar->Connect("TextChanged(const char *)", "MyMainFrame", this, "AbsMod()");
	tetar->Connect("TabPressed()", "MyMainFrame", this, "AbsApply()");
	tetar->Connect("ReturnPressed()", "MyMainFrame", this, "AbsApply()");
	hf05->AddFrame(tetar, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lstate = new TGLabel(hf05, "AAAAAAAAAAAAAAA", GC16b->GetGC(), font16b);
	hf05->AddFrame(lstate, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	cbstate = new TGComboBox(hf05);
	cbstate->AddEntry("solid", 0); cbstate->AddEntry("gas", 1);
	cbstate->Select(0);
	cbstate->Resize(100, 30);
	cbstate->Connect("Selected(Int_t)", "MyMainFrame", this, "GasSwitch(Int_t)");
	hf05->AddFrame(cbstate, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf05, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf05 ends
	
	//** hf06 starts
	TGHorizontalFrame *hf06 = new TGHorizontalFrame(vlay);
	TGLabel *lpre = new TGLabel(hf06, "AAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(lpre, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nepre = new TGNumberEntry(hf06, 50, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	nepre->Resize(80, 30);
	nepre->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hf06->AddFrame(nepre, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lpreu = new TGLabel(hf06, "AAAAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(lpreu, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *ltem = new TGLabel(hf06, "AAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(ltem, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	netem = new TGNumberEntry(hf06, 300, 8, -1, TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEANonNegative);
	netem->Resize(80, 30);
	netem->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hf06->AddFrame(netem, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *ltemu = new TGLabel(hf06, "AAAAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(ltemu, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lrho = new TGLabel(hf06, "AAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(lrho, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	nerho = new TGNumberEntry(hf06, 1000, 5, -1, TGNumberFormat::kNESReal, TGNumberFormat::kNEANonNegative);
	nerho->Resize(80, 30);
	nerho->GetNumberEntry()->Connect("TextChanged(const char *)", "MyMainFrame", this, "SetOutOfDate()");
	hf06->AddFrame(nerho, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	TGLabel *lrhou = new TGLabel(hf06, "AAAAAAAA", GC16b->GetGC(), font16b);
	hf06->AddFrame(lrhou, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf06, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf06 ends
	
	//** hf07 starts
	TGHorizontalFrame *hf07 = new TGHorizontalFrame(vlay);
	tbcalc = new TGTextButton(hf07, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	tbcalc->SetFont(font16b);
	tbcalc->Connect("Clicked()", "MyMainFrame", this, "Calculate()");
	hf07->AddFrame(tbcalc, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hf07, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hf07 ends
	
	//** hf08 starts
	TGHorizontalFrame *hf08=new TGHorizontalFrame(vlay);
	TGLabel *lein = new TGLabel(hf08,"AAAAAAAAAAAA",GC16b->GetGC(),font16b);
	hf08->AddFrame(lein, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	TGLabel *lthk = new TGLabel(hf08,"AAAAAAAAAAAA",GC16b->GetGC(),font16b);
	hf08->AddFrame(lthk, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	TGLabel *llst = new TGLabel(hf08,"AAAAAAAAAAAA",GC16b->GetGC(),font16b);
	hf08->AddFrame(llst, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	TGLabel *lres = new TGLabel(hf08,"AAAAAAAAAAAA",GC16b->GetGC(),font16b);
	hf08->AddFrame(lres, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	vlay->AddFrame(hf08, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY,2,2,2,2));
	//** hf08 ends
	
	//** hf09 starts
	TGHorizontalFrame *hf09=new TGHorizontalFrame(vlay);
	for(unsigned j = 0; j < 4; j++) {
		lout[0][j] = new TGLabel(hf09,"AAAAAAAAAAAA",GC16->GetGC(),font16);
		hf09->AddFrame(lout[0][j], new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	}
	vlay->AddFrame(hf09, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY,2,2,2,2));
	//** hf09 ends
	
	//** hf10 starts
	TGHorizontalFrame *hf10=new TGHorizontalFrame(vlay);
	for(unsigned j = 0; j < 4; j++) {
		lout[1][j] = new TGLabel(hf10,"AAAAAAAAAAAA",GC16->GetGC(),font16);
		hf10->AddFrame(lout[1][j], new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	}
	vlay->AddFrame(hf10, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY,2,2,2,2));
	//** hf10 ends
	
	//** hfx starts
	TGHorizontalFrame *hfx = new TGHorizontalFrame(vlay);
	TGTextButton *tbexit = new TGTextButton(hfx,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "gApplication->Terminate(0)");
	tbexit->SetFont(font16b);
	hfx->AddFrame(tbexit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	vlay->AddFrame(hfx, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsCenterY, 2, 2, 2, 2));
	//** hfx ends
	
	fMain->AddFrame(vlay, new TGLayoutHints(kLHintsCenterX|kLHintsCenterY|kLHintsExpandY|kLHintsExpandX, 2, 2, 2, 2));
	//* vlay ends
	
	// Set a name to the main frame
	fMain->SetWindowName("Energy loss calculator");
	
	// Map all subwindows of main frame
	fMain->MapSubwindows();
	
	// Initialize the layout algorithm
	fMain->Resize(fMain->GetDefaultSize());
	
	// Map main frame
	fMain->MapWindow();
	
	lmode->SetText("Working mode");
	lform->SetText("Formula");
	lpro->SetText("Projectile");
	ltar->SetText("Absorber");
	lstate->SetText("State");
	
	lpre->SetText("  P =");   lpreu->SetText("mbar  ");
	ltem->SetText("  T =");   ltemu->SetText("K     ");
	lrho->SetText("rho ="); lrhou->SetText("mg/cm3");
	
	lein->SetText("Initial E");
	lthk->SetText("Abs. Thickness");
	llst->SetText("Lost E");
	lres->SetText("Residual E");
	
	tbcalc->SetText("&Calculate");
	tbexit->SetText("&Exit");
	
	for(unsigned i = 0; i < 2; i++) {
		for(unsigned j = 0; j < 4; j++) {
			lout[i][j]->SetText("-");
		}
	}
	
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

void MyMainFrame::ModeSwitch(int mode) {
	nein1->SetNumber(0);
	nein2->SetNumber(0);
	cbin1->RemoveAll();
	if(mode > 3) {
		nein2->SetState(kFALSE);
		lin2->SetText("");
		cbin2->SetEnabled(kFALSE);
	}
	else {
		nein2->SetState(kTRUE);
		cbin2->SetEnabled(kTRUE);
		cbin2->RemoveAll();
	}
	switch(mode) {
		case 0:
			lin1->SetText("Initial E");
			for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
			cbin1->Select(1);
			lin2->SetText("Abs. thickness");
			for(unsigned j = 0; j < NTUNIT; j++) cbin2->AddEntry(tunit[j], j);
			cbin2->Select(1);
			break;
		case 1:
			lin1->SetText("Residual E");
			for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
			cbin1->Select(1);
			lin2->SetText("Abs. thickness");
			for(unsigned j = 0; j < NTUNIT; j++) cbin2->AddEntry(tunit[j], j);
			cbin2->Select(1);
			break;
		case 2:
			lin1->SetText("Lost E");
			for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
			cbin1->Select(1);
			lin2->SetText("Abs. thickness");
			for(unsigned j = 0; j < NTUNIT; j++) cbin2->AddEntry(tunit[j], j);
			cbin2->Select(1);
			break;
		case 3:
			lin1->SetText("Initial E");
			for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
			cbin1->Select(1);
			lin2->SetText("Residual E");
			for(unsigned j = 0; j < NTUNIT; j++) cbin2->AddEntry(tunit[j], j);
			cbin2->Select(1);
			break;
		case 4:
			lin1->SetText("Abs. thickness");
			for(unsigned j = 0; j < NTUNIT; j++) cbin1->AddEntry(tunit[j], j);
			cbin1->Select(1);
			break;
		case 5:
			lin1->SetText("P.-T. energy");
			for(unsigned j = 0; j < NEUNIT; j++) cbin1->AddEntry(eunit[j], j);
			cbin1->Select(1);
	}
	SetOutOfDate();
	return;
}

void MyMainFrame::FormSwitch() {
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

void MyMainFrame::Calculate() {
	char fmtstr[STRMAXL];
	double in1, in2, out, outv[4];
	int mode;
	const int forsel[NFORMULAS] = {NIST, SRIM, BARBUI, SCHWALM, VEDALOSS};
	
	if(!upPro) ProApply();
	if(!fPro) return;
	
	if(!upTar) AbsApply();
	if(!fTar) return;
	
	bool isgas = false;
	if(cbstate->GetSelected() == 1) isgas = true;
	double P = nepre->GetNumber();
	double T = netem->GetNumber();
	double rho = nerho->GetNumber();
	el.SetAbsParam(mid, isgas, rho, P, T);
	if(el.GetAbsParam(mid, isgas, rho, P, T) < 0) goto bad;
	if(isgas && (rho > 0)) nerho->SetNumber(rho);
	
	in1 = nein1->GetNumber();
	in2 = nein2->GetNumber();
	
	mode = cbmode->GetSelected();
	if(mode == 4)      in1 = tconv(cbin1->GetSelected(), in1, rho);
	else               in1 = econv(cbin1->GetSelected(), in1, proA);
	if(mode < 3)       in2 = tconv(cbin2->GetSelected(), in2, rho);
	else if(mode == 3) in2 = econv(cbin2->GetSelected(), in2, proA);
	
	struct timeval t0, t1, td;
	gettimeofday(&t0, NULL);
	out = el.Compute(mode, proZ, proA, mid, forsel[cbform->GetSelected()], in1, in2);
	gettimeofday(&t1, NULL);
	timersub(&t1, &t0, &td);
	printf(GRN "ELossGUI" NRM " energy loss calculation took %2ld.%06ld s\n", td.tv_sec, td.tv_usec);
	if(out < 0) goto bad;
	
	switch(mode) {
		case 0:
			outv[0] = in1;
			outv[1] = in2;
			outv[2] = in1 - out;
			outv[3] = out;
			break;
		case 1:
			outv[0] = out;
			outv[1] = in2;
			outv[2] = out - in1;
			outv[3] = in1;
			break;
		case 2:
			outv[0] = out;
			outv[1] = in2;
			outv[2] = in1;
			outv[3] = out - in1;
			break;
		case 3:
			outv[0] = in1;
			outv[1] = out;
			outv[2] = in1 - in2;
			outv[3] = in2;
			break;
		case 4:
			outv[0] = out;
			outv[1] = in1;
			outv[2] = out;
			outv[3] = 0;
			break;
		case 5:
			outv[0] = in1;
			outv[1] = out;
			outv[2] = in1;
			outv[3] = 0;
	}
	
	printval(outv[0] * 1.e6, "eV", fmtstr);
	lout[0][0]->SetText(fmtstr);
	printval(outv[1] * 1.e-6, "m", fmtstr);
	lout[0][1]->SetText(fmtstr);
	printval(outv[2] * 1.e6, "eV", fmtstr);
	lout[0][2]->SetText(fmtstr);
	if(outv[3] >= EMIN) {
		printval(outv[3] * 1.e6, "eV", fmtstr);
		lout[0][3]->SetText(fmtstr);
		lout[0][3]->SetTextColor(black);
	}
	else {
		lout[0][3]->SetText("-");
		lout[0][3]->SetTextColor(gray);
	}
	
	printval(el.MeV_to_AMeV(outv[0], proA) * 1.e6, "eV/u",  fmtstr);
	lout[1][0]->SetText(fmtstr);
	printval(el.um_to_mgcm2(outv[1], rho) * 1.e-3, "g/cm2", fmtstr);
	lout[1][1]->SetText(fmtstr);
	printval(el.MeV_to_AMeV(outv[2], proA) * 1.e6, "eV/u",  fmtstr);
	lout[1][2]->SetText(fmtstr);
	if(outv[3] >= EMIN) {
		printval(el.MeV_to_AMeV(outv[3], proA) * 1.e6, "eV/u",  fmtstr);
		lout[1][3]->SetText(fmtstr);
		lout[1][3]->SetTextColor(black);
	}
	else {
		lout[1][3]->SetText("-");
		lout[1][3]->SetTextColor(gray);
	}
	
	tbcalc->ChangeBackground(green->GetPixel());
	for(unsigned i = 0; i < 2; i++) {
		for(unsigned j = 0; j < 3; j++) {
			lout[i][j]->SetTextColor(black);
		}
	}
	up = true;
	return;
	
	bad:
	up = true;
	tbcalc->ChangeBackground(red->GetPixel());
	for(unsigned i = 0; i < 2; i++) {
		for(unsigned j = 0; j < 4; j++) {
			lout[i][j]->SetTextColor(gray);
		}
	}
	return;
}

void MyMainFrame::SetOutOfDate() {
	if(!up) return;
	up = false;
	tbcalc->ChangeBackground(yello->GetPixel());
	for(unsigned i = 0; i < 2; i++) {
		for(unsigned j = 0; j < 4; j++) {
			lout[i][j]->SetTextColor(gray);
		}
	}
	return;
}

int main(int argc, char **argv) {
	TApplication ELossGUI("App", &argc, argv);
	MyMainFrame *gui = new MyMainFrame(gClient->GetRoot(), WINDOWX, WINDOWY);
	ELossGUI.Run();
	delete gui;
	return 0;
}
