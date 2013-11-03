#include <iostream>
#include <TLSClient.h>

int main (void)
{
  try
  {
    // A very basic TLS client, with X.509 authentication.
    TLSClient client;
    client.debug (0);
    client.ca ("../../pki/ca.cert.pem");
    client.limit (1024);
    client.init ("../../pki/client.cert.pem",
                 "../../pki/client.key.pem");
    client.connect ("127.0.0.1", "5556");

    client.send ("This is a test.");

    std::string response;
    client.recv (response);
    std::cout << "c: '" << response << "'\n";
    client.bye ();
  }

  catch (std::string& e)
  {
    std::cout << "ERROR " << e << "\n";
  }

  catch (...)
  {
    std::cout << "ERROR\n";
  }

  return 0;
}

