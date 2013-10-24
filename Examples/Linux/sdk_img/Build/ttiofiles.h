#include <string.h>
#include "stdlib.h"
#include "stdio.h"

#include <string>
#include <fstream>
#include <iostream> // for std::cout

typedef struct {
	int 	theInt;
	char 	theString[100];
}var_t;
  
typedef var_t theCell[10];


int saveVars(const char *theFile, theCell &vars){
	
	int n=0;
	std::fstream f (theFile,std::ios::out);
	
	if (!f.is_open()){
     std::cout << "File not found: " << theFile << std::endl;
     return -1;
  }
  
  while (vars[n].theInt){
  	f << vars[n].theString << "=" << vars[n].theInt << std::endl;
  	n++;
  }//*/
	f.close();
	return 0;
};

int loadVars(const char *theFile, theCell &vars){
  long int theInt;
	char * pEnd;
	int n=0;
  std::fstream f (theFile, std::ios::in);

  if (!f.is_open ()) {
     std::cout << "File not found: " << theFile << std::endl;
     return -1;
  }

  while (f.good ()) {
	  // TODO: it would probably be a good idea to move this stuff into a 
          // separate function
	
	  // 1. read a line from the file
	  std::string line;
	  std::getline (f, line);
	  
	  if ((line.substr (0, 1) == ";") || line.empty ()) {
		  continue;
	  }
	
	  // 2. find the index of first '=' character on the line
	  std::string::size_type offset = line.find_first_of ('=');
	
	  // 3. extract the key (LHS of the ini line)
	  std::string key = line.substr (0, offset);
	  // 4. extract the value (RHS of the ini line)
	  std::string value = line.substr (offset+1, line.length ()-(offset+1));
	  theInt=strtol(value.c_str(),&pEnd,10);
	  //theInt+=20;
	  // 5. ??? do what you need to do with the key/value pair
	  //std::cout << key.c_str () << " = " << value.c_str () << std::endl;
	  //std::cout << theInt << std::endl;
	  //if (!strcmp(key.c_str(),"value1")){
	  	vars[n].theInt=theInt;
	  	strcpy(vars[n].theString,key.c_str());
			//printf("%s:%d\n\n",vars[n].theString,vars[n].theInt+1);
	  	n++;
  }
  
  f.close ();

  return n;
	
};
