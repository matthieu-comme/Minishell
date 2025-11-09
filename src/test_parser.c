#include "../include/parser.h"
#include <stdio.h>
#include <string.h>
#define MAX 100
// make clean
// make
// ./test

void print_test_result(char *func_name, int test_result)
{
	printf("- Fonction %s : %s\n", func_name, (test_result == 0) ? "SUCCES" : "ECHEC");
}

void print_expected_result(char *func_name, char *obtained, char *expected)
{
	printf("Après %s: '%s'\nRésultat attendu: '%s'\n\n", func_name, obtained, expected);
}

int test_trim()
{
	int result = 0;
	char s1[] = "        toto 123    !                  ";
	char s2[] = "titi1 2 3    ";
	char s3[] = "               tutu123.";
	char s4[] = "tata123";

	char t1[] = "toto 123    !";
	char t2[] = "titi1 2 3";
	char t3[] = "tutu123.";
	char t4[] = "tata123";

	trim(s1);
	trim(s2);
	trim(s3);
	trim(s4);

	int r1 = strcmp(s1, t1);
	int r2 = strcmp(s2, t2);
	int r3 = strcmp(s3, t3);
	int r4 = strcmp(s4, t4);

	if (r1 || r2 || r3 || r4)
	{
		result = -1;

		if (r1)
			print_expected_result("trim", s1, t1);
		if (r2)
			print_expected_result("trim", s2, t2);
		if (r3)
			print_expected_result("trim", s3, t3);
		if (r4)
			print_expected_result("trim", s4, t4);
	}

	return result;
}

void test_separate_s()
{
	char buffer[256];
	int ret;

	printf("Démarrage des tests unitaires pour separate_s...\n");

	strcpy(buffer, "hello,world");
	ret = separate_s(buffer, ",", 256);
	assert(ret == 0);
	assert(strcmp(buffer, "hello , world") == 0);
	printf("[PASS] Test 1 : Cas simple\n");

	strcpy(buffer, "a+b=c");
	ret = separate_s(buffer, "+=", 256);
	assert(ret == 0);
	assert(strcmp(buffer, "a + b = c") == 0);
	printf("[PASS] Test 2 : Plusieurs séparateurs\n");

	strcpy(buffer, "(bonjour)");
	ret = separate_s(buffer, "()", 256);
	assert(ret == 0);
	assert(strcmp(buffer, " ( bonjour ) ") == 0);
	printf("[PASS] Test 3 : Séparateurs au début/fin\n");

	strcpy(buffer, "wow!!");
	ret = separate_s(buffer, "!", 256);
	assert(ret == 0);
	assert(strcmp(buffer, "wow !  ! ") == 0);
	printf("[PASS] Test 4 : Séparateurs contigus\n");

	strcpy(buffer, "bonjour");
	ret = separate_s(buffer, ",;!", 256);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour") == 0);
	printf("[PASS] Test 5 : Aucun séparateur présent\n");

	buffer[0] = '\0';
	ret = separate_s(buffer, ",", 256);
	assert(ret == 0);
	assert(strlen(buffer) == 0);
	printf("[PASS] Test 6 : Chaîne vide\n");

	strcpy(buffer, "texte");
	ret = separate_s(buffer, "", 256);
	assert(ret == 0);
	assert(strcmp(buffer, "texte") == 0);
	printf("[PASS] Test 7 : Liste séparateurs vide\n");

	strcpy(buffer, "a,b");
	ret = separate_s(buffer, ",", 5);
	assert(ret == -1);
	assert(strcmp(buffer, "a,b") == 0);
	printf("[PASS] Test 8 : Dépassement de taille (Overflow)\n");

	strcpy(buffer, "a,b");
	ret = separate_s(buffer, ",", 6);
	assert(ret == 0);
	assert(strcmp(buffer, "a , b") == 0);
	printf("[PASS] Test 9 : Taille exacte\n");

	ret = separate_s(NULL, ",", 256);
	assert(ret == -1);
	ret = separate_s(buffer, NULL, 256);
	assert(ret == -1);
	printf("[PASS] Test 10 : Pointeurs NULL\n");

	printf("\nTous les tests ont réussi !\n");
}

int test_replace()
{
	int result = 0;

	char s1[MAX] = "toto";
	char s2[MAX] = "abcd";

	char t1[] = "PRoto";
	char t2[] = "abcxy";

	int r1 = replace(s1, "t", "PR", MAX) || strcmp(s1, t1);
	int r2 = replace(s2, "d", "xyzzz", 6) || strcmp(s2, t2);
	int r3 = replace(s1, "blablabal", "", MAX);

	if (r1 || r2 || r3 != -1)
	{
		result = -1;

		if (r1)
			print_expected_result("replace", s1, t1);
		if (r2)
			print_expected_result("replace", s2, t2);
		if (r3)
			print_expected_result("replace", s1, "ERREUR");
	}

	return result;
}

int test_clean()
{
	int result = 0;

	char s1[] = "        to          t          o                   ";
	char s2[] = "abcd";

	char t1[] = " to t o ";
	char t2[] = "abcd";

	int r1 = clean(s1) || strcmp(s1, t1);

	int r2 = clean(s2) || strcmp(s2, t2);

	if (r1 || r2)
	{
		result = -1;

		if (r1)
			print_expected_result("clean", s1, t1);
		if (r2)
			print_expected_result("clean", s2, t2);
	}

	return result;
}

