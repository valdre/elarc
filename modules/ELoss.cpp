/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 15/09/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/

#include "ELoss.h"

int chartype(char c) {
	if((c>='0')&&(c<='9')) return 0;
	if((c>='A')&&(c<='Z')) return 1;
	if((c>='a')&&(c<='z')) return 2;
	return -1;
}

ELoss::ELoss() {
	Mcnt = 0;
	elemDB.clear(); compDB.clear();
	mateLS.clear(); mateID.clear();
	tables.clear();
	
	LoadDB();
	vedaDB = false;
}

ELoss::~ELoss() {
	
}


// *************** DB funcions

void ELoss::LoadDB() {
	std::vector< std::vector< std::string > > arg;
	
	//Reading element names, atomic numbers, masses and densities
	struct element nel;
	int Z, lastZ = 0;
	ReadFile("db/elements.txt", arg);
	for(unsigned i=0; i<arg.size(); i++) {
		if(arg[i].size() >= 4) {
			Z = atoi(arg[i][0].c_str());
			if(Z>0 && Z<=118) {
				nel.Name = arg[i][1];
				nel.M = atof(arg[i][2].c_str());
				
				if(arg[i][3] == "gas") nel.rho = 0;
				else if(arg[i][3] == "na") nel.rho = -1;
				else nel.rho = atof(arg[i][3].c_str());
				
				if(Z == lastZ+1) elemDB.push_back(nel);
				else if(Z <= lastZ) elemDB[Z-1] = nel;
				else elemDB.resize(Z, nel);
				
				lastZ = Z;
			}
		}
		for(unsigned j=0; j<arg[i].size(); j++) {
			arg[i][j].clear();
		}
		arg[i].clear();
	}
	arg.clear();
	
	//Reading compound names, formulas and densities
	struct compound nco;
	ReadFile("db/compounds.txt", arg);
	for(unsigned i=0; i<arg.size(); i++) {
		if(arg[i].size() >= 3) {
			nco.Name = arg[i][0];
			nco.Form = arg[i][1];
			
			if(arg[i][2] == "gas") nco.rho = 0;
			else if(arg[i][2] == "na") nco.rho = -1;
			else nco.rho = atof(arg[i][2].c_str());
			
			compDB.push_back(nco);
		}
		for(unsigned j=0; j<arg[i].size(); j++) {
			arg[i][j].clear();
		}
		arg[i].clear();
	}
	arg.clear();
	return;
}

int ELoss::ReadFile(const char *fn, std::vector< std::vector< std::string > > &arg) {
	//Try to open DB file
	FILE *f = fopen(fn, "r");
	if(f == NULL) {
		printf(RED "ReadFile" NRM " %s not found\n", fn);
		return -1;
	}
	printf(GRN "ReadFile" NRM " reading from %s...\n", fn);
	
	//Parse DB file
	int row = 0, pos;
	bool space;
	char buff[STRMAXL], str[STRMAXL];
	for(;;) {
		if(fgets(buff, STRMAXL, f) == NULL) break;
		row++;
		if(strlen(buff) >= (STRMAXL - 1)) printf(YEL "ReadFile" NRM " very long string at row %d not properly read!\n", row);
		std::vector< std::string > vStr;
		pos   = 0;
		space = true;
		for(unsigned i=0; i<strlen(buff); i++) {
			if((buff[i] == '#') || (buff[i] < (char)32)) break; //comment, newline or strange char
			if( buff[i] == ' ') { //space to separate read elements
				if(space == false) {
					str[pos]='\0';
					pos=0;
					std::string Str(str);
					vStr.push_back(Str);
					space = true;
				}
				continue;
			}
			//good stuff, no space
			str[pos++] = buff[i];
			if(space) space = false;
		}
		if(pos > 0) {
			str[pos] = '\0';
			std::string Str(str);
			vStr.push_back(Str);
		}
		if(vStr.size() > 0) arg.push_back(vStr);
	}
	fclose(f);
	return 0;
}

// *************** Material functions
//Adds a new material: usually rho, P and T are not needed, but they can be provided in order to override the DB values or if the compound is not in the DB
//rho=0 => force gas
int ELoss::AddAbsorber(const char *name, const double rho, const double P, const double T) {
	printf(BLD "AddAbs  " NRM " adding absorber %s...\n", name);
	for(unsigned j = 0; j < mateLS.size(); j++) {
		if(mateLS[j].Name == name) {
			printf(BLD "AddAbs  " NRM " absorber %s already loaded...\n", name);
			if(rho > 0) mateLS[j].rho = rho;
			if(P > 0) mateLS[j].P = P;
			if(T > 0) mateLS[j].T = T;
			return mateLS[j].mid;
		}
	}
	
	struct material Abs;
	if(SetAbsorber(Abs, name, rho, P, T) < 0) return -1;
	
	Abs.mid = Mcnt++;
	mateLS.push_back(Abs);
	mateID.resize(Abs.mid + 1, -1);
	mateID[Abs.mid] = (int)(mateLS.size()) - 1;
	return Abs.mid;
}

