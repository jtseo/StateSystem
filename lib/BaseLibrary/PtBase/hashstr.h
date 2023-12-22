#pragma once


class UniqHashStr
{
public:
	UniqHashStr( const char* _pcstr );
	~UniqHashStr();
	const UniqHashStr & operator=( const UniqHashStr & );
	friend bool operator<( const UniqHashStr & lhs, const UniqHashStr & rhs );
public:

    static int get_string_hash_code( const wchar_t *_pstr, size_t _nSize=0);
	static int get_string_hash_code( const char* _pstr, size_t _nSize=0 );
//#ifdef _DEBUG
	static void debug_hash_add(const char *_strString);
	static void debug_set_hashtext(int _nHash, const char *_strString);
	static const char *debug_get_hashtext(int _nHash);
	static void debug_release_hashtext();
	static void debug_hash_save(const char *_strFile);
//#endif
private:
	const char* pcstr_;
	mutable UINT32  uhash_;

};
#ifdef _DEBUG
#define	HASHTOSTR(hash)	UniqHashStr::debug_get_hashtext(hash)
#endif

#define STRTOHASH(str)	UniqHashStr::get_string_hash_code(str)
bool operator<( const UniqHashStr & lhs, const UniqHashStr & rhs );
