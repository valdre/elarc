#include <stdio.h>
#include <math.h>

void vedasac2coe(const char *fn = "loss1.vedasac") {
	const char nmat[29][10] = {
		"Si",  "C10H8O4", "CH2", "Ni", "C3F8", "C",   "Ag",    "Sn", "CsI", "Au",
		"U",   "air",     "Nb",  "Ta", "V",    "CF4", "C4H10", "Al", "Pb",  "PbS",
		"KCl", "Ge",      "Ca",  "Cu", "Ti",   "Bi",  "Mg",    "Li", "Zn"
	};
	
	FILE *fout = NULL, *f = fopen(fn, "r");
	if(f == NULL) return;
	
	bool fWrite = true;
	int ret, i = 1, state = 0, idx, iz = 0;
	char row[1000], mater[1000], fnout[1000];
	double Zm, Am, Zp, Ap, coeff[12];
	for(; state >= 0;) {
		if(fgets(row, 1000, f) == NULL) break;
		switch(state) {
			case 0:
				ret = sscanf(row, " %lg %lg %s %d ", &Zm, &Am, mater, &idx);
				if(ret != 4) break;
				if(idx != i) {
					printf("Index mismatch (idx = %d, i = %d)!\n", idx, i);
					state = -1;
					break;
				}
				sprintf(fnout, "tb-vedaloss-coeff/%s.coe", nmat[i-1]);
				if(fout != NULL) fclose(fout);
				fout = fopen(fnout, "w");
				if(fout == NULL) {
					perror(fnout);
					state = -1;
					break;
				}
				fprintf(fout, "# Zp  Ap   coefficients ->\n");
				printf("Reading coefficients for material %2d:%-20s (%10s): Z =%3.0f, A =%5.1f\n", idx, mater, nmat[i - 1], Zm, Am);
				fWrite = true; iz = 1;
				state = 1;
				break;
			case 1:
				ret = sscanf(row, " %lg %lg %lg %lg %lg %lg %lg %lg ", &Zp, &Ap, coeff, coeff + 1, coeff + 2, coeff + 3, coeff + 4, coeff + 5);
				if(ret != 8) break;
				if((int)(0.5 + Zp) != iz) {
					printf("Projectile Z mismatch (idx = %d, Zp = %.0f, iz = %d)!\n", idx, Zp, iz);
					state = -1;
					break;
				}
				state = 2;
				break;
			case 2:
				ret = sscanf(row, " %lg %lg %lg %lg %lg %lg ", coeff + 6, coeff + 7, coeff + 8, coeff + 9, coeff + 10, coeff + 11);
				if(ret != 6) {
					printf("Bad line reading (idx = %d, Zp = %.0f)!\n", idx, Zp);
					state = -1;
					break;
				}
				if(coeff[0] == 0.) fWrite = false;
				if(fWrite) {
					fprintf(fout, " %3d %3.0lf ", iz, Ap);
					for(int j = 0; j < 12; j++) fprintf(fout, " % 10.3le", coeff[j]);
					fprintf(fout, "\n");
				}
				if(iz == 100) {
					i++;
					state = 0;
				}
				else {
					iz++;
					state = 1;
				}
		}
		if(state < 0) break;
	}
	if(fout != NULL) fclose(fout);
	fclose(f);
	if(i <= 29) {
		printf("Something got wrong...\n");
	}
	return;
}
