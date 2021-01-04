#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proc.h"
#include "calendar.h"
#include "util.h"
#include "colors.h"

s_arr_spreadsheets* h_proc_alloc(int initial_capacity)
{
	s_arr_spreadsheets* arr_spreadsheets;
	int i;

	arr_spreadsheets = malloc(sizeof(s_spreadsheet));
	if (arr_spreadsheets == NULL)
		return NULL;

	arr_spreadsheets->spreadsheets = malloc(initial_capacity * sizeof(s_spreadsheet));
	if (arr_spreadsheets->spreadsheets == NULL)
		return NULL;

	arr_spreadsheets->used = 0;
	arr_spreadsheets->max_capacity = initial_capacity;

	for (i = 0; i <= arr_spreadsheets->max_capacity; i++)
	{
		arr_spreadsheets->spreadsheets[i].weekend_days = 0;
		arr_spreadsheets->spreadsheets[i].half_days = 0;
		arr_spreadsheets->spreadsheets[i].full_days = 0;
		arr_spreadsheets->spreadsheets[i].absent_days = 0;
	}

	return arr_spreadsheets;
}

s_arr_spreadsheets* h_proc_open(const char* filename)
{
	FILE* fp;
	s_arr_spreadsheets* array;
	int file_size;

	fprintf(stdout, YELLOW("[!] Importing: %s\n"), filename);

	fp = fopen(filename, "rb");
	if (fp == NULL)
		return NULL;

	file_size = 0;
	while (!feof(fp))
		file_size++;

	array = h_proc_alloc(file_size);
	if (array == NULL)
		return NULL;

	for (int i = 0; !feof(fp); i++)
	{
		if (array->used == array->max_capacity)
		{
			array = realloc(array->spreadsheets, array->max_capacity * 2);
			if (array == NULL)
				return NULL;

			array->max_capacity *= 2;
		}

		if (fread(&array->spreadsheets[i], sizeof(s_spreadsheet), 1, fp) != 1)
			return NULL;

		array->used++;
	}

	return array;
}

s_error* h_proc_add(s_arr_spreadsheets* array, e_month month)
{
	if (array == NULL)
		return h_error_create(H_ERROR_READ, "h_proc_add()");

	if (array->used == array->max_capacity)
	{
		array->spreadsheets = realloc(array->spreadsheets, (array->max_capacity * 2) * sizeof(s_spreadsheet));
		if (array->spreadsheets == NULL)
			return h_error_create(H_ERROR_ALLOCATION, "h_irs_add(): array->data");

		array->max_capacity *= 2;
	}

	array->used++;
	array->spreadsheets[array->used].full_days = h_util_get_int(1, h_calendar_days_in_month(month), "Dias completos");
	array->spreadsheets[array->used].half_days = h_util_get_int(1, h_calendar_days_in_month(month), "Meios dias");
	// TODO: Um mês tem 4 fins de semana mais o menos
	array->spreadsheets[array->used].weekend_days = h_util_get_int(1, h_calendar_days_in_month(month), "Fins de semana");
	array->spreadsheets[array->used].absent_days = h_util_get_int(1, h_calendar_days_in_month(month), "Faltas");

	return NULL;
}

char* h_proc_generate_filename(e_month month, const char* extension)
{
	const char* month_str;
	char* filename;

	month_str = h_calendar_str_from_month(month);
	if (month_str == NULL)
		return NULL;

	filename = malloc(MAX_FILENAME * sizeof(char));
	if (filename == NULL)
		return NULL;

	if (memset(filename, '\0', MAX_FILENAME) == NULL)
		return NULL;

	if (strcat(filename, "../data/spreadsheet_") == NULL)
		return NULL;

	if (strcat(filename, month_str) == NULL)
		return NULL;

	if (strcat(filename, extension) == NULL)
		return NULL;
}

/*
s_error* h_proc_export_csv(s_sheet* sheet, const char* path)
{
	int i;
	FILE* fp;

	fp = fopen(path, "w");
	if (fp == NULL)
		return h_error_create(H_ERROR_WRITE, path);

	for (i = 0; i <= sheet->used; i++)
	{
		fprintf(fp, "%s;%d;%d;%d;%d;\n",
			sheet->paysheet[i].func_code,
			sheet->paysheet[i].full_days,
			sheet->paysheet[i].half_days,
			sheet->paysheet[i].weekend_days,
			sheet->paysheet[i].absent_days);
	}

	fclose(fp);
	return NULL;
}*/

/*
s_error* h_processing(s_sheet* sheet, s_arr_irs irs_array, s_arr_seg_social ss_array)
{
	int i, j, k;
	float base_salary, food_allowance, irs_retention, ss_retention_employer, ss_retention_employee;
	float irs_retention_percentage = 0, ss_retention_employer_percentage, ss_retention_employee_percentage;

	for (i = 0; i <= sheet->used; i++)
	{
		//Falta definir as constantes do sálario de acordo com o cargo do trabalhador.
		base_salary = sheet->paysheet[i].full_days * 40 + (sheet->paysheet[i].half_days * 40) / 2
					  + (sheet->paysheet[i].weekend_days * 40) * 1.5;
		//Falta definir as constantes do subsídio da alimentação de acordo com o cargo do trabalhador.
		food_allowance = sheet->paysheet[i].full_days * 5 + sheet->paysheet[i].weekend_days * 5;

		//Falta aceder aos dados dos trabalhadores para determinar o escalão de IRS.
		irs_retention_percentage = irs_array.data[i].monthly_pay_value / 100;
		irs_retention = (base_salary + food_allowance) * irs_retention_percentage;

		//Falta aceder aos dados dos trabalhadores para determinar as percentagens de descontos da SS.
		ss_retention_employer_percentage = ss_array.data[i].employer / 100;
		ss_retention_employee_percentage = ss_array.data[i].employee / 100;

		ss_retention_employer = (base_salary + food_allowance) * ss_retention_employer_percentage;
		ss_retention_employee = (base_salary + food_allowance) * ss_retention_employee_percentage;
	}

	printf("\nSalário Ilíquido: %.2f€\n", base_salary);
	printf("Subsídio da Alimentação: %.2f€\n", food_allowance);

	printf("\nRetenção IRS: %.2f€\n", irs_retention);
	printf("Retenção SS (Empregador): %.2f€\n", ss_retention_employer);
	printf("Retenção SS (Empregado): %.2f€\n", ss_retention_employee);

	printf("\nSalário Líquido: %.2f€\n", (base_salary + food_allowance) - irs_retention - ss_retention_employee);
	printf("Encargo Total (Empregador): %.2f€\n",
		(base_salary + food_allowance) + irs_retention + ss_retention_employee + ss_retention_employer);
	return NULL;
}*/