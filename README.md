# Simple-System-Statistics-Analyzer
A program that can be used to track and display different resource statistics/usage on your Linux machine. 

1.Overview of the functions I wrote

float toGB(long value)
A Helper function to convert ram usage from bytes to gigabytes
Take parameter value of type float, which is ram usage in bytes
Return the ram usage in gigabytes

int isNumber(char *string)
A helper function that decides if a string contains only numbers
Take parameter string of type char*, which is the string to check whether only contains numbers
Return 1 if string only contains numbers, 0 otherwise

void getUserSessionInfo()
Helper function that retrieves user sessions and print the users' usernames along with some session info out each on a separate line
Take no parameter
Return no value

void sampleCpuUsage(long *cpuIdle, long *cpuNonIdle)
Helper function that reads the cpu idle time and non idle time from /proc/stat file and store these info in locations pointed by pointers cpuIdle and cpuNonIdle accordingly
Take parameters cpuIdle and cpuNonIdle of type long*, which are pointers that point to locations where to store cpu idle time and non idle time sampled by the function
Return no value 

float getCpuUsage()
A helper function used to calculate the cpu usage at a particular instant in time takes two samples in cpuUsage(by calling sampleCpuUsage 2 times) and then use the two samples(which are cumulative usage since boot) to calculate the non idle usage over the period of time when the samples were taken as a parcentage 
return the percentage of cpu usage (non idle)

void getCpuInfo(int showGraphics)
A helper function that prints cpu info including number of cpu cores and cpu usage reads the /proc/cpuinfo file to get the total number of cores, and calls getCpuUsage to calculate the cpu non idle usage
	
Take parameter showGraphics of type int, which essentially indicate whether graphical representations of CPU usage should be printed or not

Return no value

void getAndPrintSystemInfo()
A helper function that reads information about the system(Name, version, architecture, etc.) and prints the info

Take no parameter
	
Return no value

void getAndPrintSystemRamInfo(float *lastRamUsage, int showGraphics)
A helper function that reads system ram usage from system file with the sysinfo library and prints physical and virtual ram usage with graphical representations of the relative change in ram usage if indicated. 

Take parameter lastRamUsage of type float* which points at a location where last sampled ram usage is stored(can be used to calculate relative change in ram usage) and showGraphics of type int which indicates whether graphical representation of ram usage should be printed 

int main(int argc, char ** argv)
The main function of the program, which takes command line arguments of the user, parse the instructions and call necessary helper functions to display info wanted by the user

Take parameter arguments argc of type int, which is the number of command line arguments, and parameter argv of type char** which is the command line arguments themselves

2.How to use my program

Compile the code by running gcc -Wall mySystemStats.c

Then run ./a.out followed by a series of flags to indicate the information you want to see

1)
--graphics or -g will make the program print out graphical representations for cpu usage and ram usage

The graphical representation conventions are as follows: 

For ram: 
“|” means the start of the graphical representation, anything to the right of this is the graphical representation of the ram usage. 

Every “#” printed means a 0.01 GB positive change in physical memory usage compared to last sample. This series of “#”s is followed by a “*” to indicate that the total net change in ram usage is positive. (if the total net change is not divisible by 0.01, we will round up, so for example 0.05023 will result in 6 “#”s)

Every “:” printed means a 0.01GB negative change in physical memory usage compared to last sample. This series of “:”s is followed by a “@” to indicate that the total net change in ram usage is negative. (if the total net change is not divisible by 0.01, we will round up, so for example -0.05023 will result in 6 “:”s)

If the net change in ram usage is zero a “o” will be printed

For Cpu Usage: 
Regardless of the cpu usage, three “|”s will be printed to indicate the beginning of the graphical representation. 

After the three “|”s, for every 1 percent of the cpu usage, a “|” will be printed. (If the percentage of cpu usage is not a whole number, we will round it down)

2)
--user is used to indicate that only user session info will be wanted, so the program will not show any other info about the system ram usage and cpu usage information. 

The first column will display the usernames of each user session, the second column will show the device names each connected user is using, the third column will show the host name for each user’s remote login. 

3)
--system is used to indicate that only system info will be shown. So the program will not show any info about the user session info. 

The system info will have two parts, the ram usage and cpu usage. 

Ram(Memory) Usage will show physical and virtual memory used by the user in GB, and also compare them next to the total physical and virtual memory available. 

Cpu Usage will show the CPU non idle usage as a percentage at the instant the samples were taken. 

4)
--samples=N --tdelay=T 
Warning: these two are positional arguments, so they must be placed in the order shown if both of them are used as command line arguments. 
“--tdelay=T” cannot be passed as a command line argument independently of samples. (It must be preceded by “--samples=N”)

If any of the above two rules are broken, the program will terminate and output an error. 

Where N will indicate the number of samples memory usage, user sessions, and cpu usage will be taken. 
Where T will specify the time interval between each sample. 

Note: The flags shown in 1), 2), 3), 4) can be used in any order(and can each be present or not), but if both “--system” and “--user” are used at the same time, the second flag will offset the effect of the first flag, the same goes for any other flag called more than once as well. For example if I call “--samples=3 --samples=5”, the program will only take the last flag about samples and print samples 5 times. 

Also Note: when using samples flag, “--samples=” and “--tdelay=” must be used before the number values, just giving the number values without the text as command line arguments will result in error. 

Any other unrecognized flags will result in an error. 
