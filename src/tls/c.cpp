#include <stdio.h>
#include <Socket.h>




#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <gnutls/gnutls.h>

// A very basic TLS client, with X.509 authentication.

#define MAX_BUF 1024
#define CAFILE "pki/client.cert.pem"

#define MSG "This is a test."

const char *PORT = "5556";
const char *SERVER = "127.0.0.1";

int main (void)
{
  Socket s;
  s.client_cert (CAFILE);

  gnutls_global_init ();
  gnutls_certificate_credentials_t xcred;
  gnutls_certificate_allocate_credentials (&xcred);
  gnutls_certificate_set_x509_trust_file (xcred, CAFILE, GNUTLS_X509_FMT_PEM);
  gnutls_session_t session;
  gnutls_init (&session, GNUTLS_CLIENT);

  // Use default priorities
  const char *err;
  int ret = gnutls_priority_set_direct (session, "NORMAL", &err);
  if (ret < 0)
  {
    if (ret == GNUTLS_E_INVALID_REQUEST)
      fprintf (stderr, "c: Syntax error at: %s\n", err);

    exit (1);
  }

  // put the x509 credentials to the current session
  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

  // connect to server
  int sd = socket (AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sa;
  memset (&sa, '\0', sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (atoi (PORT));
  inet_pton (AF_INET, SERVER, &sa.sin_addr);

  int error = connect (sd, (struct sockaddr *) & sa, sizeof (sa));
  if (error < 0)
  {
    fprintf (stderr, "c: Connect error\n");
    exit (1);
  }

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);

  // Perform the TLS handshake
  ret = gnutls_handshake (session);

  if (ret < 0)
  {
    fprintf (stderr, "c: *** Handshake failed\n");
    gnutls_perror (ret);
    goto end;
  }
  else
  {
    printf ("c: - Handshake was completed\n");
  }

  gnutls_record_send (session, MSG, strlen (MSG));

  char buffer[MAX_BUF + 1];
  ret = gnutls_record_recv (session, buffer, MAX_BUF);
  if (ret == 0)
    printf ("c: - Peer has closed the TLS connection\n");
  else if (ret < 0)
    fprintf (stderr, "c: *** Error: %s\n", gnutls_strerror (ret));
  else
  {
    printf ("c: - Received %d bytes: ", ret);
    for (int ii = 0; ii < ret; ii++)
      fputc (buffer[ii], stdout);
    fputs ("\n", stdout);

    gnutls_bye (session, GNUTLS_SHUT_RDWR);
  }

end:
  shutdown (sd, SHUT_RDWR);     // no more receptions
  close (sd);

  gnutls_deinit (session);
  gnutls_certificate_free_credentials (xcred);
  gnutls_global_deinit ();

  return 0;
}