int ELoss::SetAbsorber(struct material &Abs, const char *name, const double rho, const double P, const double T) {
	//Assign name
	Abs.Name = name;
	//Search name or formula in DB
	bool inDB = false;
	for(unsigned i = 0; i < compDB.size(); i++) {
		if(compDB[i].Name == name || compDB[i].Form == name) {
			printf(BLD "AddAbs  " NRM " absorber found in compound DB: %s\n", compDB[i].Form.c_str());
			Abs.Form = compDB[i].Form;
			if(rho < 0) {
				if(compDB[i].rho < 0) {
					printf(YEL "AddAbs  " NRM " density value not provided and missing from DB, assuming 1000 mg/cm3\n");
					Abs.rho = 1000.;
				}
				else {
					Abs.rho = compDB[i].rho;
					if(compDB[i].rho > 0) printf(BLD "AddAbs  " NRM " solid absorber: rho = %5.0f mg/cm^3 (from DB)\n", compDB[i].rho);
					else printf(BLD "AddAbs  " NRM " gas absorber (from DB)\n");
				}
			}
			else {
				Abs.rho = rho;
				if(rho > 0) printf(BLD "AddAbs  " NRM " solid absorber: rho = %5.0f mg/cm^3 (provided)\n", rho);
				else printf(BLD "AddAbs  " NRM " gas absorber (provided)\n");
			}
			
			inDB = true;
			break;
		}
	}
	//If not in DB, I assume that name is the formula
	if(!inDB) Abs.Form = Abs.Name;
	
	char form[STRMAXL];
	strcpy(form, Abs.Form.c_str());
	if(ParseFormula(form, Abs.Z, Abs.W, Abs.A) < 0) return -1;
	
	if(Abs.Z.size() == 1) {
		int i = Abs.Z[0] - 1;
		if(rho < 0) {
			if(elemDB[i].rho < 0) {
				printf(RED "AddAbs  " NRM " density value not provided and missing from DB, assuming 1000 mg/cm3\n");
				Abs.rho = 1000.;
			}
			else {
				Abs.rho = elemDB[i].rho;
				if(elemDB[i].rho > 0) printf(BLD "AddAbs  " NRM " solid absorber: rho = %5.0f mg/cm^3 (from DB)\n", elemDB[i].rho);
				else printf(BLD "AddAbs  " NRM " gas absorber (from DB)\n");
			}
		}
		else {
			Abs.rho = rho;
			if(rho > 0) printf(BLD "AddAbs  " NRM " solid absorber: rho = %5.0f mg/cm^3 (provided)\n", rho);
			else printf(BLD "AddAbs  " NRM " gas absorber (provided)\n");
		}
	}
	else if(Abs.rho < 0) {
		printf(RED "AddAbs  " NRM " compound is not in the DB and density value was not provided\n");
		return -1;
	}
	
	//Assigning gas pressure and temperature
	if(Abs.rho == 0) {
		Abs.isgas = true;
		if(P > 0) {
			Abs.P = P;
			printf(BLD "AddAbs  " NRM " P = %8.2f mbar (provided)\n", P);
		}
		else {
			Abs.P = 1000;
			printf(BLD "AddAbs  " NRM " P = %8.2f mbar (default)\n", Abs.P);
		}
		if(T > 0) {
			Abs.T = T;
			printf(BLD "AddAbs  " NRM " T = %8.2f K    (provided)\n", T);
		}
		else {
			Abs.T = 300;
			printf(BLD "AddAbs  " NRM " T = %8.2f K    (default)\n", Abs.T);
		}
		
		double a = 0;
		for(unsigned j = 0; j < Abs.Z.size(); j++) a += Abs.A[j] * ((double)(Abs.W[j]));
		Abs.rho = a * Abs.P / (83.144621 * Abs.T);
	}
	else {
		Abs.isgas = false;
		Abs.P = -1;
		Abs.T = -1;
	}
	
	SetVedaParam(form, Abs);
	Abs.mid = -1;
	return 0;
}

int ELoss::ParseFormula(const char *form, std::vector< int > &Z, std::vector< int > &W, std::vector< double > &A) {
	Z.clear(); W.clear(); A.clear();
	
	char num[3], el[3];
	int z, w, err = 0;
	double a = -1;
	unsigned i = 0, j;
	if(chartype(form[0]) == 0) { //First char is a number: I assume that first element is a specific isotope!
		for(j = 0; (chartype(form[j]) == 0) && (j < 3); j++) {
			num[j] = form[j]; //Reading A for first element (CAUTION: only first element may have a specific A!)
		}
		num[j] = '\0';
		if(chartype(form[j]) == 0) err = -1; //ERROR: too many numbers!
		else {
			i = j;
			a = (double)(atoi(num));
		}
	}
	for(; (i<strlen(form)) && (!err); i++) {
		if(chartype(form[i])!=1) {
			//ERROR: Element names must start with an upper case letter!
			err=-2;
			break;
		}
		el[0] = form[i];
		if(chartype(form[i+1]) == 2) {
			el[1] = form[i+1];
			el[2] = '\0';
			i++;
		}
		else {
			el[1] = '\0';
		}
		
		z = -1;
		for(j = 0; j < elemDB.size(); j++) {
			if(elemDB[j].Name == el) {
				z = j + 1;
				if(a < 0) a = elemDB[j].M;
				break;
			}
		}
		if(z<0) {
			//ERROR: Element name doesn't exist!
			err=-3;
			break;
		}
		
		for(j = 0; (chartype(form[i+j+1]) == 0) && (j < 3); j++) {
			num[j] = form[i+j+1]; //Reading stechiometric number
		}
		num[j] = '\0';
		if(chartype(form[i+j+1]) == 0) {
			//ERROR: stechiometric number is too big!
			err=-1;
			break;
		}
		i += j;
		if(j == 0) w = 1;
		else w = atoi(num);
		
		Z.push_back(z);
		W.push_back(w);
		A.push_back(a);
		a = -1;
	}
	
	if((Z.size() == 0) && (err == 0)) err = -4;
	switch(err) {
		case -1: printf(RED "ParseFor" NRM " too many consecutive numerical characters\n"); break;
		case -2: printf(RED "ParseFor" NRM " element names must begin with an upper case letter\n"); break;
		case -3: printf(RED "ParseFor" NRM " one or more elements don't exist\n"); break;
		case -4: printf(RED "ParseFor" NRM " no elements added\n");
	}
	if(err) return -1;
	return 0;
}

void ELoss::SetVedaParam(const char *form, struct material &Abs) {
	//Vedaloss absorbers
	const char nmat[29][10] = {
		"Si",  "C10H8O4", "CH2", "Ni", "C3F8", "C",   "Ag",    "Sn", "CsI", "Au",
		"U",   "air",     "Nb",  "Ta", "V",    "CF4", "C4H10", "Al", "Pb",  "PbS",
		"KCl", "Ge",      "Ca",  "Cu", "Ti",   "Bi",  "Mg",    "Li", "Zn"
	};
	
	//Trim the mass number for the first element
	int istart=0;
	for(; chartype(form[istart]) == 0; istart++);
	
	for(Abs.Absorber = 0; Abs.Absorber < 29; Abs.Absorber++) {
		if(strcmp(form + istart, nmat[Abs.Absorber])==0) break;
	}
	if(Abs.Absorber == 29) Abs.Absorber = 10;
	else Abs.Absorber++;
	
	Abs.Pression = 0;
	Abs.Atloc = (float)(Abs.A[0]); //ok il primo elemento, tanto per i composti Atloc=0!
	switch(Abs.Absorber) {
		case 5: case 12: case 16: case 17:
			Abs.Pression = (float)(Abs.P);
			[[gnu::fallthrough]]; //niente break!! Anche in questi casi Atloc=0!
		case 2: case 3: case 9: case 20: case 21:
			Abs.Atloc = 0;
	}
	return;
}

