/***************************************************************************//**
  @file		filesystem.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		13 ene. 2023
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

// +Incluir el header propio (ej: #include "template.h")+
#include <stdio.h>
#include <string.h>
#include "ff15/source/ff.h"
#include "ff15/source/diskio.h"

#include "filesystem.h"
#include "SD/SD.h"
#include "node.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ELEM_SIZE 100
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject;   /* File object */

FRESULT error;
DIR directory; /* Directory object */
FILINFO fileInformation;
UINT bytesWritten;
UINT bytesRead;
const TCHAR driverNumberBuffer[4U] = {SD_DISK + '0', ':', '/', '\0'};


static data file;
static Node *n;
static element_info_t elements [ELEM_SIZE];
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool node_append();
static bool map_files(char * dir, Node *n);

#ifdef DEBUG_FILE
static void	nodePrint(char *name, Node *n);
#endif

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static int temperaturas_actuales[4];+


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool init_filesys (void) {


    printf("\r\nPlease insert a card into board.\r\n");

	if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
	{
		printf("Mount volume failed.\r\n");
		return false;
	}

	#if (FF_FS_RPATH >= 2U)
	    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
	    if (error)
	    {
	        printf("Change drive failed.\r\n");
	        return false;
	    }
	#endif

	#if FF_USE_MKFS	//	ESTO ESTA COMENTADO PORQUE BORRA EL CONTENIDO
	   /* printf("\r\nMake file system......The time may be long if the card capacity is big.\r\n");
	    if (f_mkfs(driverNumberBuffer, 0, work, sizeof work))
	    {
	        printf("Make file system failed.\r\n");
	        return false;
	    }*/
	#endif /* FF_USE_MKFS */

	return true;
}

// bool create_dir(char * dir){
// 	printf("\r\nCreate directory......\r\n");
// 	    error = f_mkdir(_T(dir));
// 	    if (error)
// 	    {
// 	        if (error == FR_EXIST)
// 	        {
// 	            printf("Directory exists.\r\n");
// 	        }
// 	        else
// 	        {
// 	            printf("Make directory failed.\r\n");
// 	        }
// 	        return false;
// 	    }
// 	    return true;

// }

// bool create_file(char * filename){
//    printf("\r\nCreate a file in that directory......\r\n");
//    error = f_open(&g_fileObject, _T(filename), (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
//    if (error)
//    {
//        if (error == FR_EXIST)
//        {
//            printf("File exists.\r\n");
//        }
//        else
//       {
//            printf("Open file failed.\r\n");
//        }
//        return false;
//    }
//    return true;
//}

bool list_file(char * dir){
    printf("\r\nList the file in that directory......\r\n");

    if (f_opendir(&directory, dir))
    {
        printf("Open directory failed.\r\n");
        return false;
    }

    for (;;)
    {
        error = f_readdir(&directory, &fileInformation);

        /* To the end. */
        if ((error != FR_OK) || (fileInformation.fname[0U] == 0U))
        {
            break;
        }
        if (fileInformation.fname[0] == '.')
        {
            continue;
        }
        if (fileInformation.fattrib & AM_DIR)
        {
            printf("Directory file : %s.\r\n", fileInformation.fname);
        }
        else
        {
            printf("General file : %s.\r\n", fileInformation.fname);
        }
    }

    return true;

}

bool statrt_mapping (){
	if(n){nodeDestroy(n);}

	strcpy(elements[0].name, "filelsys");
	strcpy(elements[0].path, "");

    n=nodeNew(elements[0].name, elements[0].path);
    bool state = map_files (elements[0].path, n);
#ifdef DEBUG_FILE
    nodePrint(n->name,n);
	nodePrint(n->children->name, n->children);
	nodePrint(n->children->next->name, n->children->next);
	nodePrint(n->children->next->next->name, n->children->next->next);
	nodePrint(n->children->next->next->next->name, n->children->next->next->next);
	nodePrint(n->children->next->next->next->next->name, n->children->next->next->next->next);
	nodePrint(n->children->next->next->next->next->next->name, n->children->next->next->next->next->next);
	nodePrint(n->children->next->next->next->next->next->next->name, n->children->next->next->next->next->next->next);
	nodePrint(n->children->next->next->next->next->next->next->next->name, n->children->next->next->next->next->next->next->next);
#endif
	printf("Mapping ended\n");
    return state;

}

