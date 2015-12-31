#include <stdio.h>
#include <string.h>
#include <string.h>     /* strerror() */
#include <errno.h>      /* errno */

#include <fcntl.h>      /* open() */
#include <unistd.h>     /* close() */
#include <sys/ioctl.h>  /* ioctl() */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/input.h>    /* EVIOCGVERSION ++ */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#define EV_BUF_SIZE 16
using namespace std;
struct key_msg
{
    int value;
    time_t time;
};
int main(int argc, char *argv[])
{
    int fd, sz;
    unsigned i;

    /* A few examples of information to gather */
    unsigned version;
    unsigned short id[4];                   /* or use struct input_id */
    char name[256] = "N/A";
    vector<struct key_msg> key_msg_collector;
    int shmid = shmget(IPC_PRIVATE,128*sizeof(struct key_msg),IPC_CREAT|0600);
    if (shmid <0){
        printf("error in create share memory!\n");
        return -1;
    }
    struct key_msg *key_freq = (struct key_msg *)shmat(shmid,NULL,0);
    bzero(key_freq,128*sizeof(struct key_msg ));
    struct input_event ev[EV_BUF_SIZE]; /* Read up to N events ata time */

    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s /dev/input/eventN\n"
            "Where X = input device number\n",
            argv[0]
        );
        return EINVAL;
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr,
            "ERR %d:\n"
            "Unable to open `%s'\n"
            "%s\n",
            errno, argv[1], strerror(errno)
        );
    }
    /* Error check here as well. */
    ioctl(fd, EVIOCGVERSION, &version);
    ioctl(fd, EVIOCGID, id);
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);

    fprintf(stderr,
        "Name      : %s\n"
        "Version   : %d.%d.%d\n"
        "ID        : Bus=%04x Vendor=%04x Product=%04x Version=%04x\n"
        "----------\n"
        ,
        name,

        version >> 16,
        (version >> 8) & 0xff,
        version & 0xff,

        id[ID_BUS],
        id[ID_VENDOR],
        id[ID_PRODUCT],
        id[ID_VERSION]
    );
    pid_t pid = fork();
    if (pid==0){
        int status;
        while(1){
            sleep(1);
            printf("%ld\n",key_freq[48]);
        }
        waitpid(pid,&status,0);
    }else{

    struct key_msg *key_freq = (struct key_msg *)shmat(shmid,NULL,0);
    /* Loop. Read event file and parse result. */
    for (;;) {
        sz = read(fd, ev, sizeof(struct input_event) * EV_BUF_SIZE);

        if (sz < (int) sizeof(struct input_event)) {
            fprintf(stderr,
                "ERR %d:\n"
                "Reading of `%s' failed\n"
                "%s\n",
                errno, argv[1], strerror(errno)
            );
            goto fine;
        }

        /* Implement code to translate type, code and value */
        for (i = 0; i < sz / sizeof(struct input_event); ++i) {
            fprintf(stderr,
                "%ld.%06ld: "
                "type=%02x "
                "code=%02x "
                "value=%02x\n",
                ev[i].time.tv_sec,
                ev[i].time.tv_usec,
                ev[i].type,
                ev[i].code,
                ev[i].value
            );
            if(ev[i].value==1)
            {
                [ev[i].code];
                printf("%ld\n\n\n",key_freq[48]);
            }
        }
    }
    }

fine:
    close(fd);

    return errno;
}