int ELoss::GetAbsParam(const int mid, bool &isgas, double &rho, double &P, double &T) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "GetParam" NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "GetParam" NRM " bad material index\n");
		return -1.;
	}
	isgas = mateLS[jm].isgas;
	rho   = mateLS[jm].rho;
	P     = mateLS[jm].P;
	T     = mateLS[jm].T;
	return 0;
}

int ELoss::SetAbsParam(const int mid, const bool isgas, const double rho, const double P, const double T) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "GetParam" NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "GetParam" NRM " bad material index\n");
		return -1.;
	}
	
	struct material Abs = mateLS[jm];
	Abs.isgas = isgas;
	if(isgas) {
		if(P > 0) Abs.P = P;
		if(T > 0) Abs.T = T;
		double a = 0;
		for(unsigned j = 0; j < Abs.Z.size(); j++) a += Abs.A[j] * ((double)(Abs.W[j]));
		Abs.rho = a * Abs.P / (83.144621 * Abs.T);
	}
	else {
		if(rho > 0) mateLS[jm].rho = rho;
	}
	
	char form[STRMAXL];
	strcpy(form, Abs.Form.c_str());
	SetVedaParam(form, Abs);
	return 0;
}

// *************** Energy loss functions
//Braune-Schwalm formula for electronic energy loss
double ELoss::Schwalm(const int Zp, double E, const struct material &mat) {
	double dEdx=0., wtot=0.;
	double z, w, a, vq, v, y1, y2, xi, h, r, AL, g;
	double H[101]={8.080,5.706,0.609,-6.214,-10.646,-9.119,-3.171,2.205,3.656,1.097,-3.536,-7.364,-7.897,-4.762,0.120,4.370,6.883,7.880,8.016,7.764,
		7.327,6.766,6.100,5.347,4.534,3.701,2.898,2.182,1.606,1.219,1.054,1.125,1.426,1.930,2.595,3.368,4.193,5.016,5.793,6.490,
		7.087,7.575,7.957,8.234,8.384,8.239,7.350,5.386,3.265,2.891,4.604,6.653,7.722,7.937,7.806,7.575,7.303,6.998,6.661,6.294,
		5.901,5.487,5.056,4.617,4.177,3.746,3.334,2.950,2.604,2.305,2.063,1.883,1.771,1.730,1.762,1.866,2.038,2.273,2.565,2.906,
		3.286,3.696,4.125,4.564,5.004,5.436,5.853,6.249,6.618,6.959,7.269,7.546,7.791,8.004,8.188,8.345,8.477,8.586,8.676,8.749,8.807
	};
	double rho = mat.rho/1000.;
	double zp  = (double)Zp;
	
	for(unsigned j = 0; j < mat.Z.size(); j++) {
		if(E < EMIN) E = EMIN;
		
		z  = (double)(mat.Z[j]);
		w  = (double)(mat.W[j]);
		a  = mat.A[j] / rho;
		
		vq = 2.1322e-3 * E;
		v  = sqrt(vq);
		xi = vq / z;
		h  = (mat.Z[j] > 100) ? 9. : H[mat.Z[j]];
		r  = v / pow(zp, 0.509);
		AL = log(xi * (4.444 * z * z + 922.2 * a));
		y1 = 3.3e-4 * log(1. + 54721. * xi * (1.76e-2 * a * (1. - exp(-pow(0.054 * z, 5))) + 1. - exp(-0.3 * z)));
		y2 = (2. * sqrt(xi) / (z * (1. + 1.e4 * sqrt(xi))) - 1.32e-5 * h * AL * exp(-0.32 * AL * AL)) / (1. + 1.e12 * xi * xi * xi);
		g  = (Zp <= 2) ? (1. - exp(-116.79 * r - 3350.4 * r * r)) : (1. - pow(1.035 - 0.4 * exp(-0.16 * zp), 1. - exp(-137. * v)) * exp(-120.4 * v / pow(zp, 0.65)));
		
		dEdx += w * g * g * zp * zp * z * (y1 + y2) / (mat.A[j] * vq);
		wtot += w;
	}
	return dEdx / wtot;
}

//Braune-Schwalm formula for nuclear energy loss
double ELoss::Nuclear(const int Zp, const double E, const struct material &mat) {
	double dEdx=0., wtot=0.;
	double z, w, a, y, eps;
	//Atomic mass from EAL (I don't generate a different table for each isotope!)
	double zp  = (double)Zp;
	double ap = (Zp == 1) ? 1 : 2.072 * zp + 2.32e-03 * zp * zp;
	
	for(unsigned j = 0; j < mat.Z.size(); j++) {
		z  = (double)(mat.Z[j]);
		w  = (double)(mat.W[j]);
		a  = mat.A[j];
		
		y   = (ap + a) * sqrt(pow(zp, 0.6666667) + pow(z,0.6666667));
		eps = 3.25e4 * E * ap * a / (y * zp * z);
		
		dEdx += w * 8.6785 * (sqrt(eps) * log(eps + 2.1718) / (1. + 6.8 * eps + 3.4 * pow(eps, 1.5))) * zp * z * ap / (y * a);
		wtot += w;
	}
	return dEdx / wtot;
}

//table integration to obtain the desired range table
void ELoss::Integral(const std::vector< double > &vE, std::vector< double > &vR) {
	long double buff = 0, intg = 0;
	double e1, r1, e2, r2;
	
	if(vE.size() == 0) return;
	e1  = 0;
	r1 = vR[0];
	vR[0] = 0;
	for(unsigned j = 1; j < vE.size(); j++) {
		e2  = vE[j];
		r2  = vR[j];
		
		//to avoid huge approximation errors, I sum small quantities together (in buff), the I add them to intg only when buff becomes large enough
		buff += (long double)((r1 + r2) * (e2 - e1) / 2.);
		if(intg * 0.01 < buff) {
			intg += buff;
			buff  = 0;
		}
		vR[j] = (double)(intg + buff);
		e1 = e2;
		r1 = r2;
	}
	return;
}

