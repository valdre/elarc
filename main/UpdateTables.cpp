/*******************************************************************************
*                                                                              *
*                         Simone Valdre' - 15/09/2021                          *
*                  distributed under GPL-3.0-or-later licence                  *
*                                                                              *
*******************************************************************************/ 

#include "ELoss.h"

int main(int argc, char *argv[]) {
	bool over = false;
	int c;
	//Decode options
	while((c = getopt(argc, argv, "ho"))!=-1) {
		switch(c) {
			case 'o':
				over = true;
				break;
			case 'h': default:
				if(c != 'h') printf(YEL "UpdateTb" NRM " Unrecognized option\n");
				printf("\n**********  HELP **********\n");
				printf("    -h         this help\n");
				printf("    -o         overwrite existing table files\n");
				printf("\n");
				return 0;
		}
	}
	ELoss el;
	el.UpdateTables(over);
	return 0;
}
