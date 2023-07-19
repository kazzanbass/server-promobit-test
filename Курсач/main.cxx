#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

void secondTask(void);
void secondBTask(void);
void firstTask(void);

int main()
{

	int choice;

	cout << "Программа курсовой работы Корниенко Романа\n";

	for (;;)
	{

		cout << "\n\n\n\n"
			 << "_____________________________";
		cout << ("\nВведите номер задания:\n1 - задание 1\n2 - задание 2\n3 - задание 3\n4 - выход из программы\n==> ");
		cin >> choice;
		cout << "\n";

		switch (choice)
		{
		case 1:
			firstTask(); // Функция, имеющая решение первого задания
			break;
		case 2:
			secondTask(); // Функция, имеющая решение второго задания
			break;
		case 3:
			secondBTask(); // Функция, имеющая решение третьего задания
			break;
		case 4:
			exit(0); // Выход из приложения
			break;
		}
	}
	return 0;
}

void secondTask(void)
{

	int **a;
	int *prosmas;
	int n, m, pros, b, c, i, j;

	srand(time(NULL)); // Формирования периода изменения случайных чисел

	cout << "Введите n(Кол-во строк)==>";
	cin >> n;
	cout << "Введите m(Кол-во столбцов)==>";
	cin >> m;

	prosmas = (int *)malloc(sizeof(int) * m); // Выделение памяти для указателя

	a = (int **)malloc(n * sizeof(int *)); // Выделение памяти для указателя на указатель, в нем хранится n-е кол-во указателей
	for (i = 0; i < n; i++)
	{
		*(a + i) = (int *)malloc(m * sizeof(int)); // Выделение памяти для подуказателей
	}

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
		{
			a[i][j] = rand() % 20 + (-10); // Формирование случайного числа для динамического двумерного массива
		}
	}

	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < m; ++j)
		{
			cout << a[i][j] << "\t"; // Вывод элементов двумерного массива
		}
		cout << "\n";
	}

	for (b = 0; b < m; b++)
	{
		pros = 1; // Переменная для вычисления произведения
		for (c = 0; c < n; c++)
		{
			if (a[c][b] < 0)
			{
				pros = a[c][b] * pros;
				prosmas[b] = pros; // Запись значения переменной в массив
			}
		}
	}

	cout << "Массив : \n";

	for (b = 0; b < m; b++)
	{
		cout << prosmas[b] << "\t"; // Вывод эл-в массива
	}
}

void secondBTask(void)
{

	int **a;
	int *b;
	int n, p, j, m, i, o, max;

	srand(time(NULL)); // Формирования случайного числа

	cout << "Введите n ==>";
	cin >> n;

	a = (int **)malloc(sizeof(int *) * n); // Выделения памяти для двумерного массива
	b = (int *)malloc(sizeof(int) * n);	   // Выделения памяти для одномерного массива

	for (int i = 0; i < n; i++)
	{
		*(a + i) = (int *)malloc(sizeof(int) * n); // Выделение памяти для подмассиву
	}

	for (m = 0; m < n; m++)
	{
		for (j = 0; j < n; j++)
		{
			a[m][j] = rand() % 20 - 10; // Запись случайного значения в эл-ты массива
		}
	}

	for (int m = 0; m < n; m++)
	{
		for (j = 0; j < n; j++)
		{
			cout << a[m][j] << "\t"; // Вывод эл-ов массива
		}
		cout << "\n";
	}

	cout << "\n";

	for (m = 0; m < n; m++)
	{
		b[m] = a[m][n - 1 - m]; // Запись в массив побочной диагонали
	}

	cout << "Массив : \n";

	for (i = 0; i < n; i++)
		cout << b[i] << "\t"; // Вывод массив

	for (o = 0; o < n - 1; o++)
	{ // Сортировка выбором
		max = o;
		for (int i = o + 1; i < n; i++)
		{
			if (*(b + i) > *(b + max))
			{
				max = i;
			}
		}
		if (max != o)
		{
			p = *(b + max);
			*(b + max) = *(b + o);
			*(b + o) = p;
		}
	}

	cout << "\nСортированный массив :\n";

	for (i = 0; i < n; i++)
	{
		cout << b[i] << " ";
	}
}

void firstTask(void)
{

	int x, i, o, min, p, minimum, minz, maxz, maximum, k, f, s;
	int *mas;
	cout << "Введите кол-во элементов в массиве ==> ";
	cin >> x;
	cout << "\n";
	srand(time(0)); // Формирование случайных чисел

	mas = (int *)malloc(sizeof(int) * x); // Выделение памяти для указателя

	for (k = 0; k < x; k++)
	{
		*(mas + k) = rand() % 100; // Заполнение эл-ов массива случайными числами
		cout << *(mas + k) << "\n";
	}

	minimum = *(mas + 0);
	minz = 0;
	for (f = 0; f < x; f++) // Поиск минимального числа
	{
		if (mas[f] < minimum)
		{
			minimum = mas[f];
			minz = f;
		}
	}

	cout << "\nминимальный номер ==>" << minz + 1 << " число " << minimum << "\n";

	maximum = mas[0];
	maxz = 0;
	for (s = 0; s < x; s++) // Поиск максимального числа
	{
		if (mas[s] >= maximum)
		{
			maximum = mas[s];
			maxz = s;
		}
	}
	cout << "максимальный номер ==>" << maxz + 1 << " число " << maximum << "\n\n";

	if (maxz < minz)
		cout << "Да\n\n";
	else if (minz < maxz)
		cout << "Нет\n\n";
	else
		cout << "Одинаково\n\n";

	/************Сортировка выбором***************/
	for (o = 0; o < x - 1; o++)
	{
		min = o; // Запись номера эл-а с минимальным значением как значение переменной o
		for (i = o + 1; i < x; i++)
		{
			if (*(mas + i) < *(mas + min))
			{			 // Если значение эл-та с номером i меньше чем значение эл-та с номером min
				min = i; // То запись номера эл-а с минимальным значением как значение переменной i
			}
		}
		if (min != o)
		{
			p = *(mas + min); // Переменная-буфер, используемая для обмена значениями между эл-ми массива
			*(mas + min) = *(mas + o);
			*(mas + o) = p;
		}
	}

	cout << "Упорядоченный массив:\n";
	for (k = 0; k < x; k++)
	{
		cout << *(mas + k) << "\n"; // Вывод эл-ов упорядоченного массива
	}
}