//generate range table from input files
// first column  ->  energy per nucleon
//second column  ->  range per nucleon
// third column  ->  electronic dE/dx
//fourth column  ->     nuclear dE/dx
int ELoss::Tab(const int form, const int Zp, const struct material &mat) {
	const char flabel[4][8] = {"nist", "barbui", "srim", "schwalm"};
	//First search for existing table
	unsigned j;
	for(j = 0; j < tables.size(); j++) {
		if((tables[j].z == Zp) && (tables[j].mid == mat.mid) && (tables[j].form == form)) break;
	}
	if(j < tables.size()) {
		printf(BLD "Tab     " NRM " %s energy loss table selected!\n", flabel[form - 1]);
		return j;
	}
	
	//If the table doesn't exist, then create a new one!
	char fn[STRMAXL];
	FILE *f;
	printf(BLD "Tab     " NRM " Table creation for Z =%3d in %s (%s formula)\n", Zp, mat.Form.c_str(), flabel[form - 1]);
	sprintf(fn, "%s/%s_%d_%s.tab", TABLFOLD, flabel[form - 1], Zp, mat.Form.c_str());
	f = fopen(fn, "r");
	if((f == NULL) && (form == BARBUI)) {
		BarbTab(Zp, mat);
		f = fopen(fn, "r");
	}
	if((f == NULL) && (form == SCHWALM)) {
		SchwTab(Zp, mat);
		f = fopen(fn, "r");
	}
	
	if(f == NULL) {
		printf(UP YEL "Tab     " NRM " %s not found                   \n", fn);
		return -1;
	}
	
	int ret;
	double e, rng, dd1, dd2, emax = 0;
	std::vector< double > vE, vR, vDe, vDn;
	char row[STRMAXL];
	for(;;) {
		if(fgets(row, STRMAXL, f) == NULL) break;
		ret = sscanf(row, " %lg %lg %lg %lg ", &e, &rng, &dd1, &dd2);
		if(ret != 4) continue;
		
		vE.push_back(e); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(rng);
		if(e > emax) emax = e;
	}
	fclose(f);
	
	if(vE.size() == 0) {
		printf(UP YEL "Tab     " NRM " table creation failed                  \n");
		return -1;
	}
	
	struct table tb;
	tb.z = Zp; tb.mid = mat.mid; tb.form = form;
	tb.emax = emax;
	tb.range.set_points(vE, vR);
	tb.energy.set_points(vR, vE);
	tb.dedx_e.set_points(vE, vDe);
	tb.dedx_n.set_points(vE, vDn);
	tables.push_back(tb);
	printf(BLD "Tab     " NRM " %s energy loss table selected!\n", flabel[form - 1]);
	return (int)(tables.size()) - 1;
}

//generate intermediate file from SRIM input files with M. Barbui corrections
int ELoss::BarbTab(const int Zp, const struct material &mat) {
	char fn[STRMAXL];
	FILE *f;
	
	if((!mat.isgas) && (!(mat.Form == "C10H8O4")) && (!(mat.Z.size() == 1))) {
		printf(YEL "BarbTab " NRM " Barbui corrections work on gases, mylar and simple elements only!\n");
		return -1;
	}
	
	printf(BLD "BarbTab " NRM " Table creation for Z =%3d in %s\n", Zp, mat.Form.c_str());
	sprintf(fn, "%s/srim_1_%s.tab", TABLFOLD, mat.Form.c_str());
	if((f = fopen(fn, "r")) == NULL) {
		printf(UP YEL "TbBarb  " NRM " %s not found\n", fn);
		return -1;
	}
	
	//M. Barbui scaling parameters
	double P0, P1, p2, p3;
	if(mat.isgas) {
		//gas absorber
		P0 = 1.468 - 0.08301 * log((double)Zp);
		P1 = 4.615 - 0.71544 * log((double)Zp);
		p2 = 0.4039;
		p3 = 0.2965;
	}
	else {
		//solid or liquid absorber
		P0 = 1.45 - 0.07 * log((double)Zp);
		P1 = 6.;
		if(mat.Form == "C10H8O4") {
			//mylar absorber
			p2 = 0.5089;
			p3 = 0.62;
		}
		else {
			//simple element absorber
			p2 = 0.4641 - 0.0011 * (double)(mat.Z[0]);
			p3 = 0.6;
		}
	}
	
	int ret;
	double e, dd1, dd2, ec;
	std::vector< double > vE, vR, vDe, vDn;
	vE.push_back(0); vDe.push_back(0); vDn.push_back(0); vR.push_back(0);
	char row[STRMAXL];
	for(;;) {
		if(fgets(row, STRMAXL, f) == NULL) break;
		ret = sscanf(row, " %lg %*g %lg %lg ", &e, &dd1, &dd2);
		if((ret != 3) || (e < EMIN)) continue;
		
		//M. Barbui SRIM corrections
		if(mat.Form == "CF4") {
			if(e < 2.)          dd1 *= 1.06 - 0.0413 * e;
			else                dd1 *= 0.9744;
		}
		if(mat.Form == "C10H8O4") {
			if     (e < 0.2)    dd1 *= 1.0868;
			else if(e < 1.5268) dd1 *= 1.0398 + 0.3286 * e - 0.5029 * e * e + 0.1726 * e * e * e;
			else                dd1 *= 0.9835;
		}
		if(mat.Form == "C4H10") {
			if(e < 0.1203)      dd1 *= 0.7851 + 1.7868 * e;
		}
		
		//application of scaling factors
		ec = 1. - P0 * exp(-P1 * pow(e, p2) / pow((double)Zp, p3));
		if(ec > 0) dd1 *= pow(ec * (double)Zp, 2.);
		else dd1 = 0.;
		
		vE.push_back(e); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(1. / (dd1 + dd2));
	}
	fclose(f);
	
	if(vE.size() == 0) {
		printf(UP YEL "BarbTab " NRM " table creation failed\n");
		return -1;
	}
	Integral(vE, vR);
	
	char name[STRMAXL];
	sprintf(name, "%s/barbui_%d_%s.tab", TABLFOLD, Zp, mat.Form.c_str());
	FILE *fout = fopen(name, "w");
	fprintf(fout, "#    Energy           Range       dE/dx (elec)    dE/dx (nucl)\n");
	fprintf(fout, "#   [MeV/u]         [mg/cm2]     [MeV/(mg/cm2)]  [MeV/(mg/cm2)]\n");
	fprintf(fout, "# ------------------------------------------------------------\n");
	for(unsigned j=0; j<vE.size(); j++) {
		fprintf(fout, "   %.4le      %.4le      %.4le      %.4le\n", vE[j], vR[j], vDe[j], vDn[j]);
	}
	fclose(fout);
	
	return 1;
}

