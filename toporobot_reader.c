/* file: toporobot_reader.c
 * author: detrito
 * date: january 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toporobot_reader.h"

int c1 = 0;

void toporobot_process_input_file(const char *filename) {
	int i = 0; // line counter
	char buf[256]; // line bouffer
	
	// read input file
	FILE *file = fopen ( filename, "r" );

	// load a line in the buffer
	while (fgets (buf, sizeof(buf), file)) {
		i++;
		
		if(verbose) {
			printf("\n%i ",i);
		}
		
		toporobot_parse_line(buf);
	}
	
	if (ferror(stdin)) {
		fprintf(stderr,"Error reading stdin\n");
		abort();
	}
}

void toporobot_parse_line(char *buf) {
	const char s[] = "\t";	// tabulator as separator
	char *token; // token between separators
	char *fields[TOPOROBOT_FIELD_ITEMS];	// vector of tokens
	int i = 0; // but start putting token at position 1
		
	// get the first token
	token = strtok(buf, s);
	
	// get tokens until the end of the buffer
	while( token != NULL ) {
		i++;
		fields[i] = token;
		token = strtok(NULL, s); // get next token
	}
	
	c1 = atoi(fields[1]); // value of first column
	
	switch (c1) {
		case -6:
			if(verbose) {
				printf("Name - ");
			}
			toporobot_parse_name(fields);
			break;
		case -5:
			if(verbose) {
				printf("Coordinates - ");
			}
			toporobot_parse_coordinates(fields);
			break;
		case -2:
			if(verbose) {
				printf("Survey - ");
			}
			toporobot_parse_survey(fields);
			break;
		case -1:
			if(verbose) {
				printf("Code - ");
			}
			toporobot_parse_code(fields);			
			break;
		default:
			if(c1>=1) {
				if(verbose) {
					printf("Measure - ");
				}
				toporobot_parse_measure(fields);
			}
			else {
				if(verbose) {
					printf("Assuming newline");
				}
			}
	}
}

void toporobot_parse_name(char **fields) {
	// copy string and delete last caracter (newline)
	strncpy(cave->name, fields[3], strlen(fields[3])-1);
}

void toporobot_parse_coordinates(char **fields) {
	// coordinates of the first point of the first serie
	cave->entrance.x = atoi(fields[3]);
	cave->entrance.y = atoi(fields[4]);
	cave->entrance.z = atoi(fields[5]);
}

void toporobot_parse_survey(char **fields) {
	// allocate the memory for the Serie
	if(verbose) {
		printf("Alloating Survey memory... ");
	}
	
	survey = (Survey*) malloc(sizeof (Survey));
	
	survey->day = atoi(fields[3]); // day
	survey->month = atoi(fields[4]); // month
	survey->year = atoi(fields[5]); // year	
	strcpy(survey->name_person_measuring, fields[6]); // spéléomètre
	strcpy(survey->name_person_drawing, fields[7]); // spéléographe
	survey->auto_declination = atoi(fields[8]);// declination (0=manual)	
	survey->correction_azimuth = atof(fields[9]); // azimuth correction
	survey->correction_dip = atof(fields[10]); // azimuth correction	
	
	cave_push_survey(cave, survey);
}

void toporobot_parse_code(char **fields) {
	int id_code;

	id_code = atoi(fields[2]);
	if(verbose) {
		printf("code: %d\n", id_code);
	}
	code = (Code*) malloc(sizeof (Code));

	// units
	code->unit_azimuth = atoi(fields[3]);
	code->unit_dip = atoi(fields[4]);
	
	// accuracy
	code->accuracy_length = atof(fields[5]);
	code->accuracy_azimuth = atof(fields[6]);
	code->accuracy_dip = atof(fields[7]);

	// manual declination
	/* FIXME
	if(serie->auto_declination == 0) {
		serie->correction_azimuth = atof(fields[8]);
	}
	*/
	
	cave_set_code(cave, code, id_code);
}

void toporobot_parse_measure(char **fields) {
	int id_measure;
	id_measure = atoi(fields[2]);
	
	// -1 at column 2: begin of a new serie
	if( id_measure == -1) {
		if(verbose) {
			printf("Alloating Serie memory... ");
		}
		//serie = (Serie*) malloc(sizeof (Serie));
		serie = (Serie*) malloc(sizeof (Serie) +
			MAX_MEASURE_POINTERS * sizeof(Measure*));
		
		serie->id_serie = c1;
		//cave_add_serie(cave, serie, c1);
		
		// copy string and delete last caracter (newline)
		strncpy(serie->name, fields[10],
			strlen(fields[10])-1);	
		
		// links the serie's begin and end points
		serie->link_begin_serie = atoi(fields[3]);
		serie->link_begin_measure = atoi(fields[4]);
		serie->link_end_serie = atoi(fields[5]);
		serie->link_end_measure = atoi(fields[6]);
		
		cave_push_serie(cave,serie);
	}

	else {
		// set serie's survey and code from the first measure-line
		if(id_measure == 0) {
			code = cave_get_code(cave, atoi(fields[3]));
			if(code) {
				serie_set_code(serie, code);
			}
			else {
				fprintf(stderr,"Error: code %d not found\n", code);
				abort();
			}
		
			survey = cave_get_survey(cave, atoi(fields[4]));
			if(survey) {
				serie_set_survey(serie, survey);
			}
			else {
				fprintf(stderr,"Error: survey %d not found\n", survey);
				abort();
			}
		}
		
		// allocate the memory for a measure		
		measure = (Measure*) malloc(sizeof (Measure));
		measure->length = atof(fields[5]);
		measure->azimuth = atof(fields[6]);
		measure->dip = atof(fields[7]);
		measure->left = atof(fields[8]);
		measure->right = atof(fields[9]);
		measure->up = atof(fields[10]);
		measure->down = atof(fields[11]);
		
		// seto pointer to serie
		measure->serie = serie;
		printf("asd\n");
		// push the measure to the end of the vector of pointers to Series 
		serie_push_measure(serie, measure);
		printf("asd_end\n");
	}
}
