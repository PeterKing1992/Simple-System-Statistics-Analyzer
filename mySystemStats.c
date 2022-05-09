//mySystemStats.c: A small program that displays important user and system info about your linux machine
//@Author Zhao Ji Wang

// Import necessary libraries
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

/*
	Helper function to convert ram usage from bytes to gigabytes

	Take parameter value of type float, which is ram usage in bytes
	
	Return the ram usage in gigabytes
*/
float toGB(long value){ 
	return (float)value/(float)1073741824; 
}

/*
	Helper function that decides if a string contains only numbers

	Take parameter string of type char*, which is the string to check whether only contains numbers

	Return 1 if string only contains numbers, 0 otherwise
*/

int isNumber(char *string){
	for(int i = 0; i < strlen(string); i ++){//Go through each character in the string to see if they are digits
		if(!isdigit(string[i])){
			return 0; //if any of the characters is not digit, return 0
		}
	}
	return 1; 
}


/*
	Helper function that retrieves user sessions and print the users' usernames and session info out each on a separate line

	Take no parameter

	Return no value
*/
void getUserSessionInfo(){
	utmpname(UTMP_FILE); 
	setutent();  
	struct utmp * u = getutent(); //Read user info from UTMP_FILE and store this info in the pointer *u

	// As long as the we have not reached the end of file, keep reading the file and print each user's name and info
	while(u != NULL){
		if(u->ut_type == USER_PROCESS){//check whether the user is an actual user(excludes users such as login and boot)
			printf("Username: %s ", u->ut_user); 
			printf("%s (%s)\n", u->ut_line, u->ut_host);  
		}
		u = getutent(); 
	}
	endutent(); //Close the file after done reading
}

/*
	Helper function that reads the cpu idle time and non idle time from /proc/stat file and store these info in locations pointed by
	pointers cpuIdle and cpuNonIdle accordingly

	Take parameters cpuIdle and cpuNonIdle of type long*, which are pointers that point to locations where to store cpu idle time and non idle time sampled by the function
	
	Return no value 
*/
void sampleCpuUsage(long *cpuIdle, long *cpuNonIdle){
	FILE *cpuUsage = fopen("/proc/stat", "r");
	int cpuFlag = 1; // cpuFlag indicates that a "cpu" keyword is spotted in the document, which tells the function to start scanning for important info afterwards
	int wordCount = 0; 
	char inputString[20];
	char **leftover = malloc(sizeof(char*) * 20);
	// The following while loop reads the document word by word and retrieves cpu usage info 
	while(fscanf(cpuUsage, "%s", inputString) != EOF){
		if(strcmp(inputString, "cpu") == 0){
			cpuFlag = 1;
		}else if(cpuFlag == 1){
			if(wordCount < 10){ 
				if(wordCount != 3 && wordCount != 4){ // numbers stored in the 3rd and 4th column should be considered cpu idle time
					*cpuNonIdle += strtol(inputString, leftover, 10); 
				}else{//numbers stored in anything other than the 3rd and 4th column are cpu non idle time
					*cpuIdle += strtol(inputString, leftover, 10); 
				}
			}else{
				break; 
			}
			wordCount ++; 
		}
	}
	fclose(cpuUsage); 
}

/*
	A helper function used to calculate the cpu usage at a particular instant in time
	takes two samples in cpuUsage(by calling sampleCpuUsage 2 times) and then use the 
	two samples(which are cumulative usage since boot) to calculate the non idle usage over the period of 
	time when the samples were taken as a parcentage

	return the percentage of cpu usage (non idle)
	
*/

float getCpuUsage(){
	long cpuIdle = 0; 
	long cpuNonIdle = 0; 
	long cpuIdle2 = 0; 
	long cpuNonIdle2 = 0; 
	sampleCpuUsage(&cpuIdle, &cpuNonIdle); //take first sample of cpu usage
	sleep(2); 
	sampleCpuUsage(&cpuIdle2, &cpuNonIdle2); //take second sample of cpu usage
	long netIdleUsage = cpuIdle2 - cpuIdle; 
	long netNonIdleUsage = cpuNonIdle2 - cpuNonIdle; 
	return ((float)netNonIdleUsage/((float)netIdleUsage + (float)netNonIdleUsage))*100; //Calculate the cpu usage based on the two samples taken
}

