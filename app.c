#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

 
#define DEVICE "/dev/tadad_device"
 
int main(){
    int i, fd; //file description
    char ch, write_buf[100], read_buf[100];
 
    fd = open(DEVICE, O_RDWR);  //open for reading/writing
    if (fd == -1){
        printf("file %s either does not exist or has been locked by another process\n", DEVICE);
        exit(-1);
    }
    printf("r = read from device\nw=writing to device\nenter command: ");
    scanf("%c",&ch);
    switch(ch){
        case 'w':{
                printf("enter data: ");
                //char tmp;
                //scanf("%c",&tmp);
                scanf("%[^\n]",write_buf);
                // printf("after\n");
                write(fd, write_buf, sizeof(write_buf));
                // printf("after2\n");
                break;
            }
        case 'r':{
                read(fd, read_buf, sizeof(read_buf));
                printf("device: %s\n",read_buf);
                //int randNumber=read(fd,read_buf,sizeof(read_buf));
                //printf("The random Number: %d\n",randNumber);
                break;
            }
        default:
                printf("command not recognized\n");
                break;
    }
    close(fd);
 
    return 0;
}
