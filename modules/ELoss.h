/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 15/09/2021                          *
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
#define EMAX    1000
#define PREC       0.0001

#define EZERO      0.001
#define ITER     100

//Energy loss formulas
#define NIST       1
#define BARBUI     2
#define SRIM       3
#define SCHWALM    4
#define VEDALOSS   5

#define NISTFOLD "db/tb-nist-output"
#define SRIMFOLD "db/tb-srim-output"
#define TABLFOLD "db/tb-range-and-dedx"

extern "C" {
	void ecorr_veda_(float *eingresso,float *zpr,float *apr,float *atar, float *eout,float *elost,int *mate,float *thick,int *idir, int *icod, float *pressione);
	void de_vedaloss_(float *zpr,float *apr,float *atar,float *de,float *thick,float *e,int *mate,float *pressione);
	void vedaloss_();
}

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
	//vedaloss variables
	int Absorber;
	float Atloc, Pression;
};

struct table {
	//one table for each projectile Z, material and energy loss formula (except vedaloss)
	int z, mid, form;
	double emax;
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
	
	double ERes(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double thickness /*in um*/);
	double ELost(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double thickness /*in um*/);
	double EIn_res(const int Zp, const int Ap, const int mid, const int form, const double eres /*in MeV*/, const double thickness /*in um*/);
	double EIn_lost(const int Zp, const int Ap, const int mid, const int form, const double elost /*in MeV*/, const double thickness /*in um*/);
	double PunchThrough(const int Zp, const int Ap, const int mid, const int form, const double thickness /*in um*/);
	double Range(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/);
	double Thickness(const int Zp, const int Ap, const int mid, const int form, const double ein /*in MeV*/, const double eres /*in MeV*/);
	
	void Integral(const std::vector< double > &vE, std::vector< double > &vR);
	
	static inline double MeV_to_AMeV(double MeV,   double A)   {return MeV/A;};
	static inline double AMeV_to_MeV(double AMeV,  double A)   {return AMeV*A;};
	static inline double um_to_mgcm2(double um,    double rho) {return rho*um/10000.;};
	static inline double mgcm2_to_um(double mgcm2, double rho) {return 10000.*mgcm2/rho;};
	
private:
	// *************** Elements and compounds database
	void LoadDB();
	int ReadFile(const char *fn, std::vector< std::vector< std::string > > &arg);
	
	std::vector< struct element >  elemDB;
	std::vector< struct compound > compDB;
	
	// *************** Materials
	void SetVedaParam(const char *form, struct material &Abs);
	int Mcnt;
	std::vector< struct material > mateLS;
	std::vector< int > mateID;
	
	// *************** Energy loss
	double Schwalm(const int Zp, double E, const struct material &mat);
	double Nuclear(const int Zp, const double E, const struct material &mat);
	
	int Tab(const int form, const int Zp, const struct material &mat);
	int BarbTab(const int Zp, const struct material &mat);
	int SchwTab(const int Zp, const struct material &mat);
	int NistTab(const char *fn, bool over = false);
	int SrimTab(const char *fn, bool over = false);
	double Core(const int opt, const int Zp, const struct material &mat, const int form, double in1, double in2 = -1);
	
	std::vector< struct table > tables;
	bool vedaDB;
};

#endif