/*
	A helper function that prints cpu info including number of cpu cores and cpu usage
	reads the /proc/cpuinfo file to get the total number of cores, and calls getCpuUsage
	to calculate the cpu non idle usage
	
	Take parameter showGraphics of type int, which essentially indicate whether graphical 
	representations of CPU usage should be printed or not

	Return no value
*/

void getCpuInfo(int showGraphics){
	FILE *cpuInfo = fopen("/proc/cpuinfo", "r"); 
	char inputString[20]; 
	int colonFlag = 0; 
	int coreCountFlag = 0; 	
	char numCores[20]; 

	while (fscanf(cpuInfo, "%s", inputString) != EOF){
		if(strcmp(inputString, "cores") == 0){//When see keyword "cores", turn on coreCountFlag to let the function know it should be reading number of cpu cores next
			colonFlag = 1; 
		}else if(colonFlag == 1){
			coreCountFlag = 1;
			colonFlag = 0;  
		}else if(coreCountFlag == 1){
			coreCountFlag = 0; 
			strcpy(numCores, inputString); 
			break; 
		}
	}
	printf("Number of cores: %s\n", numCores);
	float usage = getCpuUsage(); 
	printf("Utilization: %.2f%%\n", usage);
	//This following part prints the graphical representations of the cpu usage 
	if(showGraphics == 1){
		printf("|||"); 
		for(int i = (int)(usage); i > 0 ; i--){//Print a "|" after the three "|"s(used to indicate the start of the graphical representation) for every percentage of cpu usage as graphical representation
			printf("|"); 
		}
		printf("\n"); 
	}
	fclose(cpuInfo); 
}

/*
	A helper function that reads information about the system(Name, version, architecture, etc.)
	and prints the info

	Take no parameter
	
	Return no value
*/

void getAndPrintSystemInfo(){
	struct utsname *u = (struct utsname *)malloc(sizeof(struct utsname)); 
	uname(u); 
	printf("System Name = %s\n", u->sysname); 
	printf("Machine Name = %s\n", u->nodename); 
	printf("Version = %s\n", u->version); 
	printf("Release = %s\n", u->release); 
	printf("Architecture = %s\n", u->machine); 
	free(u); 
}

/*
	A helper function that reads system ram usage from system file with the sysinfo library
	and prints physical and virtual ram usage with graphical representations of the relative
	change in ram usage if needed 

	Take parameter lastRamUsage of type float* which points at a location where last sampled ram usage
	is stored(can be used to calculate relative change in ram usage) and showGraphics of type int which
	indicates whether graphical representation of ram usage should be printed

	Return no value 
*/
void getAndPrintSystemRamInfo(float *lastRamUsage, int showGraphics){
	//Get data about physical and virtual ram usage as well as total physical and virutal ram
	struct sysinfo *s = (struct sysinfo *)malloc(sizeof(struct sysinfo)); 
	sysinfo(s); 
	long totalRamBytes = s->totalram; 
	long totalVirtualRamBytes = (s->totalswap) + totalRamBytes; 
	long totalUsedRamBytes = totalRamBytes - (s->freeram); 
	long totalUsedVirtualRamBytes = totalVirtualRamBytes - totalRamBytes - (s->freeswap) + totalUsedRamBytes; 

	float totalUsedRamGB = toGB(totalUsedRamBytes);  
	float totalRamGB = toGB(totalRamBytes); 
	float totalVirtualRamGB = toGB(totalVirtualRamBytes); 
	float totalUsedVirtualRamGB = toGB(totalUsedVirtualRamBytes); 
	
	printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", totalUsedRamGB, totalRamGB, totalUsedVirtualRamGB, totalVirtualRamGB);
	
	//show graphical representations of the ram usage
	if(showGraphics == 1){
		if(*lastRamUsage == -1){
			printf("|o %.2f (%.2f)", 0.00, totalUsedRamGB);  //if this is the first sample taken, only print "|o"
			*lastRamUsage = totalUsedRamGB; 
		}else{
			float changeInRam = totalUsedRamGB - *lastRamUsage;
			printf("|"); 
			if(changeInRam < 0){
				for(float j = changeInRam; j < 0; j+=0.01){//for every 0.01 decrease in ram usage, print a ":"
					printf(":"); 
				}
				printf("@"); 
			}else if(changeInRam > 0){
				for(float j = changeInRam; j > 0; j-=0.01){//for every 0.01 increase in ram usage, print a "#"
					printf("#"); 
				}
				printf("*"); 
			}else{
				printf("o"); 
			}
			printf(" %.2f (%.2f)", changeInRam, totalUsedRamGB); 
			*lastRamUsage = totalUsedRamGB; 
		}
	}

	printf("\n");
	free(s); 

}

