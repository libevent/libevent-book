/* Includes and stub declarations to make some examples in chapter 1 work */

#include <unistd.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

int fd[1000];
int n_sockets=1000;

int i_still_want_to_read(void);

void handle_close(int fd);
void handle_error(int fd, int err);
void handle_input(int fd, char *buf, int n);
