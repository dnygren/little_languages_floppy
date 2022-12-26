#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c_iml.h"

char	*allocateString(char *string);
void	error(char * message, int action);
char	*getmem(int size);
void addToImageDB(struct imageStruct *image);
struct	imageListStruct 	*combine(int operator, struct imageListStruct *p, struct imageListStruct *q);
char *createTemporaryVariable(int type);
void storeValue(int type, union valueUnion *value, struct valueStruct *target);
void writeValue(FILE *f, int type, union valueUnion *value);
extern struct declarationStruct        *declarations;
extern struct imageDBStruct *imageDB;
extern struct symbolTableEntryStruct *symbolTable;
extern struct functionTableElementStruct *imageFunctionTable;

/************************* LOG declarations *********************************/

void log(char *format, char *s);

/* line keeps track of the current line written to the screen */

extern int lines;

/* pause_between_screens is a flag set to true when the user specifies 
	that they want to pause between pages in screen output. */

extern bool pause_between_screens;

/* log_to_file is a flag set to true when the user specified that they
	want to output the operation log to a file. */

extern bool log_to_file;

/* log_file is the file to which the operation log will be sent. It is
	initially set to stdout so that output, by default, will be sent to the
	display. */

extern FILE *log_file;


bool iml_remove(struct imageListStruct *l)

{
	void displayImageList(struct imageListStruct *l);

	log("remove(abstract machine)\n",NULL);
	displayImageList(l);
	log("\n",NULL);

	return(true);
}



/* setImageIndex is the IML abstract function for setting a characteristic
	of an image in the IML database. */


bool setImageIndex(char *imageName, char *characteristic, struct valueStruct *characteristicValue)
{
	struct	imageDBStruct			*p;
	struct	imageDBStruct			*findImageInDB(char *name);
	struct	characteristicStruct *c;

/* find the image in the imageDB */

	p = findImageInDB(imageName);
	if (p == NULL)
		error("image not in image database", errorAction_terminate);

	c = p -> image.characteristics;

	c = allocateMemory(characteristicStruct);
	c -> name = allocateString(characteristic);
	c -> value = allocateMemory(valueStruct);

	storeValue(characteristicValue -> type, (union valueUnion *)&(characteristicValue -> u.ival), c -> value);

	c -> next = p -> image.characteristics;
	p -> image.characteristics = c;

	log("setImageindex(abstract machine)\n",NULL);

	log("image name: %s\n",imageName);
	log("characteristic: %s\n", characteristic);
	log("value: ",NULL);
	writeValue(log_file, characteristicValue -> type, (union valueUnion *)&(characteristicValue -> u.ival));
	log("\n\n",NULL);

	return(true);
}

struct imageDBStruct *findImageInDB(char *name)
{
	struct	imageDBStruct		*p;

	p = imageDB;

	while (p != NULL)
	{
		if (strcmp(name, p -> image.name) == 0) return(p);

		p = p -> next;
	}

	return(NULL);
}


/* IML abstract machine instruction view */

bool view(struct imageListStruct *l)
{
	void displayImageList(struct imageListStruct *l);

	log("view(abstract machine)\n",NULL);
	displayImageList(l);
	log("\n",NULL);

	return(true);
}


/* IML abstract machine instruction addImage. This function adds the image
	to the imageDB list. */

bool addImage(char *imageFileName, char *imageName)
{
	struct imageDBStruct *new;
	struct imageDBStruct *findImageInDB(char *name);

/* see if the image already exists. If so, then don't add it again. */

	new = findImageInDB(imageName);
	if (new != NULL) return(true);

	new = allocateMemory(imageDBStruct);

	new -> image.name = allocateString(imageName);
	new -> image.image = NULL;
	new -> image.characteristics = NULL;

	new -> next = imageDB;

	imageDB = new;

	log("addImage(abstract machine)\n",NULL);
	log("file name: %s\n",imageFileName);
	log("image name: %s\n\n",imageName);

	return(true);
}

void displayImageList(struct imageListStruct *l)
{
	struct imageListStruct *p;

	if (l == NULL)
	{
		log("image list is empty\n",NULL);
		return;
	}

	p = l;

	while (p != NULL)
	{
		log("%s\n", p -> image);

		p = p -> next;
	}
}

bool readImage(char *imageFileName, char *imageName)
{
	addImage(imageFileName, imageName);

	log("readImage(abstract machine)\n",NULL);
	log("file name: %s\n", imageFileName);
	log("image name: %s\n\n", imageName);

	return(true);
}


bool saveImage(char *imageName, char *imageFileName)
{
	log("saveImage(abstract machine)\n",NULL);
	log("image name: %s\n", imageName);
	log("file name: %s\n\n", imageFileName);

	return(true);
}

void storeImage(struct symbolTableEntryStruct *p, struct imageStruct *i)
{
	log("storeImage(abstract machine)\n",NULL);

	log("symbol name: %s\n", p -> name);
	log("image name: %s\n", i -> name);
}

