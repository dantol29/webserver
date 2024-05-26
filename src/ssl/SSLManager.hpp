#ifndef SSL_MANAGER_HPP
#define SSL_MANAGER_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>

class SSLManager
{
  public:
	static SSLManager *getInstance();

  private:
	SSLManager();
	~SSLManager();
	SSLManager(const SSLManager &other);
	SSLManager &operator=(const SSLManager &other);
};

#endif // SSL_MANAGER_HPP
