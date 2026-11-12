#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"
#include "user/user.h"

static struct pinfo tab[NPROC];

// Look up pid in the process table. Returns 0 and fills *out on success.
static int
find(int pid, struct pinfo *out)
{
  int n = getpinfo(tab, NPROC);
  if(n < 0)
    return -1;
  for(int i = 0; i < n; i++){
    if(tab[i].pid == pid){
      *out = tab[i];
      return 0;
    }
  }
  return -1;
}

int
main(void)
{
  struct pinfo me, child;
  int mypid = getpid();

  // Test 1: spin for ~20 ticks, then check we were charged about that much.
  int start = uptime();
  while(uptime() - start < 20)
    ;
  if(find(mypid, &me) < 0){
    printf("FAIL: cannot find own pid %d\n", mypid);
    exit(1);
  }
  printf("self: name=%s level=%d runticks=%d nsched=%d ctime=%d\n",
         me.name, me.level, me.runticks, me.nsched, me.ctime);
  printf("expect runticks ~20 (CPUS=1), nsched > 0\n");

  // Test 2: argument checks.
  printf("getpinfo(tab, -1) = %d (expect -1)\n", getpinfo(tab, -1));
  printf("getpinfo(tab, 2)  = %d (expect 2 or fewer)\n", getpinfo(tab, 2));

  // Test 3: a new child's counters start at 0.
  int pid = fork();
  if(pid < 0){
    printf("FAIL: fork\n");
    exit(1);
  }
  if(pid == 0){
    // child: just burn a little time and exit
    int s = uptime();
    while(uptime() - s < 3)
      ;
    exit(0);
  }
  // Parent reads the child's entry right after fork, before it has run.
  if(find(pid, &child) < 0){
    printf("FAIL: cannot find child %d\n", pid);
  } else {
    printf("child at creation: runticks=%d nsched=%d level=%d ctime=%d (expect 0 0 0)\n",
           child.runticks, child.nsched, child.level, child.ctime);
  }
  wait(0);
  exit(0);
}