#include "display.h"

void getdisplay()
{
    struct utsname *display = (struct utsname *)malloc(sizeof(struct utsname));
    uid_t uid = getuid();
    if (uname(display) != 0)
    {
        perror("uname");
        return;
    }
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL)
    {
        perror("getpwuid");
        return;
    }
    if (strcmp(home, path) == 0)
    {
        printf(MAGENTA "<%s@%s:" RESET YELLOW"~ %s" RESET MAGENTA ">" RESET, pw->pw_name, display->nodename,fg);
    }
    else
    {
        printf(MAGENTA"<%s@%s:"RESET BLUE "%s "RESET YELLOW "%s" RESET MAGENTA ">" RESET, pw->pw_name, display->nodename, path,fg);
    }
    free(display);
    return;
}
