#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

int main()
{
    pid_t pid = fork();

    if(pid == 0) {
            system("watch ls");
    }
    else {
      sleep(5);
      
      kill(getpid(),SIGTERM);  
    }
    
    return 0;
}
