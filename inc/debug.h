#ifndef DEBUG_H
	#ifdef DEBUG_COLOR
		#define DEBUG_RED "\033[31m"
		#define DEBUG_GREEN "\033[32m"
		#define DEBUG_YELLOW "\033[33m"
		#define DEBUG_MAGENTA "\033[35m"
		#define DEBUG_WHITE "\033[37m"
		#define DEBUG_NORMAL "\033[0m"
	
		//Automagically define DEBUG is DEBUG_COLOR is defined
		#ifndef DEBUG
			#define DEBUG
		#endif
		
	#else
		#define DEBUG_RED ""
		#define DEBUG_GREEN ""
		#define DEBUG_WHITE ""
		#define DEBUG_MAGENTA ""
		#define DEBUG_YELLOW ""
		#define DEBUG_NORMAL ""
	#endif
	
	#ifdef DEBUG
	
		#define DEBUG_PRINT( message, ... ) fprintf( stderr, DEBUG_RED "Error at " DEBUG_GREEN "%s" DEBUG_WHITE ":" DEBUG_MAGENTA "%i" DEBUG_WHITE ":" DEBUG_YELLOW " " message DEBUG_NORMAL "\n", __FILE__, __LINE__, ## __VA_ARGS__ ); 
	
	#else
	
		#define DEBUG_PRINT( ... )
	
	#endif

#endif