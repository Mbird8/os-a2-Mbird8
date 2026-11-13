#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"
#include "user/user.h"

static struct pinfo tab[NPROC];

static int
get(int pid, struct pinfo *out)
{
  int n = getpinfo(tab, NPROC);
  for(int i = 0; i < n; i++)
    if(tab[i].pid == pid){ *out = tab[i]; return 0; }
  return -1;
}

// Run for `dur` ticks; if sleepy, pause 1 tick between short bursts.
static void
work(int dur, int sleepy)
{
  struct pinfo me;
  int pid = getpid(), last = -1;
  int start = uptime();
  while(uptime() - start < dur){
    if(sleepy){
      pause(1);
      for(volatile int i = 0; i < 20000; i++)
        ;
    }
    if(get(pid, &me) == 0 && me.level != last){
      printf("pid %d t=%d level=%d runticks=%d\n",
             pid, uptime(), me.level, me.runticks);
      last = me.level;
    }
  }
}

int
main(int argc, char **argv)
{
  char mode = argc > 1 ? argv[1][0] : 'h';

  if(mode == 'h'){            // one hog: watch it sink, then get boosted
    work(200, 0);
  } else if(mode == 's'){     // sleeper alongside a hog
    if(fork() == 0){ work(200, 0); exit(0); }
    work(200, 1);
    wait(0);
  } else if(mode == 't'){     // two equal hogs: runticks should be similar
    int p1 = fork();
    if(p1 == 0){ for(int s = uptime(); uptime() - s < 100;) ; exit(0); }
    int p2 = fork();
    if(p2 == 0){ for(int s = uptime(); uptime() - s < 100;) ; exit(0); }
    struct pinfo a, b;
    pause(95);                // read just before they exit
    if(get(p1, &a) == 0 && get(p2, &b) == 0)
      printf("hog1 runticks=%d, hog2 runticks=%d\n", a.runticks, b.runticks);
    wait(0); wait(0);
  }
  exit(0);
}