/*
 * Please refer to OpenSSL documentation to verify you are doing this correctly,
 * Libevent does not guarantee this code is the complete picture, but to be used
 * only as an example.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>

pthread_mutex_t * ssl_locks;
int ssl_num_locks;

/* Implements a thread-ID function as requied by openssl */
static unsigned long
get_thread_id_cb(void)
{
    return (unsigned long)pthread_self();
}

static void
thread_lock_cb(int mode, int which, const char * f, int l)
{
    if (which < ssl_num_locks) {
        if (mode & CRYPTO_LOCK) {
            pthread_mutex_lock(&(ssl_locks[which]));
        } else {
            pthread_mutex_unlock(&(ssl_locks[which]));
        }
    }
}

int
init_ssl_locking(void)
{
    int i;

    ssl_num_locks = CRYPTO_num_locks();
    ssl_locks = malloc(ssl_num_locks * sizeof(pthread_mutex_t));
    if (ssl_locks == NULL)
        return -1;

    for (i = 0; i < ssl_num_locks; i++) {
        pthread_mutex_init(&(ssl_locks[i]), NULL);
    }

    CRYPTO_set_id_callback(get_thread_id_cb);
    CRYPTO_set_locking_callback(thread_lock_cb);

    return 0;
}

