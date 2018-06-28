#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pwd.h>

#define die(...) dprintf(old2, __VA_ARGS__), exit(125)

#define usage() die("\
Usage:\n\
\n\
    attach [-user] /dev/ttyX program [args]\n\
\n\
Run the program with stdin/stderr/stderr attached to the specified tty.\n\
\n\
Must run as root but if '-user' given then drop privileges to specified 'user'\n\
before starting program.\n\
");

int main(int argc, char *argv[])
{
    char *user, *device=NULL, **prog;
    int tty;

    // Clone existing stderr for die(), with close-on-exec
    int old2=dup(2);
    fcntl(old2, FD_CLOEXEC);

    // Parse params
    if (argc < 3) usage();
    if (*argv[1] == '-')             
    {
        if (argc < 4) usage();
        user=argv[1]+1; // skip leading '-'
        device=argv[2];
        prog=&argv[3];
    } else {
        user=NULL;
        device=argv[1];
        prog=&argv[2];
    }

    // Become session leader
    if (setsid() < 0) die("Can't setsid: %s\n", strerror(errno));
        
    // Close stdio, then reopen for specified device
    close(0); close(1); close(2);
    tty=open(device, O_RDWR);   // should open as stdin 
    if (tty != 0) die("Can't open %s: %s\n", device, strerror(errno));
    if (!isatty(tty)) die("%s is not a tty\n", device);
    dup(tty); dup(tty);         // dup to stdout and stderr
        
    // Set as controlling tty for this process
    if (ioctl(tty, TIOCSCTTY, 0) < 0) die("Can't use %s: %s\n", device, strerror(errno));
        
    // Drop perms if needed
    if (user) 
    {
        struct passwd * p = getpwnam(user);
        if (!p) die("Can't get user %s: %s\n", user, strerror(errno));
        if (setreuid(p->pw_uid, p->pw_gid)) die("Can't set user %s: %s\n", user, strerror(errno));
    }    
     
    // Finally, exec the program
    execv(*prog, prog);

    // Oops
    die("Can't exec %s: %s\n", *prog, strerror(errno));
}