char * open_folder(){
	if (!n || !n->children){
		return NULL;
	}
	if (n->children){
		n=n->children;
		return n->name;
	}
	return NULL;
}
char * close_folder(){
	if (!n || !n->parent){
		return NULL;
	}
	if (n->parent){
		n=n->parent;
		return n->name;
	}
	return NULL;
}
data * open_file(){
	if (!n){
		return NULL;
	}
	error = f_open(&g_fileObject, n->path, FA_READ);
	if (!error){
		error = f_read(&g_fileObject, file.g_bufferRead, sizeof(file.g_bufferRead), &file.bytes_read);
		if(!error){
			 f_close(&g_fileObject);
			return &file;
		}
	}
	return NULL;

}
char *show_next(){
	if (!strcmp(n->path,"") && n->children){
		n= n->children;
		return n->name;
	}
	if (!n || !n->next){
		if (n && n->prev){
			while (n->prev){
				n=n->prev;
			}
		}
		else{
			return NULL;
		}
	}
	else{
		n = n->next;
	}
	return n->name;
}
char *show_prev(){
	if (!strcmp(n->path,"") && n->children){
		n= n->children;
		return n->name;
	}
	if (!n || !n->prev){
		if (n && n->next){
			while (n->next){
				n=n->next;
			}
		}
		else{
			return NULL;
		}
	}
	else{
		n = n->prev;
	}
	return n->name;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


bool map_files(char * dir, Node *n){
    //printf("\r\nMapping in that directory......\r\n");
	DIR directory2; /* Directory object */
	FILINFO fileInformation2;
	static int i = 1;
	char tempStr [12+1];

    if (f_opendir(&directory2,dir))
    {
        printf("Open directory failed.\r\n");
        return false;
    }
    Node * n1 = n->children;
    for (;;)
    {
        error = f_readdir(&directory2, &fileInformation2);

        /* To the end. */
        if ((error != FR_OK) || (fileInformation2.fname[0U] == 0U))
        {
            break;
        }
        if (fileInformation2.fname[0] == '.')
        {
            continue;
        }

        if (fileInformation2.fattrib & AM_DIR)
        {
#ifdef DEBUG_FILE
            printf("Directory file : %s - %s.\r\n", dir, fileInformation2.fname);
#endif
            tempStr[0]='-'; tempStr[1]='-'; tempStr[2]='>'; tempStr[3]='\0';
            strcat(tempStr,fileInformation2.fname);
            memcpy(elements[i].name,tempStr, strlen(tempStr));
            elements[i].name[strlen(tempStr)]='\0';
            //memcpy(elements[i].name,fileInformation2.fname, strlen(fileInformation2.fname));
            memcpy (elements[i].path, dir, strlen(dir));
            elements[i].path[strlen(dir)]='/';
            elements[i].path[strlen(dir)+1]='\0';
            strcat(elements[i].path,fileInformation2.fname);
            if (!node_append(n, elements+i)){return false;}
            if (!n1){
            	n1 = n->children;
            }
            else{
            	n1 = n1->next;
            }
            i++;
            map_files(elements[i-1].path, n1);
        }
        else
        {
#ifdef DEBUG_FILE
            printf("General file: %s - %s.\r\n", dir, fileInformation2.fname);
#endif

            memcpy(elements[i].name,fileInformation2.fname, strlen(fileInformation2.fname));
            memcpy(elements[i].path, dir, strlen(dir));
            elements[i].path[strlen(dir)]='/';
            elements[i].path[strlen(dir)+1]='\0';
		   	strcat(elements[i].path,fileInformation2.fname);
            if (!node_append(n, elements+i)){return false;}
            i++;
            if (!n1){
            	n1 = n->children;
            }
            else{
            	n1 = n1->next;
            }
        }
    }

    return true;

}

bool node_append (Node *n, element_info_t * element){
	if (! nodeAppend(n, nodeNew(element->name,element->path))){		//En append necesito pasarle el parent, y tengo el hijo yo
		nodeDestroy(n);
		n = NULL;
	}
	if (! n)
	{
		printf("Error during tree initialization!\r\n");
		return false;
	}
	return true;
}

 
#ifdef DEBUG_FILE
/* Print a node */
void nodePrint(char *name, Node *n)
{
	printf("%s:\n", name);
	if (! n)
	{
		printf("  empty!\n");
		return;
	}
	Node *o;
	int i;

	printf("  data     = ");
	if (n->name)
		printf("%s", (char*)n->name);
	else
		printf("false");
	printf("\n  next     = ");
	if (n->next)
	{
		for (i = 0, o = n; o->next; i++)
			o = o->next;
		printf("%d", i);
	}
	else
		printf("false");
	printf("\n  prev     = ");
	if (n->prev)
	{
		for (i = 0, o = n; o->prev; i++)
			o = o->prev;
		printf("%d", i);
	}
	else
		printf("false");
	printf("\n  parent   = %s", (n->parent) ? "true" : "false");
	printf("\n  children = ");
	if (n->children)
	{
		for (i = 0, o = n->children; o; i++)
			o = o->next;
		printf("%d", i);
	}
	else
		printf("false");
	putchar('\n');
	putchar('\n');
}
#endif
