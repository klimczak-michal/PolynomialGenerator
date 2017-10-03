#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <conio.h>
#include <string>
#include <Windows.h>
#include <fstream>
#include <sstream>

using namespace std;

//Pomiar czasu
double PCFreq = 0.0;
__int64 CounterStart = 0;
//------------------------
//---------STALE----------
const int MAXDEGREE = 64; //Max. stopien szukanych wielomianow (max. stopien = MAXDEGREE - 1)
const int POLYNOMIAL_TABLE = 10000; //Tablica na wyniki (wielomiany nierozkladalne)
//--------------------------
//------Zmienne globalne----
//Zmienne pomocnicze uzywane do przechowywania wartosci sprawdzanego wielomianu
unsigned int i_polynomial = 0; //wielomian (dziesietnie)
unsigned int* i_bitPolynomial = new unsigned int[MAXDEGREE]; //wielomian (binarnie)
unsigned int i_degreePolynomial = 0; //stopien wielomianu
unsigned int i_coefficients = 0; //ilosc elementow wielomianu

unsigned long int divisions = 0; // opcjonalne, licznik operacji dzielenia
//Tablica ze znalezionymi wielomianami
unsigned int* i_irreduciblePolynomials = new unsigned int[POLYNOMIAL_TABLE];
//-------------------------
//---Funkcje-pomocnicze----
void initializeTable() //wpisanie poczatkowych wielomianow, konieczne
{
	i_irreduciblePolynomials[0] = 2; //f(x) = x
	i_irreduciblePolynomials[1] = 3; //f(x) = x+1

	for (int i = 2; i < POLYNOMIAL_TABLE; i++)
	{
		i_irreduciblePolynomials[i] = 0;
	}
}

//------Pomiar-czasu-------
void startTimer()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = double(li.QuadPart);

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
double endTimer()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq * 1000;
}
//-------------------------

unsigned int polynomialDegree(unsigned int* a) //stopien wielomianu, indeks najstarzego bita
{
	unsigned int tmp = 0;

	for (int i = 0; i < MAXDEGREE; i++)
	{
		if (a[i] == 1) tmp = i;
	}
	return tmp;
}

unsigned int* polynomialToArray(unsigned int a) //decimal to binary
{
	unsigned int i = 0, i_tmp = a, *res = new unsigned int[MAXDEGREE];

	while (i < MAXDEGREE)
	{
		if (i_tmp > 0)
		{
			res[i] = i_tmp % 2;
			i_tmp /= 2;
		}
		else res[i] = 0; //wypelnia reszte zerami
		i++;
	}

	return res;
}

unsigned int arrayToPolynomial(unsigned int* a, unsigned int sizea) //binary to decimal
{
	unsigned int result = 0;
	for (int i = 0; i < sizea; i++)
	{
		if (a[i] == 1) result += pow(2, i); //lub bez if: result += pow(2*a[i], i)
	}
	//cout << result << endl;
	return result;
}

unsigned int polynomialCoefficients(unsigned int* a, unsigned int sizea) //ilosc elementow niezerowych
{
	unsigned short int i = 0, coefficients = 0;

	while (i < sizea)
	{
		if (a[i] == 1) coefficients++;
		i++;
	}

	return coefficients;
}

bool polynomialCheck(unsigned int polynomial) //(nie)parzystosc liczby, parzystosc oznacza, ze jest rozkladalna i nie ma sensu dalej sprawdzac
{
	if (polynomial % 2 == 1) return true;
	else return false;
}

void display(unsigned int *a, unsigned int sizea) //wyswietl pojedynczy wielomian w postaci wielomianowej
{
	string disp = "";
	string tmp;
	unsigned int counter = 0;

	for (int i = 0; i < sizea; i++)
	{
		if (a[i] == 1)
		{
			counter++;
			if (counter == 1)
			{
				if (i == 0) disp = "1";
				else if (i == 1) disp = "x";
				else
				{
					tmp = to_string(i);
					disp = "x^" + tmp;
				}
			}
			else
			{
				if (i == 1)
				{
					disp = "x + " + disp;
				}
				else
				{
					tmp = to_string(i);
					disp = "x^" + tmp + " + " + disp;
				}
			}
		}
	}
	disp = "f(x) = " + disp;
	cout << disp << endl;
}

void display2(unsigned int* a, unsigned int sizea) //wyswietl pojedynczy wielomian w postaci binarnej
{
	cout << "x = ";
	for (int i = sizea - 1; i > -1; i--)
	{
		cout << a[i];
	}
	cout << endl;
}

void displayTable(unsigned int type) //wyswietl tablice wynikowa (jako wielomiany o okreslonej ilosci skladnikow)
{
	system("cls");
	unsigned int i = 0, *tmp, result = 0;

	if (type == 0)
	{
		unsigned int* tmp = new unsigned int[MAXDEGREE];

		for (int i = 0; i_irreduciblePolynomials[i] != 0; i++)
		{
			tmp = polynomialToArray(i_irreduciblePolynomials[i]);
			display(tmp, MAXDEGREE);
		}
		delete tmp;
	}

	else
	{
		while (i_irreduciblePolynomials[i] != 0)
		{
			tmp = polynomialToArray(i_irreduciblePolynomials[i]);
			if (polynomialCoefficients(tmp, MAXDEGREE) == type)
			{
				display(tmp, MAXDEGREE);
				result++; //licznik na wielomiany o N skladnikach
				delete tmp;
			}

			i++;
		}

		cout << "Irreducible polynomials with " << type << " coefficients: " << result << endl;
	}
}

