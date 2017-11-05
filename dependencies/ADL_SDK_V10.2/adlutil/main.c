///
///  Copyright (c) 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.c
/// \brief C/C++ ADL application to retrieve and set TimigOverride and resolution/color depth modes
///
///  Author: Borislav Zahariev


#if defined (LINUX)
#include "../include/adl_sdk.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <errno.h> // fopen_s
#include <stdlib.h>	
#include <string.h>	//memeset

// Linux equivalent of sprintf_s
#define sprintf_s snprintf
// Linux equivalent of fscanf_s
#define fscanf_s fscanf
// Linux equivalent of GetProcAddress
#define GetProcAddress dlsym

#else
#include <windows.h>
#include "..\include\adl_sdk.h"
#endif

#include <stdio.h>


typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
typedef int ( *ADL_ADAPTER_ACTIVE_GET ) ( int, int* );
typedef int ( *ADL_DISPLAY_COLORCAPS_GET ) ( int, int, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_GET ) ( int, int, int, int *, int *, int *, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_SET ) ( int, int, int, int );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET ) ( int, int *, ADLDisplayInfo **, int );
typedef int ( *ADL_DISPLAY_MODES_GET ) (int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);
typedef int ( *ADL_DISPLAY_MODES_SET ) (int iAdapterIndex, int iDisplayIndex, int lpNumModes, ADLMode * lpModes);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDE_GET) (int iAdapterIndex, int iDisplayIndex, ADLDisplayMode *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDE_SET) (int iAdapterIndex, int iDisplayIndex, ADLDisplayModeInfo *lpMode, int iForceUpdate);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET) (int iAdapterIndex, int iDisplayIndex, int iMaxNumOfOverrides, ADLDisplayModeInfo *lpModeInfoList, int *lpNumOfOverrides);
typedef int ( *ADL_ADAPTER_VIDEOBIOSINFO_GET )( int iAdapterIndex, ADLBiosInfo* lpBiosInfo );


int OpenADL( void );
void CloseADL( void );
int GetValue( char * name, int * iValue, int line );
int GetHex(  char * name, int * iValue, int line );
int GetFloat(  char * name, float * fValue, int line );
int AdlModeFromFile( LPADLMode  lpADLMode );
int AdlModeToFile( LPADLMode lpADLMode );
int AdlDisplayModeInfoFromFile(  int * lpAdapterIdx, int * lpDisplayIdx, ADLDisplayModeInfo * lpModeInfoList );
int AdlDisplayModeInfoToFile( FILE * file, int iAdapterIdx, int iDisplayIdx, ADLDisplayModeInfo * lpModeInfoList );
int AdlBiosInfoToFile( int iAdapterIndex, int iDisplayIndex, ADLBiosInfo * lpAdlBI );
int DisplayErrorAndPause( char * sError );
void ShowHelp(void);

void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}

enum	COMMAND	{ NONE, GETALL, GETMODE, SETMODE, INFOOVER, SETOVER, GETLIST, GENERATE, BIOSINFO };

#if defined (LINUX)
// Linux equivalent function of fopen_s
int fopen_s ( FILE ** file, const char *filename, const char *mode )
{
	if ( NULL == file )
		return EINVAL;

	( *file ) = fopen ( filename, mode );

	if ( NULL != *file )
		return 0;
	else
		return errno;
}

#endif

#if defined (LINUX)
    void *hDLL;		// Handle to .so library
#else
    HINSTANCE hDLL;		// Handle to DLL
#endif

    LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	LPADLMode lpADLMode = NULL;
	ADLDisplayModeInfo * lpModeInfoList = NULL;

	FILE * file = NULL;
	FILE * file2 = NULL;
	char msg[ 128 ];
	char err[ 128 ];
	int sMsg = sizeof( msg );
	int sErr = sizeof( err );

