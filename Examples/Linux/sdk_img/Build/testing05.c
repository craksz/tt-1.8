#include "ttiofiles.h"

//test code for ttiofiles

int main (int argc, char **argv) {

	const char *theFile = "input.txt";
	theCell theVars;
	int total;
//	theVars.value1=0;
//	printf("\t\t%d\n",theVars.value1);
	//printf("%d\n\n",argc);
	if (argc>1){
		theVars[0].theInt=12;
		strcpy(theVars[0].theString,"yeahhhh");
		theVars[1].theInt=15;
		strcpy(theVars[1].theString,"fuck yeah");
		theVars[2].theInt=NULL;
		//theVars.value3=122;
		saveVars(theFile,theVars);
	}
	total=loadVars(theFile,theVars);

	for(int i=0;i<total;i++){
		printf("%s:%d\n\n",theVars[i].theString,theVars[i].theInt);
	}
	return 0;
};
