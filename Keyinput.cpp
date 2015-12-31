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
#include <pthread.h>
#include <deque>
#include <time.h>
#define EV_BUF_SIZE 16
using namespace std;
class key_msg
{
    public:
        key_msg(int va,time_t tm)
        {
            code = va;
            stime = tm;
        }
        int code;
        time_t stime;
};

void * gather_input(void *args)
{
    int sz;
    int fd;
    unsigned version;
    unsigned short id[4];                   /* or use struct input_id */
    char name[256] = "N/A";
    unsigned long i;
    char *event_path = (char *)(((void **)args)[0]);
    deque<key_msg * >* key_events_p  = (deque<key_msg *> *)(((void **)args)[1]);
    struct input_event ev[EV_BUF_SIZE]; /* Read up to N events ata time */
    if ((fd = open(event_path, O_RDONLY)) < 0) {
        fprintf(stderr,
            "ERR %d:\n"
            "Unable to open `%s'\n"
            "%s\n",
            errno, event_path, strerror(errno)
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
    /* Loop. Read event file and parse result. */
     while(1){
        sz = read(fd, ev, sizeof(struct input_event) * EV_BUF_SIZE);

        if (sz < (int) sizeof(struct input_event)) {
            fprintf(stderr,
                "ERR %d:\n"
                "Reading of `%s' failed\n"
                "%s\n",
                errno, "keyboard", strerror(errno)
            );
            goto fine;
        }
        /* Implement code to translate type, code and value */
        for (i = 0; i < sz / sizeof(struct input_event); ++i) {
            /*
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
            */
            if (ev[i].value ==1)
            {
                key_msg *tmp_store = new key_msg(ev[i].code,time((time_t *)NULL));
                key_events_p->push_back(tmp_store);

            }
        }

    }
fine:
        close(fd);
        return NULL;
}
int main(int argc, char *argv[])
{

    /* A few examples of information to gather */
    deque<key_msg *> key_events;
    pthread_t gather;
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s /dev/input/eventN\n"
            "Where X = input device number\n",
            argv[0]
        );
        return EINVAL;
    }
    void * args[2]={argv[1],&key_events};
    void * result;
    pthread_create(&gather,NULL,gather_input,args);

    while(1)
    {
        sleep(2);
        key_msg * this_dat;
        if (key_events.empty()==true)
        {
            cout<<"None Data"<<endl;
            continue;
        }
        this_dat= key_events.front();
        key_events.pop_front();

        cout<<this_dat->code<<endl;
        delete this_dat;
    }
    pthread_join(gather,&result);

    return errno;
}
