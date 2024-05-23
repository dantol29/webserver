#ifndef SSL_CONTEXT_HPP
#define SSL_CONTEXT_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>

class SSLContext
{
  public:
	SSLContext();
	~SSLContext();

	SSL_CTX *getContext();

  private:
	SSL_CTX *ctx;
};

#endif // SSL_CONTEXT_HPP