//generate range table from Braune-Schwalm formula
int ELoss::SchwTab(const int Zp, const struct material &mat) {
	printf(BLD "SchwTab " NRM " Table creation for Z =%3d in %s\n", Zp, mat.Form.c_str());
	
	double e, dd1, dd2, e1, y1, e2, y2, ec, yc;
	std::vector< double > vE, vR, vDe, vDn;
	vE.push_back(0); vDe.push_back(0); vDn.push_back(0); vR.push_back(0);			//j = 0
	
	const double emin = (double)EMIN;
	const double emax = (double)EMAX;
	
	dd1 = Schwalm(Zp, emin, mat); dd2 = Nuclear(Zp, emin, mat);
	vE.push_back(emin); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(1. / (dd1 + dd2));			//j = 1
	
	ec  = (emax + emin) / 2.; dd1 = Schwalm(Zp, ec, mat); dd2 = Nuclear(Zp, ec, mat);
	vE.push_back(ec); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(1. / (dd1 + dd2));			//j = 2
	
	dd1 = Schwalm(Zp, emax, mat); dd2 = Nuclear(Zp, emax, mat);
	vE.push_back(emax); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(1. / (dd1 + dd2));			//j = 3
	
	int pos;
	for(unsigned j = 1; j + 2 < vE.size(); j++) {
		e1 = vE[j];     y1 = vR[j];
		ec = vE[j + 1]; yc = vR[j + 1];
		e2 = vE[j + 2]; y2 = vR[j + 2];
		for(; fabs(yc - y1 - (ec - e1) * (y2 - y1) / (e2 - e1)) > PREC;) {
			if(ec - e1 < e2 - ec) pos = 1;
			else pos = 0;
			e = pos ? ((ec + e2)/2.) : ((ec + e1)/2.);
			dd1 = Schwalm(Zp, e, mat); dd2 = Nuclear(Zp, e, mat);
			
			if(pos) {
				vE.insert(vE.begin() + j + 2, e); vDe.insert(vDe.begin() + j + 2, dd1); vDn.insert(vDn.begin() + j + 2, dd2);
				vR.insert(vR.begin() + j + 2, 1. / (dd1 + dd2));
			}
			else {
				vE.insert(vE.begin() + j + 1, e); vDe.insert(vDe.begin() + j + 1, dd1); vDn.insert(vDn.begin() + j + 1, dd2);
				vR.insert(vR.begin() + j + 1, 1. / (dd1 + dd2));
				ec = vE[j + 1]; yc = vR[j + 1];
			}
			e2 = vE[j + 2]; y2 = vR[j + 2];
		}
	}
	if(vE.size() == 0) {
		printf(UP YEL "TbSchw  " NRM " table creation failed\n");
		return -1;
	}
	Integral(vE, vR);
	
	char name[STRMAXL];
	sprintf(name, "%s/schwalm_%d_%s.tab", TABLFOLD, Zp, mat.Form.c_str());
	FILE *fout = fopen(name, "w");
	fprintf(fout, "#    Energy           Range       dE/dx (elec)    dE/dx (nucl)\n");
	fprintf(fout, "#   [MeV/u]         [mg/cm2]     [MeV/(mg/cm2)]  [MeV/(mg/cm2)]\n");
	fprintf(fout, "# ------------------------------------------------------------\n");
	for(unsigned j=0; j<vE.size(); j++) {
		fprintf(fout, "   %.4le      %.4le      %.4le      %.4le\n", vE[j], vR[j], vDe[j], vDn[j]);
	}
	fclose(fout);
	
	return 1;
}

void stechio(std::vector< double > &w, std::vector< int > &iw) {
	unsigned i, nelem = w.size();
	double stk, mind = 1.e99, diff;
	unsigned chk, imin = 0;
	
	iw.resize(nelem);
	for(i = 1; i < 500; i++) {
		chk = 0;
		diff = 0;
		for(unsigned j = 0; j < nelem; j++) {
			stk = ((double)i) * w[j] / 100.;
			iw[j] = (int)(0.5 + stk);
			if(fabs(stk - (double)(iw[j])) <= 0.0001 * (double)i) chk++;
			diff += (stk - (double)(iw[j])) * (stk - (double)(iw[j]));
		}
		if(diff < mind) {
			mind = diff;
			imin = i;
		}
		if(chk == nelem) break;
	}
	if(i == 500) {
		for(unsigned j = 0; j < nelem; j++) iw[j] = (int)(0.5 + ((double)imin) * w[j] / 100.);
	}
	return;
}

int ELoss::NistTab(const char *fn, bool over /*= false*/) {
	FILE *f = fopen(fn, "r");
	if(f == NULL) {
		perror(YEL "fopen   " NRM);
		return -1;
	}
	int z, a;
	char name[STRMAXL];
	int ret = sscanf(fn, NISTFOLD "/%d_%d_%[^.].txt", &a, &z, name);
	if(ret != 3) {
		printf(YEL "NistTab " NRM " file name parsing failed\n");
		fclose(f);
		return -1;
	}
	struct material Abs;
	ret = SetAbsorber(Abs, name);
	if(ret < 0) {
		fclose(f);
		return -1;
	}
	
	sprintf(name, "%s/nist_%d_%s.tab", TABLFOLD, z, Abs.Form.c_str());
	FILE *fout = fopen(name, "r");
	if(fout != NULL) {
		fclose(fout);
		if(!over) {
			printf(BLD "NistTab " NRM " Table for Z =%3d in %s already in DB, skipping...\n", z, Abs.Form.c_str());
			fclose(f);
			return 0;
		}
	}
	
	printf(BLD "NistTab " NRM " Table creation for Z =%3d in %s\n", z, Abs.Form.c_str());
	double e, dd1, dd2, rng;
	std::vector< double > vE, vR, vDe, vDn;
	
	char row[STRMAXL];
	vE.push_back(0); vDe.push_back(0); vDn.push_back(0); vR.push_back(0);
	for(;;) {
		if(fgets(row, STRMAXL, f) == NULL) break;
		ret = sscanf(row, " %lg %lg %lg %lg ", &e, &dd1, &dd2, &rng);
		e /= (double)a;
		if((ret != 4) || (e < EMIN)) continue;
		dd1 *= 1.e-3; dd2 *= 1.e-3; rng *= 1.e+3 / (double)a;
		vE.push_back(e); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(rng);
	}
	fclose(f);
	
	if(vE.size() == 0) {
		printf(UP YEL "NistTab " NRM " table creation failed\n");
		return -1;
	}
	
	fout = fopen(name, "w");
	fprintf(fout, "#    Energy           Range       dE/dx (elec)    dE/dx (nucl)\n");
	fprintf(fout, "#   [MeV/u]         [mg/cm2]     [MeV/(mg/cm2)]  [MeV/(mg/cm2)]\n");
	fprintf(fout, "# ------------------------------------------------------------\n");
	for(unsigned j=0; j<vE.size(); j++) {
		fprintf(fout, "   %.4le      %.4le      %.4le      %.4le\n", vE[j], vR[j], vDe[j], vDn[j]);
	}
	fclose(fout);
	return 1;
}