//nieuzywane na ta chwile
unsigned int* polynomialMultiplication(unsigned int *a, unsigned int* b, unsigned int sizea, unsigned int sizeb) //mnozenie
{
	unsigned int* result = new unsigned int[2 * (MAXDEGREE - 1)];
	//np. ilosc bitow w sprawdzanym wielomianie to n1 = 8
	//a szukamy w 2x mniejszych wartosciach, to n2 = 7
	//wynik mnozenia nie powinien przekroczyc rozmiaru dwukrotnie, czyli 2*(7-1)

	for (int j = 0; j < 32; j++) //inicjacja tablicy
	{
		result[j] = 0;
	}

	for (int i = 0; i <= sizeb; i++) //zwykle mnozenie po kreska
	{
		for (int j = 0; j <= sizea; j++)
		{
			result[j + i] += a[j] * b[i];
		}
	}

	for (int j = 0; j < 32; j++) //operacja modulo 2 na kolejnych potegach wielomianu
	{
		result[j] = result[j] % 2;
	}

	return result;
}

unsigned int* divisionMoveBits(unsigned int degreea, unsigned int* b, unsigned int degreeb) //przesuniecie bitowe
{
	unsigned int distance = degreea - degreeb; //wielkosc "skoku" / przesuniecia
	unsigned int* result = b;

	if (distance == 0) return result; //jezeli przesuniecie jest rowne zero to wroc

	//else
	for (int i = degreeb; i > -1; i--) //od najstarszego bita do 0
	{
		result[i + distance] = result[i]; //przesun o N indeksow w przod (o N w lewo (liczbowo))
		if (i < distance) result[i] = 0; //zeruje mlodsze bity, ktore wciaz maja stare wartosci
	}

	return result;
}

unsigned int polynomialDivision(unsigned int *a, unsigned int *b)
{
	unsigned int* tmp_b = new unsigned int[MAXDEGREE]; //tablica zmieniajacy sie dzielnik (przesuwany)
	unsigned int degreea = polynomialDegree(a); //aktualny stopien dzielnika
	unsigned int degreeb = polynomialDegree(b); //oryginalny stopien dzielnej
	//unsigned int tmp_degreeb = degreeb;

	for (int i = 0; i < MAXDEGREE; i++)
	{
		tmp_b[i] = b[i];
	}


	while (degreea >= degreeb) //dziel dopoki dzielnik miesci sie pod dzielna
	{
		tmp_b = divisionMoveBits(degreea, tmp_b, degreeb); //przesuniecie dzielnika pod najstarszy bit dzielnej

		for (int i = 0; i <= degreea; i++) //petla na XOR, od 0 do stopnia dzielnej
		{
			a[i] = a[i] ^ tmp_b[i]; //XOR
		}

		degreea = polynomialDegree(a); //obliczenie stopnia dzielnej po dzieleniu

		for (int i = 0; i < MAXDEGREE; i++) // przywrocenie dzielnika w oryginalne miejsce
		{
			tmp_b[i] = b[i];
		}
	}
	delete tmp_b;
	return arrayToPolynomial(a, MAXDEGREE); //zwraca reszte jako integer
}

bool polynomialIrreducibleCheck() //single check for irreducibility
{
	unsigned int no = 0, h = i_irreduciblePolynomials[no]; //no - indeks dzielnika, h - wartosc dzielnika
	unsigned int *g_a = new unsigned int[MAXDEGREE], *h_a; //g_a - tablica na dzielna (sprawdzany wielomian), h_a - tablica na dzielniki
	unsigned int remainder = 1; //reszta, wartosc bedzie nadpisana tak czy siak

	h_a = polynomialToArray(h); //zamiana dzielnika z dziesietnego na tablice bitow

	for (int i = 0; i < MAXDEGREE; i++)
	{
		g_a[i] = i_bitPolynomial[i]; //kopiowanie sprawdzanego wielomianu
	}

	do
	{
		remainder = polynomialDivision(g_a, h_a); //obliczanie reszty z dzielenia
		divisions++; //licznik operacji dzielenia, opcjonalne

		if (remainder == 0) //reszta == 0 oznacza, ze dzielnik istnieje, czyli wielomian jest rozkladalny
		{
			//cout << "reducible" << endl;
			return false;
		}

		//else
		no++; //zwiekszenie licznika (przejscie do kolejnego dzielnika)
		h = i_irreduciblePolynomials[no];
		h_a = polynomialToArray(h); //przygotowanie kolejnego dzielnika

		for (int i = 0; i < MAXDEGREE; i++) //odtworzenie wartosci sprawdzanego wielomianu
		{
			g_a[i] = i_bitPolynomial[i];
		}

	} while (h != 0 && polynomialDegree(g_a) > polynomialDegree(h_a)); //koniec petli gdy: kolejny dzielnik jest rowny 0 (dotarl do konca znanych wielomianow nierozkladalanych) i stopien kolejnego dzielnika jest rowny stopniowi sprawdzanego wielomianu

	delete g_a;
	delete h_a;
	//cout << "irreducible" << endl;
	return true; //jezeli wyszedl z while bez przerwania to nie znalazl dzielnika - sukces
}

