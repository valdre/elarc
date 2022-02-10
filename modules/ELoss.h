/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 17/11/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#ifndef _ELOSS
#define _ELOSS

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include "ShellColors.h"
#include "Spline.h"

#define STRMAXL 1000

#define EMIN       0.001
#define EMINVEDA   0.1
#define DECAMAX    3

#define ITER     100
#define PREC       0.0005

//Energy loss formulas
#define NFORMULAS  5
// 0 means AUTO
#define NIST       1
#define BARBUI     2
#define SRIM       3
#define SCHWALM    4
#define VEDALOSS   5

//Computing modes
#define NMODES          6
#define EIN_THK_TO_ERES 0
#define ERES_THK_TO_EIN 1
#define ELST_THK_TO_EIN 2
#define EIN_ERES_TO_THK 3
#define THK_TO_EPT      4
#define EPT_TO_THK      5

#define NISTFOLD "db/tb-nist-output"
#define SRIMFOLD "db/tb-srim-output"
#define VEDAFOLD "db/tb-vedaloss-coeff"
#define TABLFOLD "db/tb-range-and-dedx"

struct element {
	std::string Name;
	double M;
	double rho; //=0 means gas, <0 means undefined
};

struct compound {
	std::string Name, Form;
	double rho; //=0 means gas, <0 means undefined
};

struct material {
	int mid;
	std::string Name, Form;
	//vectors with data of compound elements:
	std::vector< int > Z, W; //atomic number and stechiometric coefficient
	std::vector< double > A; //mass number (exact or average on natural isotopic distribution)
	bool isgas;
	double rho;
	//gas data (ignored for solids and liquids): pression and temperature
	double P, T;
};

struct table {
	//one table for each projectile Z, material and energy loss formula
	int z, mid, form;
	double emax, rmax;
	tk::spline range; //range table (integral of 1/(dE/dx) in funcion of E/A)
	tk::spline energy;  //energy table (E/A in function of the integral of 1/(dE/dx))
	tk::spline dedx_e, dedx_n; //electronic and nuclear dE/dx in function of E/A
};

// const double nan = sqrt(-1);

class ELoss {
public:
	ELoss();
	~ELoss();
	
	//AddAbsorber performs SetAbsorber, then it adds the struct material to the mateLS list.
	int AddAbsorber(const char *name /*name or formula*/, const double rho = -1 /*density in mg/cm^3*/, const double P = -1 /*pressure in mbar, ignored if not a gas*/, const double T = -1 /*temperature in K, ignored if not a gas*/);
	int SetAbsorber(struct material &Abs, const char *name /*name or formula*/, const double rho = -1 /*density in mg/cm^3*/, const double P = -1 /*pressure in mbar, ignored if not a gas*/, const double T = -1 /*temperature in K, ignored if not a gas*/);
	int ParseFormula(const char *form, std::vector< int > &Z, std::vector< int > &W, std::vector< double > &A);
	int GetAbsParam(const int mid, bool &isgas, double &rho, double &P, double &T);
	int SetAbsParam(const int mid, const bool isgas, const double rho = -1, const double P = -1, const double T = -1);
	
	void UpdateTables(bool over = false);
	
	double Compute(const int mode, const int Zp, const int Ap, const int mid, const int form, double in1, double in2 = -1);
	double Getdedx(const int Zp, const int Ap, const int mid, const int form, double ein);
	
	static inline double MeV_to_AMeV(const double &MeV,   const double &A)   {return MeV/A;};
	static inline double AMeV_to_MeV(const double &AMeV,  const double &A)   {return AMeV*A;};
	static inline double um_to_mgcm2(const double &um,    const double &rho) {return rho*um/10000.;};
	static inline double mgcm2_to_um(const double &mgcm2, const double &rho) {return 10000.*mgcm2/rho;};
	
private:
	// *************** Elements and compounds database
	void LoadDB();
	int ReadFile(const char *fn, std::vector< std::vector< std::string > > &arg);
	
	std::vector< struct element >  elemDB;
	std::vector< struct compound > compDB;
	
	// *************** Materials
	int Mcnt;
	std::vector< struct material > mateLS;
	std::vector< int > mateID;
	
	// *************** Energy loss
	double Schwalm(const int Zp, double E, const struct material &mat);
	double Nuclear(const int Zp, const double E, const struct material &mat);
	double Vedapoli(const double le, const double *p);
	double Vedaloss(const double E, const double *p);
	
	void Integral(const std::vector< double > &vE, std::vector< double > &vR);
	void Derivate(const std::vector< double > &vE, const std::vector< double > &vR, std::vector< double > &vDe);
	
	int Tab(const int form, const int Zp, const struct material &mat);
	int BarbTab(const int Zp, const struct material &mat);
	int SchwTab(const int Zp, const struct material &mat);
	int VedaTab(const int Zp, const struct material &mat);
	int NistTab(const char *fn, bool over = false);
	int SrimTab(const char *fn, bool over = false);
	
	std::vector< struct table > tables;
};

#endif