int ELoss::SrimTab(const char *fn, bool over /*= false*/) {
	FILE *f = fopen(fn, "r");
	if(f == NULL) {
		perror(YEL "fopen   " NRM);
		return -1;
	}
	
	int z;
	double a, rho = -1;
	double e, dd1, dd2, rng;
	std::vector< double > vE, vR, vDe, vDn;
	std::vector< std::string > elem;
	std::vector< double > w;
	std::vector< int > iw;
	int ret, state = 0;
	char row[STRMAXL], rstr[STRMAXL], name[STRMAXL], u1[10], u2[10];
	double rw;
	FILE *fout;
	for(; state >= 0;) {
		if(fgets(row, STRMAXL, f) == NULL) break;
		for(unsigned j = 0; j < strlen(row); j++) {
			if(row[j] == ',') row[j] = '.';
		}
		
		switch(state) {
			case 0:
				ret = sscanf(row, " Ion = %*s [%d] . Mass = %lg amu", &z, &a);
				if(ret == 2) state++;
				break;
			case 1:
				ret = sscanf(row, " Target Density =  %lg g/cm3 = %*g atoms/cm3", &rho);
				if(ret == 1) state++;
				break;
			case 2:
				if(strncmp(row, "    ----   ----   -------   -------", 30) == 0) state++;
				break;
			case 3:
				if(strncmp(row, " ====================================", 30) == 0) {
					stechio(w, iw);
					name[0] = '\0';
					for(unsigned j = 0; j < elem.size(); j++) {
						if(iw[j] <= 1) strcpy(rstr, elem[j].c_str());
						else sprintf(rstr, "%s%d", elem[j].c_str(), iw[j]);
						if(strlen(name) + strlen(rstr) < STRMAXL) strcat(name, rstr);
					}
					struct material Abs;
					ret = SetAbsorber(Abs, name);
					if(ret < 0) {
						state = -1;
						break;
					}
					sprintf(name, "%s/srim_%d_%s.tab", TABLFOLD, z, Abs.Form.c_str());
					fout = fopen(name, "r");
					if(fout != NULL) {
						fclose(fout);
						if(!over) {
							printf(BLD "SrimTab " NRM " Table for Z =%3d in %s already in DB, skipping...\n", z, Abs.Form.c_str());
							state = -2;
							break;
						}
					}
					rho *= 1000.;
					printf(BLD "SrimTab " NRM " Table creation for Z =%3d (A =%3.0f) in %s (rho =%5.0f mg/cm3)\n", z, a, Abs.Form.c_str(), rho);
					vE.push_back(0); vDe.push_back(0); vDn.push_back(0); vR.push_back(0);
					state++;
				}
				else {
					ret = sscanf(row, " %s %*d %lg %*g ", rstr, &rw);
					if(ret == 2) {
						elem.push_back(rstr);
						w.push_back(rw);
					}
				}
				break;
			case 4:
				ret = sscanf(row, " %lg %10s %lg %lg %lg %10s %*g %*s %*g %*s", &e, u1, &dd1, &dd2, &rng, u2);
				if(ret == 6) {
					if(u1[0] == 'e')      e *= 1.e-6;
					else if(u1[0] == 'k') e *= 1.e-3;
					else if(u1[0] == 'G') e *= 1.e3;
					e /= a;
					if(u2[0] == 'A')                       rng *= 1.e-4;
					else if(u2[0] == 'm' && u2[1] == 'm')  rng *= 1.e3;
					else if(u2[0] == 'm' && u2[1] == '\0') rng *= 1.e6;
					rng = um_to_mgcm2(rng, rho) / a;
					vE.push_back(e); vDe.push_back(dd1); vDn.push_back(dd2); vR.push_back(rng);
				}
		}
	}
	fclose(f);
	//state == -1 -> error
	//state == -2 -> material already in db (and overwrite flag is false)
	if(state == -1) return -1;
	if(state == -2) return 0;
	
	if(vE.size() == 0) {
		printf(UP YEL "SrimTab " NRM " table creation failed                                       \n");
		return -1;
	}
	
	fout = fopen(name, "w");
	fprintf(fout, "#    Energy           Range       dE/dx (elec)    dE/dx (nucl)\n");
	fprintf(fout, "#   [MeV/u]         [mg/cm2]     [MeV/(mg/cm2)]  [MeV/(mg/cm2)]\n");
	fprintf(fout, "# ------------------------------------------------------------\n");
	for(unsigned j=0; j<vE.size(); j++) {
		fprintf(fout, "   %.4le      %.4le      %.4le      %.4le\n", vE[j], vR[j], vDe[j], vDn[j]);
	}
	fclose(fout);
	return 1;
}

void ELoss::UpdateTables(bool over /*= false*/) {
	char filename[STRMAXL];
	struct dirent *ent;
	int ret, Nskip = 0, Nfail = 0, Nsucc = 0;
	
	DIR *dir = opendir(NISTFOLD);
	if(dir == NULL) {
		perror(YEL "opendir " NRM);
		goto srim;
	}
	while((ent = readdir(dir)) != NULL) {
		if(ent->d_type != DT_REG) continue;
		sprintf(filename, NISTFOLD "/%s", ent->d_name);
		ret = NistTab(filename, over);
		if(ret < 0) Nfail++;
		else if(ret == 0) Nskip++;
		else Nsucc++;
	}
	free(dir);
	
	srim:
	dir = opendir(SRIMFOLD);
	if(dir == NULL) {
		perror(RED "opendir " NRM);
		return;
	}
	while((ent = readdir(dir)) != NULL) {
		if(ent->d_type != DT_REG) continue;
		sprintf(filename, SRIMFOLD "/%s", ent->d_name);
		ret = SrimTab(filename, over);
		if(ret < 0) Nfail++;
		else if(ret == 0) Nskip++;
		else Nsucc++;
	}
	free(dir);
	printf("\n");
	printf(BLU "UpdateTb" NRM " Files parsed: %4d\n", Nfail + Nskip + Nsucc);
	printf("              success: " GRN "%4d\n" NRM, Nsucc);
	printf("              skipped: %4d\n", Nskip);
	printf("               failed: " RED "%4d\n" NRM, Nfail);
	printf("\n");
	return;
}

