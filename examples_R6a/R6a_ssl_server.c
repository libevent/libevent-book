/* Simple echo server using OpenSSL bufferevents */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

static char private_key[] =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXQIBAAKBgQC1XaK4b4MnyIzuDgphD1ySx60vgNuGU4ZAST4X23/Iz9yEwRBR\n"
    "b7CCfKbbRGFT0Y76SiWT/bWmTCpJcOxbrr+nr08YMN4vOweyHLN8UuFb/a054ONx\n"
    "W/YI2FzmXymn9ndulP53hlhTbiCbVXBMaF+JD6iiafSxoyk7fNp0yu8G4QIDAQAB\n"
    "AoGAOkzmQNl/1KsQOnoC9h5lCL3tOwb2CmERF9szfaHOmvPOlFInd7YqjFebn4KE\n"
    "stf7WRO6rq7w1ItJUKBfKj2rV6ZkyCsToPlSroRJhKz58UZrRxpzoAVNYf2qe6T8\n"
    "kn77l3PKDRS8aZvML/6IUBY+YLjA6VSAzWq6/RDlyt54zd0CQQDaUY7OqZjsLBmn\n"
    "VUEO+6etZPIkdafpVPIJ95gfibC+j4Y/yRKc9xGPnnOUAfDni639Vxj+iQgPUtPr\n"
    "HfkQPi7bAkEA1KtPtp0XWPv+crf1zNCw5QM0HUTkAyUEP/8LlzlQemMF+nc1m43s\n"
    "O8mSyo2j+zz7B2klg/DrQ10ufn1bqF638wJAOR4BWLwyUAexpn+9h6f7VHgGiddm\n"
    "WLtqn4Txfb7OSOzP2LxIbFyPcZ2o614eotV+bbttxJohS7EF1IuA7+j05QJBAL3B\n"
    "jYK3cFmpn0Pk+KEjpHLzBNEI4xobMUuY2lK4hSJusKrnKyH85EgEd8Hb1/EYXDQk\n"
    "kaDEmmalAhNC70GwIhkCQQCRN5geEhYzN+CaxVYPph99WetPYsTGN1qJPbpK+5n1\n"
    "p8dYihTLZ/c1QREQbGXcDgaQdTMvckc0PuO/Hhl+reLh\n"
    "-----END RSA PRIVATE KEY-----\n";

static char certificate[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDeDCCAuGgAwIBAgIJAKef6j1Xr5X0MA0GCSqGSIb3DQEBBQUAMIGFMQswCQYD\n"
    "VQQGEwJBVTETMBEGA1UECBMKU29tZS1TdGF0ZTEhMB8GA1UEChMYSW50ZXJuZXQg\n"
    "V2lkZ2l0cyBQdHkgTHRkMQ4wDAYDVQQLEwVTaWxseTENMAsGA1UEAxMEZGVycDEf\n"
    "MB0GCSqGSIb3DQEJARYQc29ja2V0QGdtYWlsLmNvbTAeFw0xMTA2MTUyMTE5NTha\n"
    "Fw0xMjA2MTQyMTE5NThaMIGFMQswCQYDVQQGEwJBVTETMBEGA1UECBMKU29tZS1T\n"
    "dGF0ZTEhMB8GA1UEChMYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMQ4wDAYDVQQL\n"
    "EwVTaWxseTENMAsGA1UEAxMEZGVycDEfMB0GCSqGSIb3DQEJARYQc29ja2V0QGdt\n"
    "YWlsLmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAtV2iuG+DJ8iM7g4K\n"
    "YQ9cksetL4DbhlOGQEk+F9t/yM/chMEQUW+wgnym20RhU9GO+kolk/21pkwqSXDs\n"
    "W66/p69PGDDeLzsHshyzfFLhW/2tOeDjcVv2CNhc5l8pp/Z3bpT+d4ZYU24gm1Vw\n"
    "TGhfiQ+oomn0saMpO3zadMrvBuECAwEAAaOB7TCB6jAdBgNVHQ4EFgQUcerX8Brq\n"
    "OyK7htSk6lFKkOif5zswgboGA1UdIwSBsjCBr4AUcerX8BrqOyK7htSk6lFKkOif\n"
    "5zuhgYukgYgwgYUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIEwpTb21lLVN0YXRlMSEw\n"
    "HwYDVQQKExhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxDjAMBgNVBAsTBVNpbGx5\n"
    "MQ0wCwYDVQQDEwRkZXJwMR8wHQYJKoZIhvcNAQkBFhBzb2NrZXRAZ21haWwuY29t\n"
    "ggkAp5/qPVevlfQwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQAHRweL\n"
    "z5zE70Nd6eBkuEvrcrx9QM/f51PI+zObVD+xx95kG6IQr4kTqsizOB2z64l7chly\n"
    "sA/X7E7tRjh2q403FsCZNPOzozrrVnGq9fT1LsyfTlE8Rojp5zOCGcedI5ZBc9Tx\n"
    "hZ4Y0qGytoq1Hl61WK5ebiUXtrzUeX37tP0kZw==\n"
    "-----END CERTIFICATE-----";


static void
ssl_readcb(struct bufferevent * bev, void * arg)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    printf("Received %zu bytes\n", evbuffer_get_length(in));
    printf("----- data ----\n");
    printf("%.*s\n", (int)evbuffer_get_length(in), evbuffer_pullup(in, -1));

    bufferevent_write_buffer(bev, in);
}

