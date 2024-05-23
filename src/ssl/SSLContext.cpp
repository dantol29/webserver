#include "SSLContext.hpp"

SSLContext::SSLContext()
{
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (!ctx)
	{
		throw std::runtime_error("Unable to create SSL context");
	}
	SSL_CTX_set_ecdh_auto(ctx, 1);
}

SSLContext::~SSLContext()
{
	if (ctx)
	{
		SSL_CTX_free(ctx);
	}
}

SSL_CTX *SSLContext::getContext()
{
	return ctx;
}
