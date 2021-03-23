#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define BUFFER_SIZE 1024
char const* pInfoDir ="/proc";
char const* pStatAt ="/proc/%d/stat";
char const* pCmdLineAt ="/proc/%d/cmdline";

bool isNumStr(char const* str)
{
    for (int i=0; i< strlen(str); i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

char* fileRead(char* filePath)
{
    FILE* f;
    if ((f = fopen(filePath, "rb")) == NULL){	
    
		return NULL;
	}
    //char buffer[BUFFER_SIZE];
    char* buffer;
    buffer = (char*)malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
    size_t ret = fread(buffer,1,BUFFER_SIZE,f);
    // if (ret!=BUFFER_SIZE){
    //     return NULL;
    // }
    return buffer;
}
int main(int argc, char const** argv){

    printf("PID     PPID        CmdLine\n");

    DIR* dir = opendir(pInfoDir);	

	if (dir == NULL){
		printf("Cannot open '%s' directory\n", pInfoDir);
		return 1;
	}

    struct dirent *entry;
    while ((entry=readdir(dir)) != 0){

        char const *entryName = entry->d_name;
        if ( !isNumStr(entryName) || entry->d_type != DT_DIR)
            continue;

        int pid,ppid; 
        pid= atoi(entryName);

        //read PPID
        char name[100];
        sprintf(name,pStatAt,pid);
        char* pStat= fileRead(name);
        sscanf(strchr(pStat, ')') + 1, "%*s %d", &ppid);

        //read cmdline
        sprintf(name,pCmdLineAt,pid);
        char* cmdLine = fileRead(name);
       
        
        printf("%d      %d      %s\n",pid,ppid,cmdLine);
       
    }
    closedir(dir);
    return 0;

}