static void
ssl_acceptcb(struct evconnlistener *serv, int sock, struct sockaddr *sa,
             int sa_len, void *arg)
{
    struct event_base *evbase;
    struct bufferevent *bev;
    SSL_CTX *server_ctx;
    SSL *client_ctx;

    server_ctx = (SSL_CTX *)arg;
    client_ctx = SSL_new(server_ctx);
    evbase = evconnlistener_get_base(serv);

    bev = bufferevent_openssl_socket_new(evbase, sock, client_ctx,
                                         BUFFEREVENT_SSL_ACCEPTING,
                                         BEV_OPT_CLOSE_ON_FREE);

    bufferevent_enable(bev, EV_READ);
    bufferevent_setcb(bev, ssl_readcb, NULL, NULL, NULL);
}

static SSL_CTX *
evssl_init(void)
{
    EVP_PKEY *pkey;
    X509 *cx509;
    BIO *pkey_bio;
    BIO *cert_bio;
    SSL_CTX  *server_ctx;

    SSL_load_error_strings();
    SSL_library_init();
    RAND_status();

    pkey_bio = BIO_new_mem_buf(private_key, -1);
    cert_bio = BIO_new_mem_buf(certificate, -1);
    pkey = PEM_read_bio_PrivateKey(pkey_bio, NULL, NULL, NULL);
    cx509 = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);

    server_ctx = SSL_CTX_new(SSLv23_server_method());

    SSL_CTX_use_certificate(server_ctx, cx509);
    SSL_CTX_use_PrivateKey(server_ctx, pkey);
    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
    SSL_CTX_set_cipher_list(server_ctx, "RC4+RSA:HIGH:+MEDIUM:+LOW");
    SSL_CTX_set_session_cache_mode(server_ctx, SSL_SESS_CACHE_OFF);

    return server_ctx;
}

int
main(int argc, char **argv)
{
    SSL_CTX *ctx;
    struct evconnlistener *listener;
    struct event_base *evbase;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */

    ctx = evssl_init();
    evbase = event_base_new();
    listener = evconnlistener_new_bind(
                         evbase, ssl_acceptcb, (void *)ctx,
                         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 1024,
                         (struct sockaddr *)&sin, sizeof(sin));

    event_base_loop(evbase, 0);

    evconnlistener_free(listener);
    SSL_CTX_free(ctx);

    return 0;
}