int main( int argc, char *argv[] )
{
    ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;
    ADL_ADAPTER_ACTIVE_GET           ADL_Adapter_Active_Get;
    ADL_DISPLAY_DISPLAYINFO_GET      ADL_Display_DisplayInfo_Get;
	ADL_DISPLAY_MODES_GET			ADL_Display_Modes_Get;
	ADL_DISPLAY_MODES_SET			ADL_Display_Modes_Set;
	ADL_DISPLAY_MODETIMINGOVERRIDE_GET	ADL_Display_ModeTimingOverride_Get;
	ADL_DISPLAY_MODETIMINGOVERRIDE_SET	ADL_Display_ModeTimingOverride_Set;
	ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET	ADL_Display_ModeTimingOverrideList_Get;
	ADL_ADAPTER_VIDEOBIOSINFO_GET	ADL_Adapter_VideoBiosInfo_Get;
	
    int  i, j, k;
    int  ADL_Err;
    int  iActive;
    int  iNumberAdapters;
    int  iAdapterIndex;
    int  iDisplayIndex;
    int  iNumDisplays;
    int  iNumModes;
	int  iMaxNumOfOverrides = 100;		// Adjust this if necessary, but 100 seems enough.
	int  lpNumOfOverrides;
	int command = NONE;

	ADLMode adlmode;
	ADLDisplayMode AdlDM;
	ADLDisplayModeInfo AdlDmi;
	ADLBiosInfo AdlBI;

    memset ( &AdlDM,'\0', sizeof (AdlDM));
	memset ( &AdlDmi,'\0', sizeof (AdlDmi));

	if ( argc < 2 )
	{
		ShowHelp();
		return 0;
	}

	// Get the display mode
	if ( 0 == strcmp (argv[ 1] , "get" ) )
	{
		command = GETMODE;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	// Get the Override Info
	else if ( 0 == strcmp (argv[ 1] , "info" ) )
	{
		command = INFOOVER;
		if ( 4 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
			if ( fopen_s( &file2, argv[ 3 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[3] );
				return 0;
			}
		}
		else if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
			file2 = stdout;
		}
		else
		{
			printf( "The 'info' command requires at least one file\n" );
			ShowHelp();
			return 0;
		}
	}
	// Get the Override LIst
	else if ( 0 == strcmp (argv[ 1] , "list" ) )
	{
		command = GETLIST;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	// Generate pattern
	else if ( 0 == strcmp (argv[ 1] , "gen" ) )
	{
		command = GENERATE;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	// Set mode according to the settings in a text file
	else if ( 0 == strcmp (argv[ 1] , "set" ) )
	{
		command = SETMODE;
		if ( argc < 3 )
		{
			DisplayErrorAndPause( "ERROR: The 'set' command requires a filename\n" );
			return 0;
		}
		else
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
	}
	// Set Override mode according to the settings in a text file
	else if ( 0 == strcmp (argv[ 1] , "over" ) )
	{
		command = SETOVER;
		if ( argc < 3 )
		{
			DisplayErrorAndPause( "ERROR: The 'over' command requires a filename\n" );
			return 0;
		}
		else
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
	}
	// Retrieves all modes from the system across multiple GPUs
	else 	if ( 0 == strcmp (argv[ 1] , "all" ) )
	{
		command = GETALL;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	else 	if ( 0 == strcmp (argv[ 1] , "bios" ) )
	{
		command = BIOSINFO;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	else
	{
			printf( "\nERROR: Unrecognized command!\n" );
			ShowHelp();
			return 0;
	}

			ADL_Err = OpenADL();
			// Error during ADL initialization?
			if (ADL_OK != ADL_Err )
  					return DisplayErrorAndPause( "ERROR: ADL Initialization error!" );
			// Get the function pointers from ADL:
			ADL_Display_Modes_Get = (ADL_DISPLAY_MODES_GET)GetProcAddress(hDLL,"ADL_Display_Modes_Get");
			if ( NULL == ADL_Display_Modes_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get not available!" );
			ADL_Display_Modes_Set = (ADL_DISPLAY_MODES_SET)GetProcAddress(hDLL,"ADL_Display_Modes_Set");
			if ( NULL == ADL_Display_Modes_Set )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Set not available!" );
	        ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
			if ( NULL == ADL_Adapter_NumberOfAdapters_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_NumberOfAdapters_Get not available!" );
            ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
			if ( NULL == ADL_Adapter_AdapterInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_AdapterInfo_Get not available!" );
			ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL,"ADL_Adapter_Active_Get");
			if ( NULL == ADL_Adapter_Active_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_Active_Get not available!" );
			ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL,"ADL_Display_DisplayInfo_Get");
			if ( NULL == ADL_Display_DisplayInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_DisplayInfo_Get not available!" );
			ADL_Display_ModeTimingOverride_Get = (ADL_DISPLAY_MODETIMINGOVERRIDE_GET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverride_Get");
			if ( NULL == ADL_Display_ModeTimingOverride_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Get not available!" );
			ADL_Display_ModeTimingOverride_Set = (ADL_DISPLAY_MODETIMINGOVERRIDE_SET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverride_Set");
			if ( NULL == ADL_Display_ModeTimingOverride_Set )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Set not available!" );
			ADL_Display_ModeTimingOverrideList_Get = (ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverrideList_Get");
			if ( NULL == ADL_Display_ModeTimingOverrideList_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverrideList_Get not available!" );
			ADL_Adapter_VideoBiosInfo_Get = (ADL_ADAPTER_VIDEOBIOSINFO_GET)GetProcAddress(hDLL,"ADL_Adapter_VideoBiosInfo_Get");
			if ( NULL == ADL_Adapter_VideoBiosInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_VideoBiosInfo_Get not available!" );


		if ( GETALL == command )
		{
				ADL_Err = ADL_Display_Modes_Get ( -1, -1, &iNumModes, &lpADLMode );
				if ( lpADLMode )
				{
					for ( i = 0; i < iNumModes; i++ )
					{
						ADL_Err = AdlModeToFile( &lpADLMode[ i ] );
					}
				}
				if ( ADL_OK != ADL_Err )
					DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
				CloseADL();
				return ADL_Err;
		}

		if ( SETMODE == command )
		{
			iNumModes = 1;
			ADL_Err = AdlModeFromFile( &adlmode );

			if ( ADL_OK != ADL_Err )
			{
				if ( 0 < ADL_Err )
						sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
				else
						sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
				DisplayErrorAndPause( msg );
				CloseADL();
				return ADL_ERR;
			}

			// Obtain the ADL_Display_Modes_Set
			ADL_Err = ADL_Display_Modes_Set( adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex, iNumModes, &adlmode );

			if ( ADL_OK != ADL_Err )
				DisplayErrorAndPause( "ERROR: Calling ADL_Display_Modes_Set()" );
			CloseADL();
			return ADL_Err;
		}

		if ( SETOVER == command )
		{
				ADL_Err = AdlDisplayModeInfoFromFile( &iAdapterIndex, &iDisplayIndex, &AdlDmi );
				if ( ADL_OK != ADL_Err )
				{
					if ( 0 < ADL_Err )
							sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
					else
							sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
					DisplayErrorAndPause( msg );
					CloseADL();
					return ADL_ERR;
				}

				ADL_Err = ADL_Display_ModeTimingOverride_Set( iAdapterIndex, iDisplayIndex,  &AdlDmi, 1 );
				if ( ADL_OK != ADL_Err )
					DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Set() failed!" );

				CloseADL();
				return ADL_Err;
		}

		if ( INFOOVER == command )
		{
				ADL_Err = AdlModeFromFile( &adlmode );
				if ( ADL_OK != ADL_Err )
				{
					if ( 0 < ADL_Err )
							sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
					else
							sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
					DisplayErrorAndPause( msg );
				}
				else
				{
					AdlDM.iPelsHeight = adlmode.iYRes;
					AdlDM.iPelsWidth = adlmode.iXRes;
					AdlDM.iBitsPerPel = adlmode.iColourDepth;
					AdlDM.iDisplayFrequency = (int)adlmode.fRefreshRate;
					ADL_Err = ADL_Display_ModeTimingOverride_Get( adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex,  &AdlDM, &AdlDmi );
					if ( ADL_OK != ADL_Err )
						DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Get() failed!" );
					else
						AdlDisplayModeInfoToFile( file2, adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex, &AdlDmi );
				}
				CloseADL();
				return ADL_Err;
		}

		// Obtain the number of adapters for the system
        ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters );

        if ( 0 < iNumberAdapters )
        {
            lpAdapterInfo = malloc ( sizeof (AdapterInfo) * iNumberAdapters );
            memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

            // Get the AdapterInfo structure for all adapters in the system
            ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
        }
		else
		{
				DisplayErrorAndPause( "ERROR: No adapters found in this system!" );
				CloseADL();
				return ADL_Err;
		}

        // Repeat for all available adapters in the system
        for ( i = 0; i < iNumberAdapters; i++ )
        {
				iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;

				ADL_Err = ADL_Adapter_Active_Get ( iAdapterIndex, &iActive );
				// If the adapter is not active skip the steps below.
				if ( 0 == iActive || ADL_OK != ADL_Err)
					continue;

				ADL_Main_Memory_Free ( (void **)&lpAdlDisplayInfo );

				ADL_Err = ADL_Display_DisplayInfo_Get (iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 1);

				if (ADL_OK != ADL_Err)
					continue;

				for ( j = 0; j < iNumDisplays; j++ )
				{
					// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
					if (  ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED ) != 
						( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED	 &
							lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
							continue;   // Skip the not connected or not mapped displays

						// Is the display mapped to this adapter?
						if ( iAdapterIndex != lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex )
							continue;

						iDisplayIndex = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
						
						if ( BIOSINFO == command )
						{
								// Obtain the ASIC Bios Info
								ADL_Err = ADL_Adapter_VideoBiosInfo_Get ( iAdapterIndex, &AdlBI );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Adapter_VideoBiosInfo_Get() failed!" );
								else
								{
									ADL_Err = AdlBiosInfoToFile( iAdapterIndex, iDisplayIndex, &AdlBI );
									if ( ADL_OK != ADL_Err )
										DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
								}
						}

						if ( GETMODE == command )
						{
								// Obtain the ADL_Display_Modes_Get
								ADL_Err = ADL_Display_Modes_Get ( iAdapterIndex, iDisplayIndex, &iNumModes, &lpADLMode );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get() failed!" );
								else
								{
									if ( lpADLMode )
									{
										ADL_Err = AdlModeToFile( lpADLMode );
									}

									if ( ADL_OK != ADL_Err )
										DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
								}
						}

						if ( GETLIST == command )
						{
								lpModeInfoList = malloc ( sizeof (ADLDisplayModeInfo) * iMaxNumOfOverrides );
								memset ( lpModeInfoList, '\0', sizeof (ADLDisplayModeInfo) * iMaxNumOfOverrides );

								ADL_Err = ADL_Display_ModeTimingOverrideList_Get( iAdapterIndex, iDisplayIndex,
																														iMaxNumOfOverrides, lpModeInfoList, &lpNumOfOverrides);
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverrideList_Get() failed!" );
								else
								{
										if ( 0 == lpNumOfOverrides )
											printf( "\nTimingOverride list for AdapterIndex %d, DisplayIndex %d is empty!\n",  iAdapterIndex, iDisplayIndex );
										else
										{
											for ( k = 0; k < lpNumOfOverrides; k++ )
												ADL_Err = AdlDisplayModeInfoToFile( file, iAdapterIndex, iDisplayIndex, &lpModeInfoList[ k ] );
										}
								}
							   ADL_Main_Memory_Free ( (void **)&lpModeInfoList );
						}

						if ( GENERATE == command )
						{
							ADL_Err = ADL_Display_Modes_Get ( iAdapterIndex, iDisplayIndex, &iNumModes, &lpADLMode );
							if ( ADL_OK != ADL_Err )
								DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get() failed!" );
							else
							{
								AdlDmi.iTimingStandard = ADL_DL_MODETIMING_STANDARD_CVT; // a valid standard. Avoid 0
								AdlDmi.iRefreshRate = (int)lpADLMode->fRefreshRate;
								AdlDmi.iPelsWidth = lpADLMode->iXRes;
								AdlDmi.iPelsHeight = lpADLMode->iYRes;
								ADL_Err = AdlDisplayModeInfoToFile( file, iAdapterIndex, iDisplayIndex, &AdlDmi );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: Generating a template failed!" );
							}
				}
            }
        }
		CloseADL();
		return 0;
}

int AdlModeToFile( LPADLMode lpADLMode )
{
	if ( NULL == file || NULL == lpADLMode )
		return ADL_ERR_NULL_POINTER;

	fprintf( file, "%-15s %d\n", "AdapterIndex", lpADLMode->iAdapterIndex );
	fprintf( file, "%-15s %d\n", "DisplayIndex", lpADLMode->displayID.iDisplayLogicalIndex);
	fprintf( file, "%-15s %d\n", "Width", lpADLMode->iXRes);
	fprintf( file, "%-15s %d\n", "Height", lpADLMode->iYRes);
	fprintf( file, "%-15s %d\n", "ColorDepth", lpADLMode->iColourDepth);
	fprintf( file, "%-15s %.2f\n", "RefreshRate", lpADLMode->fRefreshRate);
	fprintf( file, "%-15s %d\n", "XPos", lpADLMode->iXPos);
	fprintf( file, "%-15s %d\n", "YPos", lpADLMode->iYPos);
	fprintf( file, "%-15s %d\n", "Orientation", lpADLMode->iOrientation);
	fprintf( file, "%-15s %d\n", "ModeFlag", lpADLMode->iModeFlag);
	fprintf( file, "%-15s 0x%04X\n", "ModeMask", lpADLMode->iModeMask);
	fprintf( file, "%-15s 0x%04X\n\n", "ModeValue", lpADLMode->iModeValue);

	return ADL_OK;
}

int AdlModeFromFile( LPADLMode  lpADLMode )
{
	int iValue;
	float fValue;
	int line = 0;

	if ( NULL == file || NULL == lpADLMode )
		return ADL_ERR_NULL_POINTER;

	if ( ADL_OK == GetValue( "AdapterIndex" , &iValue, ++line ))
		lpADLMode->iAdapterIndex = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "DisplayIndex" , &iValue, ++line  ))
		lpADLMode->displayID.iDisplayLogicalIndex = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Width" , &iValue, ++line  ))
		lpADLMode->iXRes = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Height" , &iValue, ++line  ))
		lpADLMode->iYRes = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "ColorDepth" , &iValue, ++line  ))
		lpADLMode->iColourDepth = iValue;
	else
		return line;

	if ( ADL_OK == GetFloat( "RefreshRate" , &fValue, ++line  ))
		lpADLMode->fRefreshRate = fValue;
	else
		return line;

	if ( ADL_OK == GetValue( "XPos" , &iValue, ++line  ))
		lpADLMode->iXPos = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "YPos" , &iValue, ++line  ))
		lpADLMode->iYPos = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Orientation" , &iValue, ++line  ))
		lpADLMode->iOrientation = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "ModeFlag" , &iValue, ++line  ))
		lpADLMode->iModeFlag = iValue;
	else
		return line;

	if ( ADL_OK == GetHex( "ModeMask" , &iValue, ++line  ))
		lpADLMode->iModeMask = iValue;
	else
		return line;

	if ( ADL_OK == GetHex( "ModeValue" , &iValue, ++line ))
		lpADLMode->iModeValue = iValue;
	else
		return line;

	return ADL_OK;
}

int AdlDisplayModeInfoToFile( FILE * file, int iAdapterIdx, int iDisplayIdx, ADLDisplayModeInfo * lpModeInfoList )
{
	if ( NULL == file || NULL == lpModeInfoList )
		return ADL_ERR_NULL_POINTER;

			fprintf( file, "%-17s %d\n", "AdapterIndex", iAdapterIdx );
			fprintf( file, "%-17s %d\n", "DisplayIndex", iDisplayIdx );
			fprintf( file, "%-17s %d\n", "TimingStandard", lpModeInfoList->iTimingStandard );
			fprintf( file, "%-17s %d\n", "PossibleStandard", lpModeInfoList->iPossibleStandard );
			fprintf( file, "%-17s %d\n", "RefreshRate", lpModeInfoList->iRefreshRate );
			fprintf( file, "%-17s %d\n", "Width", lpModeInfoList->iPelsWidth );
			fprintf( file, "%-17s %d\n", "Height", lpModeInfoList->iPelsHeight );
			fprintf( file, "%-17s %d\n", "TimingFlags", lpModeInfoList->sDetailedTiming.sTimingFlags );
			fprintf( file, "%-17s %d\n", "HTotal", lpModeInfoList->sDetailedTiming.sHTotal );
			fprintf( file, "%-17s %d\n", "HDisplay", lpModeInfoList->sDetailedTiming.sHDisplay );
			fprintf( file, "%-17s %d\n", "HSyncStart", lpModeInfoList->sDetailedTiming.sHSyncStart );
			fprintf( file, "%-17s %d\n", "HSyncWidth", lpModeInfoList->sDetailedTiming.sHSyncWidth );
			fprintf( file, "%-17s %d\n", "VTotal", lpModeInfoList->sDetailedTiming.sVTotal );
			fprintf( file, "%-17s %d\n", "VDisplay", lpModeInfoList->sDetailedTiming.sVDisplay );
			fprintf( file, "%-17s %d\n", "VSyncStart", lpModeInfoList->sDetailedTiming.sVSyncStart );
			fprintf( file, "%-17s %d\n", "VSyncWidth", lpModeInfoList->sDetailedTiming.sVSyncWidth );
			fprintf( file, "%-17s %d\n", "PixelClock", lpModeInfoList->sDetailedTiming.sPixelClock );
			fprintf( file, "%-17s %d\n", "HOverscanRight", lpModeInfoList->sDetailedTiming.sHOverscanRight );
			fprintf( file, "%-17s %d\n", "HOverscanLeft", lpModeInfoList->sDetailedTiming.sHOverscanLeft );
			fprintf( file, "%-17s %d\n", "VOverscanBottom", lpModeInfoList->sDetailedTiming.sVOverscanBottom );
			fprintf( file, "%-17s %d\n", "VOverscanTop", lpModeInfoList->sDetailedTiming.sVOverscanTop );
//			fprintf( file, "%-17s %d\n", "Overscan8B", lpModeInfoList->sDetailedTiming.sOverscan8B );
//			fprintf( file, "%-17s %d\n", "OverscanGR", lpModeInfoList->sDetailedTiming.sOverscanGR );
			fprintf( file, "\n" );

	return ADL_OK;
}

int AdlDisplayModeInfoFromFile( int * lpAdapterIdx, int * lpDisplayIdx, ADLDisplayModeInfo * lpModeInfoList )
{
	int iValue;
	int line = 0;

	if ( NULL == lpModeInfoList || NULL == lpAdapterIdx || NULL == lpDisplayIdx )
		return ADL_ERR_NULL_POINTER;

	if ( ADL_OK == GetValue( "AdapterIndex" , &iValue, ++line ))
		*lpAdapterIdx = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "DisplayIndex" , &iValue, ++line ))
		*lpDisplayIdx = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "TimingStandard" , &iValue, ++line ))
		lpModeInfoList->iTimingStandard = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "PossibleStandard" , &iValue, ++line ))
		lpModeInfoList->iPossibleStandard = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "RefreshRate" , &iValue, ++line ))
		lpModeInfoList->iRefreshRate = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Width" , &iValue, ++line ))
		lpModeInfoList->iPelsWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Height" , &iValue, ++line ))
		lpModeInfoList->iPelsHeight = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "TimingFlags" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sTimingFlags = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HTotal" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHTotal = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HDisplay" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHDisplay = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HSyncStart" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHSyncStart = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HSyncWidth" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHSyncWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VTotal" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVTotal = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VDisplay" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVDisplay = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VSyncStart" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVSyncStart = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VSyncWidth" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVSyncWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "PixelClock" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sPixelClock = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HOverscanRight" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHOverscanRight = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HOverscanLeft" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHOverscanLeft = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VOverscanBottom" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVOverscanBottom = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VOverscanTop" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVOverscanTop = iValue;
	else
		return line;