//Energy loss core function (except for vedaloss!)
//                    IN1               IN2              RETURN VALUE
// opt = 0  =>   initial energy      thickness         residual energy
// opt = 1  =>  residual energy      thickness          initial energy
// opt = 2  =>   initial energy          -              maximum range
// opt = 3  =>     thickness             -           punch-through energy
// opt = 4  =>   initial energy   residual energy         thickness
double ELoss::Core(const int opt, const int Zp, const struct material &mat, const int form, double in1, double in2) {
	if(opt < 0 || opt > 4) {
		printf(RED "ELCore  " NRM "Invalid option (%d)\n", opt);
		return -1.;
	}
	if(Zp <= 0 || Zp > 118) {
		printf(RED "ELCore  " NRM "Projectile Z out of range [1:118] (%d)\n", Zp);
		return -1.;
	}
	if(in1 < 0) {
		printf(RED "ELCore  " NRM "Not admitted negative value for input 1 (%f)\n", in1);
		return -1.;
	}
	if((opt == 0 || opt == 1 || opt == 4) && (in2 < 0)) {
		printf(RED "ELCore  " NRM "Not admitted negative value for input 2 (%f)\n", in2);
		return -1.;
	}
	
	int tidx = -1;
	for(int j = 1; j < 5; j++) {
		if((form != 0) && (form != j)) continue;
		tidx = Tab(j, Zp, mat);
		if(tidx >= 0) break;
	}
	if(tidx < 0) {
		printf(RED "ELCore  " NRM " No valid energy loss formula exists for Z = %d in %s\n", Zp, mat.Name.c_str());
		return -1.;
	}
	
	//CASE 3
	if(opt == 3) {
		//in1 is a thickness with unit um => must be converted to mg/cm^2
		in1 = um_to_mgcm2(in1, mat.rho);
		return tables[tidx].energy(in1);
	}
	
	//CASE 2
	double range = tables[tidx].range(in1);
	if(range < 0) return -1;
	if(opt == 2) {
		return mgcm2_to_um(range, mat.rho);
	}
	
	//CASE 4
	double tres;
	if(opt == 4) {
		tres = tables[tidx].range(in2);
		if(tres < 0) return -1;
		return mgcm2_to_um(range - tres, mat.rho);
	}
	
	//CASES 0 and 1
	in2 = um_to_mgcm2(in2, mat.rho);
	if(opt == 0) {
		if(in2 >= range) return 0;
		tres = range - in2;
	}
	else tres = range + in2;
	
	return tables[tidx].energy(tres);
}

