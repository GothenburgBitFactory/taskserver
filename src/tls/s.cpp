#include <stdio.h>
#include <TLSServer.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <gnutls/gnutls.h>

#define KEYFILE  "pki/server.key.pem"
#define CERTFILE "pki/server.cert.pem"
#define CAFILE   "pki/ca.cert.pem"
#define CRLFILE  "pki/server.crl.pem"

// This is a sample TLS 1.0 echo server, using X.509 authentication.

#define SOCKET_ERR(err,s) if(err==-1) {perror(s);return(1);}
#define MAX_BUF 1024
#define PORT 5556
#define DH_BITS 1024

// These are global.
gnutls_certificate_credentials_t x509_cred;
gnutls_priority_t priority_cache;
static gnutls_dh_params_t dh_params;

int main (void)
{
  TLSServer server;

  gnutls_global_init ();
  gnutls_certificate_allocate_credentials (&x509_cred);
  gnutls_certificate_set_x509_trust_file (x509_cred, CAFILE, GNUTLS_X509_FMT_PEM);
  gnutls_certificate_set_x509_crl_file (x509_cred, CRLFILE, GNUTLS_X509_FMT_PEM);
  gnutls_certificate_set_x509_key_file (x509_cred, CERTFILE, KEYFILE, GNUTLS_X509_FMT_PEM);
  gnutls_dh_params_init (&dh_params);
  gnutls_dh_params_generate2 (dh_params, DH_BITS);
  gnutls_priority_init (&priority_cache, "NORMAL", NULL);
  gnutls_certificate_set_dh_params (x509_cred, dh_params);

  // Socket operations
  int listen_sd = socket (AF_INET, SOCK_STREAM, 0);
  SOCKET_ERR (listen_sd, "socket");

  struct sockaddr_in sa_serv;
  memset (&sa_serv, '\0', sizeof (sa_serv));
  sa_serv.sin_family = AF_INET;
  sa_serv.sin_addr.s_addr = INADDR_ANY;
  sa_serv.sin_port = htons (PORT);      // Server Port number

  int optval = 1;
  setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof (int));

  int err = bind (listen_sd, (struct sockaddr *) & sa_serv, sizeof (sa_serv));
  SOCKET_ERR (err, "bind");
  err = listen (listen_sd, 1024);
  SOCKET_ERR (err, "listen");

  printf ("s: Server ready. Listening to port '%d'.\n\n", PORT);

  struct sockaddr_in sa_cli;
  int client_len = sizeof (sa_cli);
  for (;;)
  {
    gnutls_session_t session;
    gnutls_init (&session, GNUTLS_SERVER);
    gnutls_priority_set (session, priority_cache);
    gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, x509_cred);

    // Request client certificate if any.
    gnutls_certificate_server_set_request (session, GNUTLS_CERT_REQUEST);
    //gnutls_certificate_server_set_request (session, GNUTLS_CERT_REQUIRE);

    // Set maximum compatibility mode. This is only suggested on public
    // webservers that need to trade security for compatibility
    gnutls_session_enable_compatibility_mode (session);

    int sd = accept (listen_sd, (struct sockaddr *) &sa_cli, (socklen_t*) &client_len);
    char topbuf[512];
    printf ("s: - connection from %s, port %d\n",
            inet_ntop (AF_INET, &sa_cli.sin_addr, topbuf,
                       sizeof (topbuf)), ntohs (sa_cli.sin_port));

    gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);

    // Key exchange.
    int ret = gnutls_handshake (session);
    if (ret < 0)
    {
      close (sd);
      gnutls_deinit (session);
      fprintf (stderr, "s: *** Handshake has failed (%s)\n\n",
               gnutls_strerror (ret));
      continue;
    }
    printf ("s: - Handshake was completed\n");

    // See the Getting peer's information example print_info(session);

    for (;;)
    {
      char buffer[MAX_BUF + 1];
      memset (buffer, 0, MAX_BUF + 1);
      ret = gnutls_record_recv (session, buffer, MAX_BUF);

      if (ret == 0)
      {
        printf ("\ns: - Peer has closed the GnuTLS connection\n");
        break;
      }
      else if (ret < 0)
      {
        fprintf (stderr, "\ns: *** Received corrupted "
                 "data(%d). Closing the connection.\n\n", ret);
        break;
      }
      else if (ret > 0)
      {
        // Echo data back to the client
        gnutls_record_send (session, buffer, strlen (buffer));
      }
    }
    printf ("\n");

    // Do not wait for the peer to close the connection.
    gnutls_bye (session, GNUTLS_SHUT_WR);

    close (sd);
    gnutls_deinit (session);
  }

  close (listen_sd);

  gnutls_certificate_free_credentials (x509_cred);
  gnutls_priority_deinit (priority_cache);
  gnutls_global_deinit ();
  return 0;
}