/* The next function scans the imageDB for images with the specified attribute
	and value. A list of these is returned in an image list structure. */


bool selectImage(char *name, struct valueStruct *value, struct imageListStruct **result)
{
	struct 	imageListStruct *r;
	struct 	imageDBStruct *p;
	struct	characteristicStruct *c;
	struct	imageListStruct	*new;

	bool		compareValue(struct valueStruct *value1, struct valueStruct *value2);


	r = NULL;
	p = imageDB;

	while (p != NULL)
	{
		c = p -> image.characteristics;
		while (c != NULL)
		{
			if (strcmp(c -> name,name) == 0)
			{
				if (compareValue(c -> value, value))
				{
					new = allocateMemory(imageListStruct);
					new -> image = allocateString(p -> image.name);
					new -> next = r;
					r = new;
				}
			}
			c = c -> next;
		}

		p = p -> next;
	}

	*result = r;

	log("selectImage(abstract machine)\n",NULL);
	log("characteristic name: %s\n", name);
	log("characteristic value: ",NULL);
	writeValue(log_file, value -> type, (union valueUnion *)&(value -> u.ival));
	log("\n",NULL);
	displayImageList(r);
	log("\n",NULL);

	return(true);
}

struct imageStruct *computeImage(int operation, struct imageStruct *p, struct imageStruct *q)
{
	char	*createTemporaryVariable(int type);
	char 	*new;

	struct imageStruct *i;

	log("computeImage(abstract machine)\n",NULL);

	log("operation: ",NULL);
	if (operation == o_none) log("o_none\n",NULL);
	if (operation == o_plus) log("o_plus\n",NULL);
	if (operation == o_minus) log("o_minus\n",NULL);

	log("operanda: %s\n", p -> name);
	if (q != NULL) log("operandb: %s\n", q -> name);
		else log("operandb: NULL\n",NULL);

	new = createTemporaryVariable(t_image);

	i = allocateMemory(imageStruct);

	i -> name = new;
	i -> image = NULL;
	i -> characteristics = NULL;

	addToImageDB(i);

	log("result: %s\n\n", new);

	return(i);
}


bool compareValue(struct valueStruct *v1, struct valueStruct *v2)
{
	if (v1 -> type != v2 -> type) return(false);

	switch (v1 -> type)
	{
		case		t_integer:			if (v1 -> u.ival == v2 -> u.ival)
												return(true);
											break;

		case		t_real:				if (v1 -> u.dval == v2 -> u.dval)
												return(true);
											break;

		case		t_string:			if (strcmp(v1 -> u.sval, v2 -> u.sval) == 0)
												return(true);
											break;

	}
	return(false);
}