/*
	if ( ADL_OK == GetValue( "Overscan8B" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sOverscan8B = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "OverscanGR" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sOverscanGR = iValue;
	else
		return line;
*/
	return ADL_OK;
}

int GetValue(  char * name, int * iValue, int line )
{
	char sField[ 256 ];
	fscanf_s( file,"%32s %d\n", sField, 33, iValue);
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		sprintf_s( err, sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}

int GetHex(  char * name, int * iValue, int line )
{
	char sField[ 256 ];
	fscanf_s( file,"%32s %X\n", sField, 33, iValue);
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		sprintf_s( err, sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}

int GetFloat(  char * name, float * fValue, int line )
{
	char sField[ 256 ];
	fscanf_s( file,"%32s %f\n", sField, 33, fValue);
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		sprintf_s( err,  sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}

// Initialize ADL
int OpenADL()
{
    ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
	int ADL_Err = ADL_ERR;

#if defined (LINUX)
	char sztemp[256];
    sprintf(sztemp,"libatiadlxx.so");
    hDLL = dlopen( sztemp, RTLD_LAZY|RTLD_GLOBAL);
#else
    hDLL = LoadLibrary( "atiadlxx.dll" );
    if (hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary("atiadlxy.dll");
    }
#endif

	if (hDLL != NULL)
    {
        ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL,"ADL_Main_Control_Create");
        if ( NULL != ADL_Main_Control_Create)
            ADL_Err = ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1);
			// The second parameter is 1, which means:
			// retrieve adapter information only for adapters that are physically present and enabled in the system
	}
    else
    {
        printf("ADL Library not found!\n" );
    }

	return ADL_Err;
}

// Destroy ADL
void CloseADL()
{
	   ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;

	if ( NULL != file )
		fclose( file );
	if ( NULL != file2 )
		fclose( file2 );

		ADL_Main_Memory_Free ( (void **)&lpAdapterInfo );
		ADL_Main_Memory_Free ( (void **)&lpAdlDisplayInfo );
	   
		ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
		if ( NULL != ADL_Main_Control_Destroy )
            ADL_Main_Control_Destroy ();
		
#if defined (LINUX)
    dlclose(hDLL);
#else
    FreeLibrary(hDLL);
#endif
}

int AdlBiosInfoToFile( int iAdapterIndex, int iDisplayIndex, ADLBiosInfo * lpAdlBI )
{
	if ( NULL == file || NULL == lpAdlBI )
		return ADL_ERR_NULL_POINTER;

		fprintf( file, "%-17s %d\n", "AdapterIndex", iAdapterIndex );
		fprintf( file, "%-17s %d\n", "DisplayIndex", iDisplayIndex );
		fprintf( file, "%-17s %s\n", "PartNumber", lpAdlBI->strPartNumber );
		fprintf( file, "%-17s %s\n", "Version", lpAdlBI->strVersion );
		fprintf( file, "%-17s %s\n\n", "Date", lpAdlBI->strDate );

	return ADL_OK;
}

int DisplayErrorAndPause( char * sError )
{
	printf ( "%s",  sError );
	printf ( "\nPress Enter to continue..." );
	getchar();
	return ADL_ERR;
}

void ShowHelp(void)
{
		printf ( "\nADL Utility, Ver 3.0        Copyright(c) 2010 Advanced Micro Devices, Inc.\n\n" );
		printf ( "adlutil get [file]        : Get the display settings of all\n" );
		printf ( "                            active adapters [and saves them to file]\n" );
		printf ( "adlutil set file          : Set the display with the parameters from file\n" );
		printf ( "adlutil list [file]       : Get ALL Override Mode settings of all\n" );
		printf ( "                            active adapters [and saves them to file]\n" );
		printf ( "adlutil info file1 [file2]: Find Override Mode, defined in file1\n" );
		printf ( "                            [and save the detailed timings in file2]\n" );
		printf ( "adlutil over file         : Set Override Display mode from settings in file\n" );
		printf ( "adlutil gen [file]        : Generate template to be used by 'over' command\n" );
		printf ( "adlutil bios [file]       : Get the Video BIOS information\n" );
		DisplayErrorAndPause( "" );
}

