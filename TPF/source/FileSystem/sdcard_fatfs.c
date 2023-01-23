/*
Test
 */

#include <stdio.h>
#include <string.h>

#include "../MCAL/board.h"
#include "filesystem.h"


#define BTN_EXT1 PORTNUM2PIN(PC,3)
#define BTN_EXT2 PORTNUM2PIN(PC,2)
#define BTN_EXT3 PORTNUM2PIN(PA,2)
/*******************************************************************************
 * Code
 ******************************************************************************/

void App_Init (void){

}

/*
 * @brief Main function
 */
void App_Run(){

    init_filesys();

    //create_dir("/dir_1");

    //create_file("/file_loco.txt");

    //list_file("/");

    statrt_mapping();

    gpioMode(PIN_SW3, INPUT);			//Ya es pulldown electricamente
    gpioMode(PIN_SW2, INPUT_PULLUP);	//Esta como do not place (no tiene resistencia)
    gpioMode(BTN_EXT1, INPUT_PULLDOWN);
    gpioMode(BTN_EXT2, INPUT_PULLDOWN);
    gpioMode(BTN_EXT3, INPUT_PULLDOWN);
    uint8_t tab=0;

    while(1){
    	if (!gpioRead(PIN_SW3)){	//NEXT
    		while (!gpioRead(PIN_SW3));
    		for(int i=0; i!=tab; i++){printf("\t");}
    		printf("%s\n",show_next());
    	}
    	if (!gpioRead(PIN_SW2)){	//PREV
			while (!gpioRead(PIN_SW2));
			for(int i=0; i!=tab; i++){printf("\t");}
			printf("%s\n",show_prev());
		}
    	if (gpioRead(BTN_EXT2)){	//OPEN FOLDER
    		while (gpioRead(BTN_EXT2));
    		tab++;
    		for(int i=0; i!=tab; i++){printf("\t");}
    	    printf("Folder opened\n");
    	    for(int i=0; i!=tab; i++){printf("\t");}
    	    printf("%s \n",open_folder());
    	}
    	if (gpioRead(BTN_EXT3)){	//CLOSE FOLDER
    		while (gpioRead(BTN_EXT3));
    		if(tab) {tab--;}
    		for(int i=0; i!=tab; i++){printf("\t");}
    	    printf("Folder closed\n");
    	    for(int i=0; i!=tab; i++){printf("\t");}
    	    printf("%s \n",close_folder());
    	}
    	if (gpioRead(BTN_EXT1)){	//OPEN FILE
			while (gpioRead(BTN_EXT1));
			for(int i=0; i!=tab; i++){printf("\t");}
			printf("File content: \n");
			printf("%s\n",open_file()->g_bufferRead);
		}
    }
}
