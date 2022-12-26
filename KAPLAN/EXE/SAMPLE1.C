#include <stdio.h>
#include <stdtype.h>
#include "c_iml.h"
#include "declarations.h"

struct declarationStruct *declarations = NULL;
struct imageDBStruct *imageDB;
struct functionTableElementStruct *imageFunctionTable;
struct symbolTableEntryStruct *symbolTable;


void log(char *format, char *s);
int lines = 0;
bool pause_between_screens = false;
bool log_to_file = false;
FILE *log_file = stdout;

void main(argc,argv)
int argc;
char **argv;

{
bool addImage(char *filename, char *imagename);
bool setImageIndex(char *name, char *characteristic, struct valueStruct *value);
bool iml_remove(struct imageListStruct *list);
struct imageListStruct *selectImage(char *name, struct valueStruct *value, struct imageListStruct **result);
struct imageListStruct *andImageReferentList(struct imageListStruct *imageList1, struct imageListStruct *imageList2);
struct imageListStruct *orImageReferentList(struct imageListStruct *imageList1, struct imageListStruct *imageList2);
struct imageListStruct *combine(int operator, struct imageListStruct *p, struct imageListStruct *q);
struct imageStruct *computeImage(int op, struct imageStruct *image1, struct imageStruct *image2);
bool readImage(char *name, char *variable);
bool saveImage(char *variable, char *name);
void freeSymbolTable(struct symbolTableEntryStruct);
struct valueStruct value;
struct argumentStruct *arguments;
struct argumentStruct *arg;
char *allocateString(char *s);
struct	imageStruct	*reduce(struct argumentStruct *arguments);
struct	imageStruct	*enlarge(struct argumentStruct *arguments);
struct	imageStruct	*extract(struct argumentStruct *arguments);
struct	imageStruct	*rotate(struct argumentStruct *arguments);
struct	imageStruct	*position(struct argumentStruct *arguments);
struct	imageDBStruct *findImageInDB(char *name);
struct imageDBStruct *dbp;
char *getmem(int size);
if (argc > 1)
{
if (strcmp(argv[1],"/p") == 0)
{
pause_between_screens = true;
}

if (strcmp(argv[1],"/f") == 0)
{
log_to_file = true;
log_file = fopen(argv[2],"w");
}
}



if(!addImage("a.img","b")) error("addimage error",errorAction_terminate);
value.type = 3;
value.ival = 1;
if(!setImageIndex("b", "c", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("a.img","c")) error("addimage error",errorAction_terminate);
value.type = 3;
value.ival = 1;
if(!setImageIndex("c", "d", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("a.img","d")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("this is a test");
if(!setImageIndex("d", "n", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("d.img","e")) error("addimage error",errorAction_terminate);
value.type = 3;
value.ival = 1;
if(!setImageIndex("e", "f", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("test");
if(!setImageIndex("e", "g", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("h.img","f")) error("addimage error",errorAction_terminate);
value.type = 2;
value.dval = 1.150000;
if(!setImageIndex("f", "j", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 3;
if(!setImageIndex("f", "k", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("a string");
if(!setImageIndex("f", "l", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i1.img","image1")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("boy");
if(!setImageIndex("image1", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("girl");
if(!setImageIndex("image1", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 5;
if(!setImageIndex("image1", "age", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i2.img","image2")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("boy");
if(!setImageIndex("image2", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 5;
if(!setImageIndex("image2", "age", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i3.img","image3")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("girl");
if(!setImageIndex("image3", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 5;
if(!setImageIndex("image3", "age", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i4.img","image4")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("boy");
if(!setImageIndex("image4", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("girl");
if(!setImageIndex("image4", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 5;
if(!setImageIndex("image4", "age", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i5.img","image5")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("boy");
if(!setImageIndex("image5", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("girl");
if(!setImageIndex("image5", "image", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 3;
value.ival = 2;
if(!setImageIndex("image5", "age", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i6.img","image6")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("red");
if(!setImageIndex("image6", "color", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("large");
if(!setImageIndex("image6", "size", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i7.img","image7")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("blue");
if(!setImageIndex("image7", "color", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("large");
if(!setImageIndex("image7", "size", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i8.img","image8")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("blue");
if(!setImageIndex("image8", "color", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("medium");
if(!setImageIndex("image8", "size", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i9.img","image9")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("blue");
if(!setImageIndex("image9", "color", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("small");
if(!setImageIndex("image9", "size", &value)) error("setImageIndex error", errorAction_terminate);
if(!addImage("i10.img","image10")) error("addimage error",errorAction_terminate);
value.type = 4;
value.sval = allocateString("red");
if(!setImageIndex("image10", "color", &value)) error("setImageIndex error", errorAction_terminate);
value.type = 4;
value.sval = allocateString("large");
if(!setImageIndex("image10", "size", &value)) error("setImageIndex error", errorAction_terminate);
readImage("ifile.img", "iv1");
saveImage("iv1", "ifile.img");
value.type = 3;
value.ival = 5;
selectImage("age", &value, &t1);
value.type = 4;
value.sval = allocateString("boy");
selectImage("image", &value, &t2);
value.type = 4;
value.sval = allocateString("girl");
selectImage("image", &value, &t3);
t4 = combine(0, t2, t3);
t5 = combine(0, t1, t4);
view(t5);
value.type = 4;
value.sval = allocateString("blue");
selectImage("color", &value, &t6);
value.type = 4;
value.sval = allocateString("large");
selectImage("size", &value, &t7);
value.type = 4;
value.sval = allocateString("medium");
selectImage("size", &value, &t8);
t9 = combine(1, t7, t8);
t10 = combine(0, t6, t9);
iml_remove(t10);
value.type = 4;
value.sval = allocateString("alpha");
selectImage("type", &value, &t11);
value.type = 4;
value.sval = allocateString("beta");
selectImage("type", &value, &t12);
t13 = combine(0, t11, t12);
iml_remove(t13);
value.type = 4;
value.sval = allocateString("red");
selectImage("color", &value, &t14);
t15 = combine(2, t14, NULL);
view(t15);
dbp = findImageInDB("b");
t16 = &(dbp -> image);
arguments = allocateMemory(argumentStruct);
arg = arguments;
arg -> next = NULL;
if (arg -> next != NULL) arg = arg -> next;
arg -> sval = allocateString("q");
arg -> argumentType = t_string;
arg -> next = allocateMemory(argumentStruct);
if (arg -> next != NULL) arg = arg -> next;
arg -> dval = 10.000000;
arg -> argumentType = t_real;
arg -> next = allocateMemory(argumentStruct);
free(arg -> next);
arg -> next = NULL;
t17 = reduce(arguments);
t18 = computeImage(3, t16, t17);
dbp = findImageInDB("c");
t19 = &(dbp -> image);
t20 = computeImage(3, t18, t19);
dbp = findImageInDB("d");
t21 = &(dbp -> image);
t22 = computeImage(3, t20, t21);
dbp = findImageInDB("e");
t23 = &(dbp -> image);
dbp = findImageInDB("f");
t24 = &(dbp -> image);
t25 = computeImage(3, t23, t24);
t26 = computeImage(4, t22, t25);
a = computeImage(2, t26, NULL);


if (log_to_file) fclose(log_file);
}