// *************** USER FUNCTIONS
// from Ein and thickness to Eres
double ELoss::ERes(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double thickness /*in um*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "ERes    " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "ERes    " NRM " bad material index\n");
		return -1.;
	}
	double out;
	if(form < 5) out = ((double)Ap) * Core(0, Zp, mateLS[jm], form, ein / ((double)Ap), thickness / (double)Ap);
	else {
		printf(BLD "ERes    " NRM " vedaloss energy loss table selected!\n");
		float e1, el1;
		float epart = (float)ein;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess = (float)thickness;
		int idir = 1, icod;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e1, &el1, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
		out = (double)e1;
	}
	if(out < 0) return -1;
	return out;
}

// from Ein and thickness to Elost
double ELoss::ELost(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double thickness /*in um*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "ELost   " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "ELost   " NRM " bad material index\n");
		return -1.;
	}
	double out, eres;
	if(form < 5) {
		eres = ((double)Ap) * Core(0, Zp, mateLS[jm], form, ein / ((double)Ap), thickness / (double)Ap);
		out = (eres <= EZERO) ? ein : ((eres >= ein) ? 0 : (ein - eres));
	}
	else {
		printf(BLD "ELost   " NRM " vedaloss energy loss table selected!\n");
		float e1, el1;
		float epart = (float)ein;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess = (float)thickness;
		int idir = 1, icod;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e1, &el1, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
		out = (double)el1;
	}
	if(out < 0) return -1;
	return out;
}

// from thickness and Eres to Ein
double ELoss::EIn_res(const int Zp, const int Ap, const int mid, const int form, const double eres /*in MeV*/, const double thickness /*in um*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "Ein_res " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "Ein_res " NRM " bad material index\n");
		return -1.;
	}
	double out;
	if(form < 5) out = ((double)Ap) * Core(1, Zp, mateLS[jm], form, eres / ((double)Ap), thickness / (double)Ap);
	else {
		printf(BLD "Ein_res " NRM " vedaloss energy loss table selected!\n");
		float ei, el1;
		float epart = (float)eres;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess = (float)thickness;
		int idir = 2, icod;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &ei, &el1, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
		out = (double)ei;
	}
	if(out < 0) return -1;
	return out;
}

// from thickness and Elost to Ein
double ELoss::EIn_lost(const int Zp, const int Ap, const int mid, const int form, const double elost /*in MeV*/, const double thickness /*in um*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "Ein_lost" NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "Ein_lost" NRM " bad material index\n");
		return -1.;
	}
	double out = -1;
	if(form<5) {
		const double ept = ((double)Ap) * Core(3, Zp, mateLS[jm], form, thickness / ((double)Ap));
		if(elost > ept) {
			printf(RED "Ein_lost" NRM " input is greater than punch-through energy (%lg MeV)!\n", ept);
			goto output;
		}
		if(elost == ept) {
			out = ept;
			goto output;
		}
		double e1 = ept, e2 = EMAX, ex;
		double elx;
		if(e2 <= e1) {
			printf(RED "Ein_lost" NRM " energy loss table is too short (check 1)!\n");
			goto output;
		}
		elx = e2 - ((double)Ap) * Core(0, Zp, mateLS[jm], form, e2 / ((double)Ap), thickness / (double)Ap);
		if(elx >= elost) {
			printf(RED "Ein_lost" NRM " energy loss table is too short (check 2)!\n");
			goto output;
		}
		int n;
		for(n = 0; n < ITER; n++) {
			ex = (e1 + e2) / 2.;
			elx = ex - ((double)Ap) * Core(0, Zp, mateLS[jm], form, ex / ((double)Ap), thickness / (double)Ap);
			if(elx < elost) e2 = ex;
			else e1 = ex;
			if(fabs(elx - elost) < PREC) break;
		}
		if(n >= ITER) {
			printf(YEL "Ein_lost" NRM " too many iterations while searching energy!\n");
		}
		out = (e1 + e2) / 2.;
	}
	else {
		printf(BLD "Ein_lost" NRM " vedaloss energy loss table selected!\n");
		float epart;
		float de    = (float)elost;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess = (float)thickness;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		de_vedaloss_(&zpart, &apart, &(mateLS[jm].Atloc), &de, &spess, &epart, &(mateLS[jm].Absorber), &(mateLS[jm].Pression));
		out = (double)epart;
	}
	output:
	if(out < 0) return -1;
	return out;
}

double ELoss::PunchThrough(const int Zp, const int Ap, const int mid, const int form, const double thickness /*in um*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "P-T     " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "P-T     " NRM " bad material index\n");
		return -1.;
	}
	double out;
	if(form < 5) out = ((double)Ap) * Core(3, Zp, mateLS[jm], form, thickness / ((double)Ap));
	else {
		const double ept = ((double)Ap) * Core(3, Zp, mateLS[jm], SCHWALM, thickness / ((double)Ap));
		printf(BLD "P-T     " NRM " vedaloss energy loss table selected!\n");
		float e1, e2, ex, dummy;
		float epart1 = (float)ept, epart2 = (float)ept, epartx;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess = (float)thickness;
		int idir = 1, icod, n;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		do {
			epart1 *= 0.9;
			ecorr_veda_(&epart1, &zpart, &apart, &(mateLS[jm].Atloc), &e1, &dummy, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e1 > 0);
		do {
			epart2 *= 1.1;
			ecorr_veda_(&epart2, &zpart, &apart, &(mateLS[jm].Atloc), &e2, &dummy, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e2 <= 0);
		for(n = 0; n < ITER; n++) {
			epartx = (epart1 + epart2) / 2.;
			ecorr_veda_(&epartx, &zpart, &apart, &(mateLS[jm].Atloc), &ex, &dummy, &(mateLS[jm].Absorber), &spess, &idir, &icod, &(mateLS[jm].Pression));
			if(ex > 0) {
				epart2 = epartx; e2 = ex;
				if(ex < EZERO) break;
			}
			else {
				epart1 = epartx; e1 = ex;
			}
		}
		if(n >= ITER) printf(RED "P-T     " NRM " too many iterations while searching energy!\n");
		out = (((double)epart1) + ((double)epart2)) / 2.;
	}
	if(out < 0) return -1;
	return out;
}

double ELoss::Range(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "Range   " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "Range   " NRM " bad material index\n");
		return -1.;
	}
	double out;
	if(form < 5) out = ((double)Ap) * Core(2, Zp, mateLS[jm], form, ein / ((double)Ap));
	else {
		const double rng = ((double)Ap) * Core(2, Zp, mateLS[jm], SCHWALM, ein / ((double)Ap));
		printf(BLD "Range   " NRM " vedaloss energy loss table selected!\n");
		float e1, e2, ex, dummy;
		float epart = (float)ein;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess1 = (float)rng, spess2 = (float)rng, spessx;
		int idir = 1, icod, n;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		do {
			spess1 *= 0.9;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e1, &dummy, &(mateLS[jm].Absorber), &spess1, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e1 <= 0);
		do {
			spess2 *= 1.1;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e2, &dummy, &(mateLS[jm].Absorber), &spess2, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e2 > 0);
		for(n = 0; n < ITER; n++) {
			spessx = (spess1 + spess2) / 2.;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &ex, &dummy, &(mateLS[jm].Absorber), &spessx, &idir, &icod, &(mateLS[jm].Pression));
			if(ex > 0) {
				spess1 = spessx; e1 = ex;
				if(ex < EZERO) break;
			}
			else {
				spess2 = spessx; e2 = ex;
			}
		}
		if(n >= ITER) printf(RED "Range   " NRM " too many iterations while searching energy!\n");
		out = (((double)spess1) + ((double)spess2)) / 2.;
	}
	if(out < 0) return -1;
	return out;
}

double ELoss::Thickness(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double eres /*in MeV*/) {
	if((mid < 0) || (mid >= (int)(mateID.size()))) {
		printf(RED "Thkness " NRM " bad material ID\n");
		return -1.;
	}
	int jm = mateID[mid];
	if((jm < 0) || (jm >= (int)(mateLS.size()))) {
		printf(RED "Thkness " NRM " bad material index\n");
		return -1.;
	}
	double out;
	if(form < 5) out = ((double)Ap) * Core(4, Zp, mateLS[jm], form, ein / ((double)Ap), eres / ((double)Ap));
	else {
		const double rng = ((double)Ap) * Core(4, Zp, mateLS[jm], SCHWALM, ein / ((double)Ap), eres / ((double)Ap));
		printf(BLD "Thkness " NRM " vedaloss energy loss table selected!\n");
		float e1, e2, ex, dummy;
		float epart = (float)ein;
		float zpart = (float)Zp;
		float apart = (float)Ap;
		float spess1 = (float)rng, spess2 = (float)rng, spessx;
		int idir = 1, icod, n;
		if(!vedaDB) {
			vedaDB = true;
			vedaloss_();
		}
		do {
			spess1 *= 0.9;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e1, &dummy, &(mateLS[jm].Absorber), &spess1, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e1 <= eres);
		do {
			spess2 *= 1.1;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &e2, &dummy, &(mateLS[jm].Absorber), &spess2, &idir, &icod, &(mateLS[jm].Pression));
		}
		while(e2 > eres);
		for(n = 0; n < ITER; n++) {
			spessx = (spess1 + spess2) / 2.;
			ecorr_veda_(&epart, &zpart, &apart, &(mateLS[jm].Atloc), &ex, &dummy, &(mateLS[jm].Absorber), &spessx, &idir, &icod, &(mateLS[jm].Pression));
			if(ex > eres) {
				spess1 = spessx; e1 = ex;
			}
			else {
				spess2 = spessx; e2 = ex;
			}
			if(fabs(e2 - e1) < EZERO) break;
		}
		if(n >= ITER) printf(RED "Thkness " NRM " too many iterations while searching energy!\n");
		out = (((double)spess1) + ((double)spess2)) / 2.;
	}
	if(out < 0) return -1;
	return out;
}

