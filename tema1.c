/*   posibil sa fi inclus prea multe biblioteci dar am vrut sa merg la sigur =)))   */
#include <stdio.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>  
#include <string.h>   
#include <time.h>
#include <fcntl.h>
#include <pwd.h>
#include <dirent.h>
#include <grp.h>
char pathhelp[10000];
/*subprogram care ar trebui sa ia un path de start si numele fisierului introdus la linia de comanda
si sa genereze path-ul integral pana la fisier(file) si sa-l copieze in stringul global pathhelp de care voiam sa
ma folosesc in programul principal
am vrut sa folosesc acest subprogram la comanda my file dar din nefericire nu functioneaza comanda myfind [  :(((  ]
pentru acest subprogram m-am inspirat de pe 
---https://stackoverflow.com/questions/8149569/scan-a-directory-to-find-files-in-c---
*/
void dir_search(char path[],char file[])
{
    struct dirent *dir;
    DIR *dp=opendir(path);
    if(dp)
    {
        while((dir=readdir(dp)) != NULL)
        {
            if(dir->d_type==DT_DIR)
            {
                if(strcmp(dir->d_name,".") && strcmp(dir->d_name,".."))
                {
                    char finalpath[200];
                    strcpy(finalpath,path);
                    strcat(finalpath,"/");
                    strcat(finalpath,dir->d_name);
                }
            }
            if(strcmp(file,dir->d_name))
            {
                char cpy[10000];
                strcpy(cpy,path);
                strcat(cpy,"/");
                strcat(cpy,file);
                strcpy(pathhelp,cpy);
            }
        }
    }
}
char loggeduser[50];
//In stringul global logged user vom retine userul care va fi logat dupa apelul comenzii login!
int main(void)
{
    printf("To use this program please try some of this commands!\n");
    printf("\n..................\n\n");
    printf("\n<login>\n<mystat>\n<myfind>\n<quit>\n<who>\n<disconnect>\n\n");
    printf("..................\n\n");
    printf("For more details about a specific command try 'man <command>'");
    printf("\n\n.................\n\n");
    char command[100];//Stringul care retine comanda apelata!
    strcpy(loggeduser,"No user logged in!\n");
    while(1){
        if(!strcmp(loggeduser,"No user logged in!\n"))printf("guest@command_line-> ");
        else printf("%s@command_line-> ",loggeduser);
        scanf("%s",command);
        if(strncmp(command,"quit",4)==0)//comanda quit va incheia executia programului
        {
          printf("Programul s-a incheiat la apelul comenzii <%s>! \n",command);
          exit(1);
          wait(NULL);
        }
        else if(strncmp(command,"disconnect",10)==0)
        {//comanda diconnect va deconecta userul logat si va permite din nou executia comenzii <login>
           if(strcmp(loggeduser,"No user logged in!\n"))
            {
                printf("\n%s has been disconnected\n",loggeduser);
                strcpy(loggeduser,"No user logged in!\n");//se goleste stringul global "loggeduser"
            }
            else printf("%s",loggeduser);
            wait(NULL);
        }
        else if(strncmp(command,"who",3)==0)
        {
            //comanda who afiseaza userul conectat la momentul in care este apelata
            printf("\n%s\n\n",loggeduser);
            wait(NULL);
        }
        /*In comanda <mystat> folosim path-ul care e dat de la linia de comanda si returnam informatiile despre fisierul 
        pe care il cautam (in caz ca exista)
        Pentru aceasta comanda am decis ca pathul sa se citeasca in parinte si sa fie trimis in copil, in copil se executa
        interogarile legate de fisierul trimis prin path ,se genereaza rezultatul final(informatiile) tot in procesul copil
        iar rezultatul final e trimis inapoi la parinte unde este si afisat
        Pentru comunicarea intre procese la acest am ales doua fisiere de tip FIFO respectiv mystat_in si mystat_out
        Surse de inspiratie--->https://pubs.opengroup.org/onlinepubs/007908799/xsh/sysstat.h.html
                           --->man 2 stat
                           --->Coduri discutate la laborator
                        
        */
        else if(strncmp(command,"mystat",6)==0)
        {
            wait(NULL);
            if(mkfifo("mystat_in",0777)<0)
            {
                if(errno != EEXIST)
                {
                    perror("Error in creating FIFO file!\n");
                    exit(0);
                }
            }
            if(mkfifo("mystat_out",0777)<0)
            {
                if(errno != EEXIST)
                {
                    perror("Error in creating FIFO file!\n");
                    exit(0);
                }
            }
            pid_t child=fork();
            if(child<0)
            {
                perror("Erron on making child process!\n");
                exit(0);
            }
            if(child>0)
            {//Parent process
                int fd=open("mystat_in",O_WRONLY);
                char path[50];
                scanf("%s",path);
                if(write(fd,path,strlen(path)+1)<0)
                {
                    perror("Error on writing in file descriptor (in)\n");
                    exit(0);
                }
                close(fd);
                int fd1=open("mystat_out",O_RDONLY);
                char cpy[200];
                if(read(fd1,cpy,200)<0)
                {
                    perror("Error on reading from file descriptor (out)\n");
                    exit(0);
                }
                strcat(cpy,"\n");
                printf("%s",cpy);
                close(fd1);
            }
            else if(child==0)
            {//Child Process
                int fd=open("mystat_in",O_RDONLY);
                char path[50];

                if(read(fd,path,50)<0)
                {
                    perror("Error on reading from file descriptor (in)\n");
                }
                struct stat mystat;
                struct passwd *pwd;
                struct group *gr;
                char rezfinal[200]="\0";
                close(fd);
                if(lstat(path,&mystat)<0)
                {
                    printf("Error at stat for %s\n",path);
                    exit(0);
                }
                switch(mystat.st_mode & S_IFMT)//cod inspirat din "man 2 stat"
                {
                    case S_IFDIR : strcpy(rezfinal, "Director ");break;
                    case S_IFREG : strcpy(rezfinal, "Normal file ");break;
                    case S_IFLNK : strcpy(rezfinal, "Link ");break;
                    case S_IFIFO : strcpy(rezfinal, "FIFO ");break;
                    case S_IFSOCK : strcpy(rezfinal, "Socket ");break;
                    case S_IFBLK : strcpy(rezfinal, "Block device ");break;
                    case S_IFCHR : strcpy(rezfinal, "Character device ");break;
                    default: strcpy(rezfinal, "Unknown ");
                }
                strcat(rezfinal,"\n");
                char octal[10];
                sprintf(octal,"%o",mystat.st_mode & 0777);
                strcat(rezfinal,"Octal perms(");
                strcat(rezfinal,octal);
                strcat(rezfinal,")\n");
                strcat(rezfinal,"permissions(");
                if( S_IRUSR & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWUSR & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXUSR & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-");
                if( S_IRGRP & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWGRP & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXGRP & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-");
                if( S_IROTH & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWOTH & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXOTH & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-"); 
                strcat(rezfinal,"-");              
                strcat(rezfinal,")\n");
                char memory[1000],dimension[100],hl[10];
                sprintf(hl,"%ld",mystat.st_nlink);
                strcat(rezfinal,"Number of Hardlinks(");
                strcat(rezfinal,hl);
                strcat(rezfinal,")\n");
                sprintf(memory,"%ld",mystat.st_blocks);
                strcat(rezfinal,"Number of sectors(");
                strcat(rezfinal,memory);
                strcat(rezfinal,")\n");
                sprintf(dimension,"%ld",mystat.st_size);
                strcat(rezfinal,"Dimenssion(");
                strcat(rezfinal,dimension);
                strcat(rezfinal,"bytes)\n");
                pwd=getpwuid(mystat.st_uid);
                if(pwd)
                {
                    strcat(rezfinal,"Owner(");
                    strcat(rezfinal,pwd->pw_name);
                    strcat(rezfinal,") UID(");
                    char cnv[10];
                    sprintf(cnv,"%ld",(long)mystat.st_uid);
                    strcat(rezfinal,cnv);
                    strcat(rezfinal,")\n");
                }
                else 
                {
                    strcat(rezfinal,"UID(");
                    char cnv[10];
                    sprintf(cnv,"%ld",(long)mystat.st_uid);
                    strcat(rezfinal,cnv);
                    strcat(rezfinal,")\n");

                }
                gr=getgrgid(mystat.st_gid);
                if(gr)
                {
                    strcat(rezfinal,"Group-Owner(");
                    strcat(rezfinal,gr->gr_name);
                    strcat(rezfinal,") GID(");
                    char cnv[10];
                    sprintf(cnv,"%ld",(long)mystat.st_gid);
                    strcat(rezfinal,cnv);
                    strcat(rezfinal,")\n");
                }
                else
                {
                    strcat(rezfinal,"GID(");
                    char cnv[10];
                    sprintf(cnv,"%ld",(long)mystat.st_gid);
                    strcat(rezfinal,cnv);
                    strcat(rezfinal,")\n");
                }
                /*In rezultatul final vor fi trecute 
                -tipul fisierului
                -permisiuni in octal
                -un string cu permisiunile
                -numarul de hardlink-uri
                -numarul de sectopare
                -Dimensiunea in octeti
                -Proprietarul si UID-ul
                -Grupul proprietar si GID-ul*/
                int fd1=open("mystat_out",O_WRONLY);
                if(write(fd1,rezfinal,strlen(rezfinal)+1)<0)
                {
                    perror("Error on writing in file descriptor (out)\n");
                    exit(0);
                }
                close(fd1);
                exit(0);
            }
            remove("myfifo_in");
            remove("myfifo_out");
        }
        /*Pentru comanda login interogam continutul fisierului users.txt si verifica daca userul dar ca parametru la linia 
        de comanda se gaseste in fisier.
        In caz afirmativ daca nu este alt user conectat acesta va deveni noul user ce executa comenzi in program
        In caz negativ se va transmite un mesaj in care scrie ca alt user executa comenzi in momentul de fata
        si sa se astepte deconectarea user-ului curent
        Ca si comunicatie intre procese e in pricipu la fel ca la mystat doar ca in loc de fisiere de tip FIFO
        folosesc doi descriptori de tip pipe()
        Surse de inspiratie--->Coduri de la laborator
        */
        else if(strncmp(command,"login",5)==0)
        {
            int fd1[2],fd2[2];
            if(pipe(fd1)<0)
            {
                perror("Error creating 1st pipe!\n");
                exit(0);
            }
            if(pipe(fd2)<0)
            {
                perror("Error creating 2nd pipe!\n");
                exit(0);
            }
            pid_t pid=fork();
            if(pid<0)
            {
                perror("Error creating child process!\n");
                exit(2);
            }
            if(pid>0)
            {//parent process
                close(fd1[0]);
                char cpy[50],rtcd[50];
                scanf("%s",cpy);
                cpy[strlen(cpy)]= '\0';
                if(write(fd1[1],cpy,sizeof(char)*(strlen(cpy)+1))<0)
                {
                    perror("Error on writing in file descriptor (1st pipe)\n");
                    exit(4);
                }
                close(fd1[1]);
                wait(NULL);//Parintele asteapta ca toti copii sa-si incheie executia
                close(fd2[1]);
                if(read(fd2[0],rtcd,50)<0)
                {
                                    
                    perror("Error on reading in file descriptor (2nd pipe)\n");
                    exit(4);
                }
                if(strcmp(loggeduser,"No user logged in!\n"))
                {
                    printf("\n%s is already connected\n",loggeduser);
                    printf("If you want to switch the user type 'disconnect' then 'login'\n\n");
                }
                else
                {
                    if(strcmp(rtcd,"User doesn't exists!"))
                    {
                        printf("%s logged in sucessfuly!\n",rtcd);
                        strcpy(loggeduser,rtcd);
                    }
                    else printf("%s\n",rtcd);
                    close(fd2[0]);
                }
            }
            else if(pid==0)
            {//child process
                close(fd1[1]);
                char cpy[50];
                if(read(fd1[0],cpy, 50)<0)
                {
                    perror("Error on reading from file descriptor (1nd pipe)\n");
                    exit(6);
                }
                char *filepath = "users.txt";
                bool infile = false;
                char *line = NULL;
                size_t len = 0;
                ssize_t read1;
                FILE *sysusers = fopen(filepath,"r");
                if(sysusers==NULL)
                {
                    perror ("Error opening file");
                    exit(7);
                }
                while ((read1 = getline(&line, &len, sysusers)) != -1)
                {//se va citi in line din fisierul users.txt transmis prin sysusers fiecare user din fisier
                    line[strcspn(line, "\n")] = '\0';
                    if (!strcmp(line, cpy)) //se compara userul trimis ca parametru venit din parinte cu fiecare linie din fisier
                    {
                        strcpy(loggeduser,cpy);
                        infile = true;
                        break;
                    }
                }
                fclose(sysusers);
                if (line)free(line);
                if(infile!=true)strcpy(cpy,"User doesn't exists!");//in caz ca userul nu exista in fisier se va transmite mesajul dat
                close(fd1[0]);
                close(fd2[0]);
                if(write(fd2[1],cpy,strlen(cpy)+1)<0)
                {
                    perror("Error on writing in file descriptor (2nd pipe)\n");
                    exit(0);
                }
                exit(0);
                printf("Chlild still exists");
            }
        }
        /*In principiu comenzile myfind si mystat lucreaza la fel
        Singura diferenta este aceea ca se trimite numele fisierului care se cauta
        si nu path-ul sau, iar myfind ar trebui sa genereze path-ul si sa-l transmita inapoti + informatiile din fisier la 
        fel ca la mystat numai ca in loc de uid gid group-owner si owner se vor transmite 
        -ultima data a accesarii
        -ultima data a modificarii
        -ultimul status de schimbare
        +nr hardlink-uri, size, permisiuni, etc.
        Ca si comunicare intre procese am vrut sa folosesc un socketpair (care functioneaza bine) insa comanda myfind nu functioneaza
        cum trebuie din cauza ca n-am reusit sa generez path-ul pentru fisier =((((
        Surse de inspiratie---->aceleasi ca la comanda <mystat>
                           ---->aceleasi ca la subprogramul dir_search */
        else if(strncmp(command,"myfind",6)==0)
        {
            int sockets[2];
            if(socketpair(AF_UNIX,SOCK_STREAM,0,sockets)<0)
            {
                perror("Error at opening stream socketpair!\n");
                exit(0);
            }
            pid_t another_child=fork();
            if(another_child<0)
            {
                perror("Erron on making child process!\n");
                exit(0);
            }
            if(another_child)
            {//parent process
                close(sockets[0]);
                char filename[10],result[10000];
                scanf("%s",filename);
               //printf("%s\n",filename);
                if(write(sockets[1],filename,strlen(filename)+1)<0)
                {
                    perror("Error on writing to descriptor!(parent)\n");
                    exit(0);
                }
               // printf("%s\n",filename);
                wait(NULL);
                if(read(sockets[1],result,10000)<0)
                {
                    perror("Error on reading from descriptor (parent)!\n");
                    exit(1);
                }
                //printf("%s\n",result);
                close(sockets[1]);
            }
            else if(another_child==0)
            {
                close(sockets[1]);
                char filename[10],result[200];
                if(read(sockets[0],filename,10)<0)
                {
                    perror("Error on reading from descriptor (child)!\n");
                    exit(0);
                }
                printf("%s\n",filename);
                dir_search("./",filename);
                char path[10000];
                strcpy(path,pathhelp);
                struct stat mystat;
                struct passwd *pwd;
                struct group *gr;
                char rezfinal[200]="\0";
                if(lstat(path,&mystat)<0)
                {
                    printf("Error at stat for %s\n",path);
                    exit(0);
                }
                switch(mystat.st_mode & S_IFMT)
                {
                    case S_IFDIR : strcpy(rezfinal, "Director ");break;
                    case S_IFREG : strcpy(rezfinal, "Normal file ");break;
                    case S_IFLNK : strcpy(rezfinal, "Link ");break;
                    case S_IFIFO : strcpy(rezfinal, "FIFO ");break;
                    case S_IFSOCK : strcpy(rezfinal, "Socket ");break;
                    case S_IFBLK : strcpy(rezfinal, "Block device ");break;
                    case S_IFCHR : strcpy(rezfinal, "Character device ");break;
                    default: strcpy(rezfinal, "Unknown ");
                }
                strcat(rezfinal,"Path[ ");
                strcat(rezfinal,path);
                strcat(rezfinal," ]\n");
                strcat(rezfinal,"\n");
                char octal[10];
                sprintf(octal,"%o",mystat.st_mode & 0777);
                strcat(rezfinal,"Octal perms(");
                strcat(rezfinal,octal);
                strcat(rezfinal,")\n");
               strcat(rezfinal,"permissions(");
                if( S_IRUSR & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWUSR & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXUSR & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-");
                if( S_IRGRP & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWGRP & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXGRP & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-");
                if( S_IROTH & mystat.st_mode) strcat(rezfinal,"r");
                else strcat(rezfinal,"-");
                if( S_IWOTH & mystat.st_mode) strcat(rezfinal,"w");
                else strcat(rezfinal,"-");
                if( S_IXOTH & mystat.st_mode) strcat(rezfinal,"x");
                else strcat(rezfinal,"-"); 
                strcat(rezfinal,"-");              
                strcat(rezfinal,")\n");
                strcat(rezfinal,")\n");
                char memory[1000],dimension[100],hl[10];
                sprintf(hl,"%ld",mystat.st_nlink);
                strcat(rezfinal,"Number of Hardlinks(");
                strcat(rezfinal,hl);
                strcat(rezfinal,")\n");
                sprintf(memory,"%ld",mystat.st_blocks);
                strcat(rezfinal,"Number of sectors(");
                strcat(rezfinal,memory);
                strcat(rezfinal,")\n");
                sprintf(dimension,"%ld",mystat.st_size);
                strcat(rezfinal,"Dimenssion(");
                strcat(rezfinal,dimension);
                strcat(rezfinal,"bytes)\n");
                strcat(rezfinal,"Last status change(");
                strcat(rezfinal,ctime(&mystat.st_ctime));
                strcat(rezfinal,")\n");
                strcat(rezfinal,"Last file access(");
                strcat(rezfinal,ctime(&mystat.st_atime));
                strcat(rezfinal,")\n");
                strcat(rezfinal,"Last file modification(");
                strcat(rezfinal,ctime(&mystat.st_mtime));
                strcat(rezfinal,")\n");
                write(sockets[0],rezfinal,strlen(rezfinal)+1);
                close(sockets[0]);
                exit(0);
            }
        }
        else
        {//daca nu se gaseste nicio comanda mentionata mai sus se afiseaza mesajul "invalid command" 
            printf("\ninvalid command\n");
            printf("try some of this commands\n<login>\n<mystat>\n<myfind>\n<quit>\n<who>\n<disconnect>\n\n");    
            wait(NULL);
        }
    }
}