/*
	The main function of the program, which takes command line arguments of the user, parse the instructions and
	call necessary helper functions to display info wanted by the user

	Take parameter argc of type int, which is the number of command line arguments, and parameter argv
	of type char** which is the command line arguments themselves
*/
int main(int argc, char ** argv){
	//Flags to indicate what kind of info the user wants to be shown
	int showSystem = 1; 
	int showGraphics = 0; 
	int numSamples = 10; 
	int tDelay = 1; 
	int showUser = 1; 
	
	int delaySet = 0; 

	//Iterate through argv to search for keywords and tweak the flags based on the keywords
	for(int i = 1; i < argc; i++){
		char *token; 
		token = strtok(argv[i], "="); 
 
		if(strcmp(argv[i], "--system") == 0){
			showUser = 0; 
			showSystem = 1; 
		}else if(strcmp(argv[i], "--graphics") == 0 || strcmp(argv[i], "-g") == 0){
			showGraphics = 1; 
		}else if(strcmp(argv[i], "--user") == 0){
			showSystem = 0; 
			showUser = 1; 
		}else if(strcmp(token, "--samples") == 0 ){
			char *leftover = strtok(NULL, "=");
			if(leftover != NULL){
				if(isNumber(leftover) == 1){ // only do the following if the leftover(text after "--samples=") is a number
					numSamples = (int)strtol(leftover, NULL, 10);
					if(argc > i + 1){
						char *tokenDelay; 
						tokenDelay = strtok(argv[i+1], "="); 
						if(strcmp(tokenDelay, "--tdelay") == 0){
							char *leftoverDelay =strtok(NULL, "=");
							if(leftoverDelay != NULL){ 
								if(isNumber(leftoverDelay) == 1){// only fo the following if the leftoverDelay(text after "--tdelay=" is a number)
									tDelay = (int)strtol(leftoverDelay, NULL, 10);
									delaySet = 1;  
								}else{
									fprintf(stderr, " '--tdelay=' should be followed by a number\n");
									exit(1);  
								}
							}else{
								fprintf(stderr, "'--tdelay' should be followed by a number\n"); 
								exit(1); 
							}
						}
					}  
				}else{ //Throw an error if "--samples=" is not followed by a number
					fprintf(stderr, "'--samples=' should be followed by a number\n");
					exit(1);  
				}
			}else{//Throw an error if "--samples=" is followed by an empty string
				fprintf(stderr, "'--samples=' should be followed by a number\n"); 
				exit(1); 
			}
		}else if(strcmp(token, "--tdelay") == 0){// Throw an error if "--tdelay" is used not by following "--samples=N"
			if(delaySet == 0){
				fprintf(stderr, "'--tdelay=' must be called after '--samples=N' where N specifies number of samples to take\n");
				exit(1); 
			}
		}else{//Any other arguments will result in an error
			fprintf(stderr, "'%s' is not recognized as a command line argument\n", argv[i]); 
			exit(1); 
		}
	}
	
	float lastRamUsage = -1;// set this variable to -1 to let the function know that there is no previous samples taken about memory
	//The following for loop repeatedly call the helper functions to gather and print the necessary information wanted by the user N number of times
	//where N is the samples indicated by the user, with an interval of delay specified by the user in between printing each sample
	for (int i = 0; i < numSamples; i++){
		printf("\033[2J");//clear the screen 
		printf("\033[H"); //reset the cursor to the upper left of the terminal
		printf("-------------------------------------------------------\n"); 
		printf("Number of Samples: %d, every %d second\n", numSamples, tDelay); 
		printf("%d out of %d samples displayed\n", i+1, numSamples); 
		//call functions based on the flags indicated by the user
		if(showSystem == 1){  
			printf("### Memory ### (Phys. Used/Tot -- Virtual Used/tot)\n"); 
			getAndPrintSystemRamInfo(&lastRamUsage, showGraphics); 
			
			printf("### CPU Info ###\n"); 
			getCpuInfo(showGraphics); 
		}
		if(showUser == 1){
			printf("### User/Sessions ###\n"); 
			getUserSessionInfo(); 
		}

		printf("### System Information ###\n");
		getAndPrintSystemInfo(); 

		sleep(tDelay); 
	}
}


