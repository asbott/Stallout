#ifdef _ST_OS_WINDOWS
	#ifdef _ST_EXPORT
		#define ST_API __declspec(dllexport)
	#else
		#define ST_API __declspec(dllimport)
	#endif
#else
	#define ST_API 
#endif