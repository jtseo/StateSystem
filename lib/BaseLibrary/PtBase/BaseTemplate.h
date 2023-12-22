

//*
typedef std::vector<void*, PT_allocator<void*> >						STLVpVoid;
typedef std::vector<const void*, PT_allocator<const void*> >						STLVpCVoid;
typedef std::map<int, void*, std::less<int>, PT_allocator<std::pair<const int, void*> > >	STLMnpVoid;
typedef std::map<int, int, std::less<int>, PT_allocator<std::pair<const int, int> > >		STLMnInt;
typedef std::vector<int, PT_allocator<int> >							STLVInt;
typedef std::set<int, std::less<int>, PT_allocator<int> >							STLSInt;
typedef std::set<float, std::less<float>, PT_allocator<float> >						STLSfloat;
typedef std::vector<unsigned short, PT_allocator<unsigned short> >		STLVUShort;
//typedef std::vector<PT_FIXED, PT_allocator<PT_FIXED> >				STLVFixed;
typedef std::vector<float, PT_allocator<float> >				STLVfloat;
typedef std::vector<STLVfloat, PT_allocator<STLVfloat> >		STLVStlVfloat;
typedef std::vector<unsigned char, PT_allocator<unsigned char> >		STLVcChar;
//typedef std::vector<Vector, PT_allocator<Vector> >					STLVVector3;
//typedef std::vector<Vector4, PT_allocator<Vector4> >					STLVVector4;
//typedef std::vector<Vector2, PT_allocator<Vector2> >					STLVVector2;
typedef std::basic_string<char, std::char_traits<char>, PT_allocator<char> >	STLString;
typedef std::vector < std::pair<STLString, STLString>, PT_allocator<std::pair<STLString, STLString> > > STLVPstrstr;
typedef std::vector < std::pair<float, int>, PT_allocator<std::pair<float, int> > > STLVPflotInt;
typedef std::vector < std::pair<int, int>, PT_allocator<std::pair<int, int> > > STLVPintInt;
typedef std::vector < std::pair<float, float>, PT_allocator<std::pair<float, float> > > STLVPflotFlot;

typedef std::vector<STLString*, PT_allocator<STLString*> >			STLVpString;
typedef std::vector<STLString, PT_allocator<STLString> >				STLVString;
typedef std::set<STLString, std::less<STLString>, PT_allocator<STLString> >							STLSString;
typedef std::vector<STLVString, PT_allocator<STLVString> >				STLVstlVString;
typedef std::map<int, STLString*, std::less<int>, PT_allocator<std::pair<const int, STLString*> > >	STLMnpString;
typedef std::map<int, STLVString*, std::less<int>, PT_allocator<std::pair<const int, STLVString*> > >	STLMnpstlVString;
typedef std::map<int, STLVString, std::less<int>, PT_allocator<std::pair<const int, STLVString> > >	STLMnstlVString;
typedef std::map<int, STLString, std::less<int>, PT_allocator<std::pair<const int, STLString> > >		STLMnString;
typedef std::map<int, STLVInt, std::less<int>, PT_allocator<std::pair<const int, STLVInt> > >		STLMnstlVInt;
typedef std::map<const STLString, STLString, std::less<STLString>, PT_allocator<std::pair<const STLString, STLString> > >		STLMstrStr;

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, PT_allocator<wchar_t> >	STLWString;
//typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, PT_allocator<wchar_t> >	STLWString;
typedef std::vector<STLWString*, PT_allocator<STLWString*> >			STLVpWString;
typedef std::vector<STLWString, PT_allocator<STLWString> >				STLVWString;
typedef std::vector<STLVWString, PT_allocator<STLVWString> >		STLVstlVWString;
typedef std::map<int, STLWString*, std::less<int>, PT_allocator<std::pair<const int, STLWString*> > >	STLMnpWString;
typedef std::map<int, STLVWString*, std::less<int>, PT_allocator<std::pair<const int, STLVWString*> > >	STLMnpstlVWString;
typedef std::map<int, STLVWString, std::less<int>, PT_allocator<std::pair<const int, STLVWString> > >	STLMnstlVWString;
typedef std::map<int, STLWString, std::less<int>, PT_allocator<std::pair<const int, STLWString> > >		STLMnWString;
typedef std::vector < std::pair<STLWString, STLWString>, PT_allocator<std::pair<STLWString, STLWString> > > STLVPwstrstr;

typedef std::vector<STLMnpVoid, PT_allocator<STLMnpVoid> >							STLVStlMpVoid;
typedef std::map<int, STLVpVoid, std::less<int>, PT_allocator<std::pair<const int, STLVpVoid> > >		STLMnStlVpVoid;
typedef std::map<int, STLMnpVoid*, std::less<int>, PT_allocator<std::pair<const int, STLMnpVoid*> > >	STLMnpstlMnpVoid;
typedef std::vector<STLVInt, PT_allocator<STLVInt> >									STLVStlVInt;
typedef std::vector<STLVpVoid, PT_allocator<STLVpVoid> >									STLVStlVpVoid;
typedef std::vector<std::pair<int, int>, PT_allocator<std::pair<int, int> > >		STLVPairIntInt;
typedef std::vector<std::pair<int, void*>, PT_allocator<std::pair<int, void*> > >		STLVPairIntPoint;

/*/

typedef std::vector<void*>				STLVpVoid;
typedef std::map<int, void*>			STLMnpVoid;
typedef std::map<int, int >				STLMnInt;
typedef std::vector<int>				STLVInt;
typedef std::vector<unsigned short>		STLVUShort;
typedef std::vector<float>				STLVfloat;
typedef std::vector<unsigned char>		STLVcChar;
//typedef std::basic_string<char, std::char_traits<char>, PT_allocator<char> >	STLString;
typedef std::string						STLString;
typedef std::vector<STLString*>			STLVpString;
typedef std::vector<STLString>			STLVString;
typedef std::map<int, STLString*>	STLMnpString;
typedef std::map<int, STLVString*>	STLMnpstlVString;
typedef std::map<int, STLVString>	STLMnstlVString;
typedef std::map<int, STLString>	STLMnString;

typedef std::wstring	STLWString;
typedef std::vector<STLWString*>			STLVpWString;
typedef std::vector<STLWString>				STLVWString;
typedef std::map<int, STLWString*>			STLMnpWString;
typedef std::map<int, STLVWString*>			STLMnpstlVWString;
typedef std::map<int, STLVWString>			STLMnstlVWString;
typedef std::map<int, STLWString>			STLMnWString;

typedef std::vector<STLMnpVoid>						STLVStlMpVoid;
typedef std::vector<STLVInt>						STLVStlVInt;
typedef std::vector<STLVpVoid>						STLVStlVpVoid;

//*/