unsigned int polynomialFinder(unsigned int start, unsigned int end) //main loop
{
	unsigned int irreducible = 2; //zaczyna od 2, bo pierwsze 2 wielomiany sa wpisane na sztywno

	for (int i = start; i < end; i++)
	{
		//przygotwanie do sprawdzenia
		i_polynomial = i;
		i_bitPolynomial = polynomialToArray(i_polynomial); //tablica bitow
		i_coefficients = polynomialCoefficients(i_bitPolynomial, MAXDEGREE); //ilosc skladnikow niezerowych

		bool test = polynomialCheck(i_polynomial); //sprawdzenie parzystosci
		if (test == true) //nieparzysta (na pewno nie dzieli sie przez 2)
		{
			test = polynomialIrreducibleCheck(); //test na nierozkladalnosc
			if (test == true)
			{
				irreducible++; //potrzebne do wyswietlenia wyniku i umiejscowienia nowego wielomianu
				//cout << irreducible << endl; //opcjonalne, wyswietla ile znalazl do tej pory
				i_irreduciblePolynomials[irreducible - 1] = i; //wstawienie do tablicy z wynikami
			}
		}
	}

	return irreducible; //zwraca ilosc do wyswietlenia podsumowania na koniec
}

//Save file
bool fileSavePolynomialTable()
{
	fstream file;
	string tmp = "";

	file.open("C:\\Users\\Michal\\Desktop\\ConsoleApplication1\\polynomials.txt", ios::out/* | ios::app*/);
	if (file.is_open() == true)
	{
		for (int i = 0; i_irreduciblePolynomials[i] != 0; i++)
		{
			tmp = to_string(i_irreduciblePolynomials[i]);
			file << tmp << endl;
		}
		file.close();
		return true;
	}
	else return false;
}

unsigned int fileReadPolynomialTable(unsigned int startDegree, unsigned int endDegree)
{
	fstream file;
	string tmp = "";
	unsigned int start = pow(2, startDegree), end = pow(2, endDegree);
	unsigned int irreducible = 0, test = 0;

	file.open("C:\\Users\\Michal\\Desktop\\ConsoleApplication1\\polynomials.txt", ios::in);
	if (file.is_open() == true)
	{
		for (int i = 0; i < POLYNOMIAL_TABLE; i++)
		{
			if (file.eof() != true)
			{
				getline(file, tmp);
				stringstream(tmp) >> test;
				if (test >= start && test <= end)
				{
					i_irreduciblePolynomials[irreducible] = test;
					irreducible++;
				}
			}
		}
		file.close();

		for (int i = irreducible; i < POLYNOMIAL_TABLE; i++)
		{
			i_irreduciblePolynomials[i] = 0;
		}

		return irreducible;
	}
	else return 0;
}

//"Find&save and read&display"
double search(unsigned int endDegree, unsigned int displayCoefficients)
{
	double time;
	initializeTable();


	startTimer();
	//stopien ustawia sie w zmiennej globalnej MAXDEGREE (ilosc bitow, czyli 16 to bedzie x^15)
	unsigned int a = polynomialFinder(4, pow(2, endDegree)); //przedzial od A do B-1 (4-128 oznacza sprawdzenie 4-127) 
	time = endTimer();

	displayTable(displayCoefficients);

	cout << "Total irreducible polynomials: " << a;
	cout << endl << "Search time:" << time << "[ms]\n\n" << endl;
	//cout << divisions; //opcjonalne, ilosc operacji dzielenia

	if (fileSavePolynomialTable() == true) cout << "Saved.";
	else cout << "Not saved. Couldn't open file";

	_getch();
	return time;
}

void show(unsigned int startDegree, unsigned int endDegree, unsigned int displayCoefficients)
{
	unsigned int a = fileReadPolynomialTable(startDegree, endDegree);
	displayTable(displayCoefficients);
	cout << "Total irreducible polynomials between " << startDegree << " and " << endDegree << " degree: " << a << endl;

}
//----------------------------------------
void menu()
{

}

//--------------------------------------------------
//------------Program----------------
int _tmain(int argc, _TCHAR* argv[])
{
	unsigned int degree = 0, coefficients = 0;

	cout << "Maksymalny stopien wielomianu: ";
	cin >> degree;
	degree++;
	cout << endl << "Ilosc niezerowych elementow wielomianu (jezeli nie ma znaczenia - \"0\"): ";
	cin >> coefficients;

	search(degree, coefficients);

	show(2, 5, 3);

	system("Pause");

	system("pause");
	return 0;
}