struct imageStruct *reduce(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName(void);
	struct imageStruct *result;

	log("reduce(image function)\n",NULL);
	log("image name: %s\n", arguments -> u.sval);
	log("reduction factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> u.dval));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

 	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *enlarge(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName(void);
	struct imageStruct *result;

	log("enlarge(image function)\n",NULL);
	log("image name: %s\n", arguments -> u.sval);
	log("enlarge factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> u.dval));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *extract(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName(void);
	struct imageStruct *result;

	log("extract(image function)\n",NULL);
	log("image name: %s\n", arguments -> u.sval);

	log("x: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> u.ival));
	log("\n",NULL);
	log("y: ",NULL);
	writeValue(log_file,
					((arguments -> next) -> next) -> argumentType,
					(union valueUnion *)&(((arguments -> next) -> next) -> u.ival));
	log("\n",NULL);
	log("w: ",NULL);
	writeValue(log_file,
					(((arguments -> next) -> next) -> next ) -> argumentType,
					(union valueUnion *)&((((arguments -> next) -> next) -> next) -> u.ival));
	log("\n",NULL);
	log("h: ",NULL);
	writeValue(log_file,
					((((arguments -> next) -> next) -> next) -> next) -> argumentType,
					(union valueUnion *)&(((((arguments -> next) -> next) -> next) -> next) -> u.ival));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *rotate(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName(void);
	struct imageStruct *result;

	log("rotate(image function)\n",NULL);
	log("image name: %s\n", arguments -> u.sval);
	log("rotate factor: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> u.dval));
	log("\n",NULL);


	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

struct imageStruct *position(struct argumentStruct *arguments)
{
	char *name;
	char *createNewVariableName(void);
	struct imageStruct *result;

	log("position(image function)\n",NULL);
	log("image name: %s\n", arguments -> u.sval);
	log("\n",NULL);

	log("x: ",NULL);
	writeValue(log_file,
					(arguments -> next) -> argumentType,
					(union valueUnion *)&((arguments -> next) -> u.ival));
	log("\n",NULL);
	log("y: ",NULL);
	writeValue(log_file,
					((arguments -> next) -> next) -> argumentType,
					(union valueUnion *)&(((arguments -> next) -> next) -> u.ival));
	log("\n",NULL);

	name = createNewVariableName();
	result = allocateMemory(imageStruct);

	result -> name = name;
	result -> image = NULL;
	result -> characteristics = NULL;

	log("resultant image: %s\n\n", name);

	return(result);
}

void addToImageDB(struct imageStruct *p)
{
	struct imageDBStruct *new;

	new = allocateMemory(imageDBStruct);

	new -> image.name =  p -> name;
	new -> image.image = NULL;
	new -> image.characteristics = NULL;

	new -> next = imageDB;
	imageDB = new;
}

void log(char *format, char *s)
{
	char buffer[81];

	if (s == NULL) fprintf(log_file,format);
		else fprintf(log_file,format,s);

	if (!log_to_file)
	{
		if (pause_between_screens)
		{
			lines++;
			if (lines > 18)
			{
				printf("\n... press ENTER to continue ...");
				gets(buffer);
				lines = 0;
			}
		}
	}
}

char *allocateString(char *s)

{
	char *p;

	if (s == NULL) return(NULL);

	p = getmem(strlen(s) + 1);

	strcpy(p,s);

	return(p);
}


void error(char *message, int action)

{
	puts("error while translating/compiling/interpreting IML program");
	puts(message);
	puts("");
	puts("");

	if (action == errorAction_terminate) abort();
}

char *getmem(int size)

{
	char *p;
	
	p = malloc(size);
	if (p == NULL)
	{
		error("could not allocate memory",errorAction_terminate);
		abort();
	}
	
	return(p);
}

struct	imageListStruct 	*combine(int operator, struct imageListStruct *p,
						struct imageListStruct *q)

{
	struct	imageListStruct	*target;
	struct	imageListStruct	*andImageReferentLists(struct imageListStruct *p,
								struct imageListStruct *q);
	struct	imageListStruct	*orImageReferentLists(struct imageListStruct *p,
								struct imageListStruct *q);

	if (operator == o_none)
		target = p;

	if (operator == o_and)
		target = andImageReferentLists(p, q);

	if (operator == o_or)
		target = orImageReferentLists(p,q);

	return(target);
}


struct	imageListStruct	*andImageReferentLists(struct imageListStruct *p,
							struct imageListStruct *q)

{
	bool		inList(struct imageListStruct *e, struct imageListStruct *p);

	struct	imageListStruct	*new;
	struct	imageListStruct	*last;
	struct	imageListStruct	*r;

	r = p;

	last = NULL;

	while (r != NULL)
	{
		if (inList(r,q))
		{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;

		}

		r = r -> next;    

	}

	return(last);
}



struct	imageListStruct	*orImageReferentLists(struct imageListStruct *p,
							struct imageListStruct *q)

{
	struct	imageListStruct	*new;
	struct	imageListStruct	*last;
	struct	imageListStruct	*r;

	r = p;
	last = NULL;

	while (r != NULL)
	{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;

			r = r -> next;    
	}

	r = q;

	while (r != NULL)
	{
		if (!inList(r, last))
		{
			new = allocateMemory(imageListStruct);
			new -> image = allocateString(r -> image);

			new -> next = last;
			last = new;
		}

		r = r -> next;
	}

	return(last);
}


bool inList(struct imageListStruct *e, struct imageListStruct *l)

{
	struct 	imageListStruct	*p;

	p = l;

	while (p != NULL)
	{
		if (strcmp(p ->  image, e -> image) == 0) return(true);

		p = p -> next;  
	}

	return(false);
}

char *createTemporaryVariable(int type)
{
     
	void	addVariableDeclaration(char *name, int type);
	char	*createNewVariableName();
	char 	*name;
     

   name = createNewVariableName();  
	addVariableDeclaration(name, type);
     
	return(name);
}

char *createNewVariableName()
{
	char      name[11];
     
	temporaryVariableCount++;

	sprintf(name,"t%d",temporaryVariableCount);
     
	return(allocateString(name));
}
      
void addVariableDeclaration(char *name, int type)
{
	struct    declarationStruct        *p;

	p = declarations;

	while (p != NULL)
	{
		if (strcmp(p -> name, name) == 0) return;

		p = p -> next;
	}
     
	p = allocateMemory(declarationStruct);
     
	p -> name = name;
	p -> type = type;
	p -> next = declarations;
     
	declarations = p;
}

void writeValue(FILE *f, int type, union valueUnion *value)
{
	switch (type)
	{
		case	t_integer:		fprintf(f,"%d",value -> ival);
									break;

		case	t_real:			fprintf(f,"%lf",value -> dval);
									break;

		case t_string:
		case t_identifier:	fprintf(f,"%s",value -> sval);
									break;
	}

}

void storeValue(int type, union valueUnion *value, struct valueStruct *target)
{
	target -> type = type;

	switch (type)
	{
		case t_integer:	target -> u.ival = value -> ival;
								break;

		case t_real:		target -> u.dval = value -> dval; 
								break;

		case t_string:		
		case t_identifier:
								target -> u.sval = value -> sval;
								target -> type = t_string;
								break;   
	} 
}




