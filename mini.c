#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
The passwd structure is defined in <pwd.h> as follows:
struct passwd {
char *pw_name; //user name
char *pw_passwd; //user password
uid_t pw_uid; //user id
gid_t pw_gid; //group id
char *pw_gecos; //user real name
char *pw_dir; //home directory 
char *pw_shell; //shell program 
};
*/

void prompt(){
	struct passwd * p1;
	char username[64];
	char hostname[256];
	char pathname[1024];
	//获取用户名
	p1=getpwuid(getuid());
	//获取路径
	getcwd(pathname,sizeof(pathname));
	//获取主机名---返回0正确
	if(gethostname(hostname,sizeof(hostname))==0){
		printf("\033[33m[DIY \033[0m\033[32m%s@%s:\033[0m\033[33m%s]\033[0m ",p1->pw_name,hostname,pathname);
	}	
	else{
		printf("\033[33m[DIY \033[33m\033[32m%s@unkown:\033[0m\033[33m%s]\033[0m",p1->pw_name,pathname);
	}
	if(geteuid()==0){
		printf("#");
	}
	else{
		printf("$");
	}
}


// 将str字符以spl分割,存于dst中，并返回子字符串数量
int split(char dst[][80], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}


int main(int argc,char* argv[]){
	while(1){
		prompt();
		fflush(stdout);//清除输出缓冲区，及时输出
		char buf[1024];
		memset(buf,0,sizeof(buf));//初始化buf为0
		size_t size=read(0,buf,sizeof(buf)-1); //从标准输入中读取命令 
		if(size>0){
			buf[size-1]='\0';
		}

		char *temp=buf;
		char dst[10][80];
		int num=split(dst,temp,"|");		
		
		if(num>1){
			int i0=0,i1=0;
			char *p0=dst[0];
			char *p1=dst[1];
			char * my_argv0[64]={0};
			char * my_argv1[64]={0};
			while((*p0==' '||*p0=='\t')){//过滤命令前面的空白符
				p0++;
			}
			while((*p1==' '||*p1=='\t')){//过滤命令前面的空白符
				p1++;
			}
			my_argv0[0]=p0;
			my_argv1[0]=p1;
			while(*p0!=0){//拆分命令于参数表
				if(*p0==' '||*p0=='\t'){
					while((*p0==' '||*p0=='\t')){//过滤命令中段的空白符
						*p0='\0';
						p0++;
					}
					if(*p0==0){
						break;
					}
					i0++;
					my_argv0[i0]=p0;
				}
				else{
					p0++;
				}
			}	
			while(*p1!=0){//拆分命令于参数表
				if(*p1==' '||*p1=='\t'){
					while((*p1==' '||*p1=='\t')){//过滤命令中段的空白符
						*p1='\0';
						p1++;
					}
					if(*p1==0){
						break;
					}
					i1++;
					my_argv1[i1]=p1;
				}
				else{
					p1++;
				}
			}
			int fd[2];
			pipe(fd);
			pid_t id=fork();
			if(id==0){//子进程
				close(fd[0]);
				dup2(fd[1],1);
				close(fd[1]);
				if(strcmp(my_argv0[0],"cd")==0){
					chdir(my_argv0[1]);
				}
				else{
					execvp(my_argv0[0],my_argv0);
				}
			}
			else{//父进程
				pid_t iid=fork();
				if(iid==0){
					dup2(fd[0],0);
					close(fd[1]);
					close(fd[0]);
					if(strcmp(my_argv1[0],"cd")==0){
						chdir(my_argv1[1]);
					}
					else{
						execvp(my_argv1[0],my_argv1);
					}
				}
				else{
					close(fd[0]);
					close(fd[1]);
					pid_t ret=waitpid(id,NULL,0);
					pid_t reta=waitpid(iid,NULL,0);
				}
				continue;
			}	
		}
	
		
		char *p=buf;
		int i=0;
		char * my_argv[64]={0};//初始化参数表
		while((*p==' '||*p=='\t')){//过滤命令前面的空白符
			p++;
		}
		my_argv[0]=p;
		while(*p!=0){//拆分命令于参数表
			if(*p==' '||*p=='\t'){
				while((*p==' '||*p=='\t')){//过滤命令中段的空白符
					
					*p='\0';
					p++;
				}
				if(*p==0){
					break;
				}
				i++;
				my_argv[i]=p;
			}
			else{
				p++;
			}
		}
		pid_t id=fork();   
		if(id==0){//子进程
			if(strcmp(my_argv[0],"cd")==0){
				chdir(my_argv[1]);
			}
			else{
				execvp(my_argv[0],my_argv);
			}
		}
		else{//父进程
			pid_t ret=waitpid(id,NULL,0);
		}
	}
return 0;
}
