
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "include/controler.h"
#include "include/config.h"


void main_app_debug_mode()
{
	controler * controler_ref = controler_new(IP_ADDRESS, RACK, SLOT, DB_INDEX);
	controler_handler(controler_ref);
}


void main_app()
{
#if defined(__WIN32) || defined(__WIN64)
	HWND var = GetConsoleWindow(); // @suppress("Type cannot be resolved")

	ShowWindow(var, SW_HIDE); // @suppress("Symbol is not resolved")

	main_app_debug_mode();

	ShowWindow(var, SW_SHOW); // @suppress("Symbol is not resolved")

#endif
}

void print_glass_info_addresses()
{
	printf("glass info size - %I64d\n", sizeof(glass_info));
	printf("JOB_NUMBER_BYTE_INDEX - %d\n", JOB_NUMBER_BYTE_INDEX);
	printf("CALLOFF_DATE_TIME_BYTE_INDEX - %d\n", CALLOFF_DATE_TIME_BYTE_INDEX);
	printf("VEHICLE_NUMBER_BYTE_INDEX - %d\n", VEHICLE_NUMBER_BYTE_INDEX);
	printf("REAR_WINDOW_TYPE_BYTE_INDEX - %d\n", REAR_WINDOW_TYPE_BYTE_INDEX);
	printf("VEHICLE_MODEL_BYTE_INDEX - %d\n", VEHICLE_MODEL_BYTE_INDEX);
	printf("ID_BYTE_INDEX - %d\n", ID_BYTE_INDEX);
	printf("PRODUCTION_SEQ_BYTE_INDEX - %d\n", PRODUCTION_SEQ_BYTE_INDEX);
}

int main(int argv, char ** argc)
{
	//print_glass_info_addresses();

#if defined(__WIN32) || defined(__WIN64)
	if((argv <= 1) || ((argv > 1) && (strcmp(argc[1], "-d") != 0)))
		main_app();
	else
		main_app_debug_mode();

#else
	main_app_debug_mode();
#endif

	return 0;
}

