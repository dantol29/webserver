#include "SSLManager.hpp"

SSLManager *SSLManager::getInstance()
{
	static SSLManager instance;
	return &instance;
}

SSLManager::SSLManager()
{
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
}

SSLManager::~SSLManager()
{
	ERR_free_strings();
	EVP_cleanup();
	// Additional cleanup can be added here
	// CRYPTO_cleanup_all_ex_data();
	// SSL_COMP_free_compression_methods();
}