int test_strcut()
{
	int result = 0;
	char s1[] = "chaine de caracteres a decouper en tokens";
	char *tokens1[8] = {NULL};
	char *expected1[8] = {"chaine", "de", "caracteres", "a", "decouper", "en", "tokens", NULL};

	int r1 = strcut(s1, ' ', tokens1, 8); // 7 tokens + NULL
	for (int i = 0; i < 8; i++)
	{
		if (tokens1[i] == NULL)
			continue;
		if (strcmp(tokens1[i], expected1[i]))
			printf("tokens[%d]: %s\nexpected[%d]: %s\n\n", i, tokens1[i], i, expected1[i]);
	}

	printf("%d\n", r1);

	return !(r1 == 7);
}

void test_substenv()
{
	char buffer[1024];
	int ret;

	printf("Démarrage des tests unitaires pour substenv...\n");

	// Configuration de l'environnement pour les tests
	setenv("TEST_VAR", "monde", 1);
	setenv("LONG_VAR", "une_valeur_tres_longue_pour_tester_les_limites", 1);
	setenv("EMPTY_VAR", "", 1);
	unsetenv("NON_EXISTENT_VAR"); // On s'assure qu'elle n'existe pas

	// --- Test 1 : Pas de substitution ---
	strcpy(buffer, "bonjour tout le monde");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour tout le monde") == 0);
	printf("[PASS] Test 1 : Pas de substitution\n");

	// --- Test 2 : Substitution simple $VAR ---
	strcpy(buffer, "bonjour $TEST_VAR");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour monde") == 0);
	printf("[PASS] Test 2 : Substitution simple\n");

	// --- Test 3 : Substitution avec accolades ${VAR} ---
	strcpy(buffer, "bonjour ${TEST_VAR} !");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour monde !") == 0);
	printf("[PASS] Test 3 : Substitution avec accolades\n");

	// --- Test 4 : Variable inexistante (doit être remplacée par vide) ---
	strcpy(buffer, "bonjour $NON_EXISTENT_VAR!");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour !") == 0);
	printf("[PASS] Test 4 : Variable inexistante\n");

	// --- Test 5 : Variables multiples et collées ---
	setenv("A", "1", 1);
	setenv("B", "2", 1);
	strcpy(buffer, "test $A$B ${A}.${B}");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "test 12 1.2") == 0);
	printf("[PASS] Test 5 : Variables multiples\n");

	// --- Test 6 : Syntaxe invalide ou littéraux ---
	// $ seul, ou suivi d'un espace, ou ${ non fermé
	strcpy(buffer, "Prix: 10$ payables. ou ${incomplet");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	// Selon l'implémentation fournie précédemment, ces cas sont traités comme littéraux
	assert(strcmp(buffer, "Prix: 10$ payables. ou ${incomplet") == 0);
	printf("[PASS] Test 6 : Syntaxe invalide ($ littéral)\n");

	// --- Test 7 : Dépassement de taille (Overflow) lors d'une substitution ---
	strcpy(buffer, "Start $LONG_VAR end");
	// La chaine finale devrait faire : 6 ("Start ") + 46 (valeur) + 4 (" end") = 56 chars.
	// On donne un max de 50. Ça doit échouer.
	ret = substenv(buffer, 50);
	assert(ret == -1);
	// Vérifier que le buffer original n'est PAS corrompu en cas d'échec (si l'implémentation le permet)
	assert(strcmp(buffer, "Start $LONG_VAR end") == 0);
	printf("[PASS] Test 7 : Dépassement de taille (Overflow)\n");

	// --- Test 8 : Taille exacte (Limite) ---
	setenv("KEY", "VAL", 1);
	strcpy(buffer, "$KEY");
	// "VAL" fait 3 chars. Il faut max=4 pour inclure le \0.
	ret = substenv(buffer, 4);
	assert(ret == 0);
	assert(strcmp(buffer, "VAL") == 0);

	// Réessai avec max=3 (trop court pour "VAL\0")
	strcpy(buffer, "$KEY");
	ret = substenv(buffer, 3);
	assert(ret == -1);
	printf("[PASS] Test 8 : Tests aux limites de taille exacte\n");

	// --- Test 9 : Cas limites des paramètres ---
	ret = substenv(NULL, 1024);
	assert(ret == -1);
	strcpy(buffer, "$TEST_VAR");
	ret = substenv(buffer, 0);
	assert(ret == -1);
	printf("[PASS] Test 9 : Paramètres invalides (NULL ou size 0)\n");

	// --- Test 10 : Variable vide ---
	strcpy(buffer, "Vide[$EMPTY_VAR]");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "Vide[]") == 0);
	printf("[PASS] Test 10 : Variable vide\n");

	printf("\nTous les tests ont réussi !\n");
}

int main()
{
	print_test_result("test_trim", test_trim());
	print_test_result("test_replace", test_replace());
	print_test_result("test_clean", test_clean());
	print_test_result("test_strcut", test_strcut());
	test_separate_s();
	test_substenv();

	return 0;
}
