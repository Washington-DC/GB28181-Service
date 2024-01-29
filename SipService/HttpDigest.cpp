#include "pch.h"
#include "HttpDigest.h"

//#include "MD5.h"

#include <osipparser2/osip_md5.h>

#ifdef _WIN32
//#define strcasecmp _stricmp
//#define strncasecmp _strnicmp
#endif
#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

void ToHex(IN HASH Bin, OUT HASHHEX Hex)
{
	std::stringstream ss;
	for (size_t i = 0; i < 16; i++)
	{
		ss << fmt::format("{:02x}", (unsigned char)Bin[i]);
	}

	auto text = ss.str();
	memcpy(Hex, text.c_str(), HASHHEXLEN + 1);
}

/* calculate H(A1) as per spec */
void
DigestCalcHA1(IN const char* pszAlg,
	IN const char* pszUserName,
	IN const char* pszRealm,
	IN const char* pszPassword,
	IN const char* pszNonce,
	IN const char* pszCNonce,
	OUT HASHHEX SessionKey)
{
	osip_MD5_CTX Md5Ctx;
	HASH HA1;

	osip_MD5Init(&Md5Ctx);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszUserName, (unsigned int)strlen(pszUserName));
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszRealm, (unsigned int)strlen(pszRealm));
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszPassword, (unsigned int)strlen(pszPassword));
	osip_MD5Final((unsigned char*)HA1, &Md5Ctx);
	if ((pszAlg != NULL) && strcasecmp(pszAlg, "md5-sess") == 0) {
		osip_MD5Init(&Md5Ctx);
		osip_MD5Update(&Md5Ctx, (unsigned char*)HA1, HASHLEN);
		osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
		osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce, (unsigned int)strlen(pszNonce));
		osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
		osip_MD5Update(&Md5Ctx, (unsigned char*)pszCNonce, (unsigned int)strlen(pszCNonce));
		osip_MD5Final((unsigned char*)HA1, &Md5Ctx);
	}
	ToHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void
DigestCalcResponse(IN HASHHEX HA1,     /* H(A1) */
	IN const char* pszNonce,    /* nonce from server */
	IN const char* pszNonceCount,       /* 8 hex digits */
	IN const char* pszCNonce,   /* client nonce */
	IN const char* pszQop,      /* qop-value: "", "auth", "auth-int" */
	IN int Aka, /* Calculating AKAv1-MD5 response */
	IN const char* pszMethod,   /* method from the request */
	IN const char* pszDigestUri,        /* requested URL */
	IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
	OUT HASHHEX Response
/* request-digest or response-digest */)
{
	osip_MD5_CTX Md5Ctx;
	HASH HA2;
	HASH RespHash;
	HASHHEX HA2Hex;

	/* calculate H(A2) */
	osip_MD5Init(&Md5Ctx);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszMethod, (unsigned int)strlen(pszMethod));
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszDigestUri, (unsigned int)strlen(pszDigestUri));

	if (pszQop == NULL) {
		goto auth_withoutqop;
	}
	else if (0 == strcasecmp(pszQop, "auth-int")) {
		goto auth_withauth_int;
	}
	else if (0 == strcasecmp(pszQop, "auth")) {
		goto auth_withauth;
	}

auth_withoutqop:
	osip_MD5Final((unsigned char*)HA2, &Md5Ctx);
	ToHex(HA2, HA2Hex);

	/* calculate response */
	osip_MD5Init(&Md5Ctx);
	osip_MD5Update(&Md5Ctx, (unsigned char*)HA1, HASHHEXLEN);
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce, (unsigned int)strlen(pszNonce));
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);

	goto end;

auth_withauth_int:

	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)HEntity, HASHHEXLEN);

auth_withauth:
	osip_MD5Final((unsigned char*)HA2, &Md5Ctx);
	ToHex(HA2, HA2Hex);

	/* calculate response */
	osip_MD5Init(&Md5Ctx);
	osip_MD5Update(&Md5Ctx, (unsigned char*)HA1, HASHHEXLEN);
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonce, (unsigned int)strlen(pszNonce));
	osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	if (Aka == 0) {
		osip_MD5Update(&Md5Ctx, (unsigned char*)pszNonceCount, (unsigned int)strlen(pszNonceCount));
		osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
		osip_MD5Update(&Md5Ctx, (unsigned char*)pszCNonce, (unsigned int)strlen(pszCNonce));
		osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
		osip_MD5Update(&Md5Ctx, (unsigned char*)pszQop, (unsigned int)strlen(pszQop));
		osip_MD5Update(&Md5Ctx, (unsigned char*)":", 1);
	}
end:
	osip_MD5Update(&Md5Ctx, (unsigned char*)HA2Hex, HASHHEXLEN);
	osip_MD5Final((unsigned char*)RespHash, &Md5Ctx);
	ToHex(RespHash, Response);
}
