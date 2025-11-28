#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define MAX 100
// make clean
// make
// ./test_parser

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
	char buffer[256];
	char *tokens[10];
	int ret;

	printf("Démarrage des tests unitaires pour strcut...\n");

	strcpy(buffer, "un,deux,trois");
	ret = strcut(buffer, ',', tokens, 10);
	assert(ret == 3);
	assert(strcmp(tokens[0], "un") == 0);
	assert(strcmp(tokens[1], "deux") == 0);
	assert(strcmp(tokens[2], "trois") == 0);
	assert(tokens[3] == NULL);
	printf("[PASS] Test 1 : Cas nominal\n");

	strcpy(buffer, "a,b,c,d,e");
	ret = strcut(buffer, ',', tokens, 3); // max=3 ("a", "b", et NULL)
	assert(ret == -1);
	assert(tokens[2] == NULL);
	printf("[PASS] Test 2 : Dépassement de taille (Overflow)\n");

	strcpy(buffer, "a,b,c");
	ret = strcut(buffer, ',', tokens, 4);
	assert(ret == 3);
	assert(strcmp(tokens[2], "c") == 0);
	assert(tokens[3] == NULL);
	printf("[PASS] Test 3 : Taille exacte\n");

	strcpy(buffer, "");
	ret = strcut(buffer, ',', tokens, 10);
	assert(ret == 0);
	assert(tokens[0] == NULL);
	printf("[PASS] Test 4 : Chaîne vide\n");

	strcpy(buffer, "unseultoken");
	ret = strcut(buffer, ',', tokens, 10);
	assert(ret == 1);
	assert(strcmp(tokens[0], "unseultoken") == 0);
	assert(tokens[1] == NULL);
	printf("[PASS] Test 5 : Aucun séparateur\n");

	strcpy(buffer, "a,,b,c,"); // Ignore les virgules en double et à la fin
	ret = strcut(buffer, ',', tokens, 10);
	assert(ret == 3);
	assert(strcmp(tokens[0], "a") == 0);
	assert(strcmp(tokens[1], "b") == 0);
	assert(strcmp(tokens[2], "c") == 0);
	assert(tokens[3] == NULL);
	printf("[PASS] Test 6 : Séparateurs multiples (ignorés)\n");

	strcpy(buffer, "a,b");
	ret = strcut(buffer, ',', tokens, 1);
	assert(ret == -1);
	assert(tokens[0] == NULL);
	printf("[PASS] Test 7 : Cas limite (max=1)\n");

	strcpy(buffer, "a,b");
	ret = strcut(buffer, ',', tokens, 0);
	assert(ret == -1);
	printf("[PASS] Test 8 : Cas limite (max=0)\n");

	printf("\nTous les tests ont réussi !\n");
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

	strcpy(buffer, "bonjour tout le monde");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour tout le monde") == 0);
	printf("[PASS] Test 1 : Pas de substitution\n");

	strcpy(buffer, "bonjour $TEST_VAR");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour monde") == 0);
	printf("[PASS] Test 2 : Substitution simple\n");

	strcpy(buffer, "bonjour ${TEST_VAR} !");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour monde !") == 0);
	printf("[PASS] Test 3 : Substitution avec accolades\n");

	strcpy(buffer, "bonjour $NON_EXISTENT_VAR!");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "bonjour !") == 0);
	printf("[PASS] Test 4 : Variable inexistante\n");

	setenv("A", "1", 1);
	setenv("B", "2", 1);
	strcpy(buffer, "test $A$B ${A}.${B}");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "test 12 1.2") == 0);
	printf("[PASS] Test 5 : Variables multiples\n");

	strcpy(buffer, "Prix: 10$ payables. ou ${incomplet");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "Prix: 10$ payables. ou ${incomplet") == 0);
	printf("[PASS] Test 6 : Syntaxe invalide ($ littéral)\n");

	strcpy(buffer, "Start $LONG_VAR end");
	// La chaine finale devrait faire : 6 ("Start ") + 46 (valeur) + 4 (" end") = 56 chars.
	ret = substenv(buffer, 50);
	assert(ret == -1);
	assert(strcmp(buffer, "Start $LONG_VAR end") == 0);
	printf("[PASS] Test 7 : Dépassement de taille (Overflow)\n");

	setenv("KEY", "VAL", 1);
	strcpy(buffer, "$KEY");
	ret = substenv(buffer, 4);
	assert(ret == 0);
	assert(strcmp(buffer, "VAL") == 0);

	strcpy(buffer, "$KEY");
	ret = substenv(buffer, 3);
	assert(ret == -1);
	printf("[PASS] Test 8 : Tests aux limites de taille exacte\n");

	ret = substenv(NULL, 1024);
	assert(ret == -1);
	strcpy(buffer, "$TEST_VAR");
	ret = substenv(buffer, 0);
	assert(ret == -1);
	printf("[PASS] Test 9 : Paramètres invalides (NULL ou size 0)\n");

	strcpy(buffer, "Vide[$EMPTY_VAR]");
	ret = substenv(buffer, 1024);
	assert(ret == 0);
	assert(strcmp(buffer, "Vide[]") == 0);
	printf("[PASS] Test 10 : Variable vide\n");

	printf("\nTous les tests ont réussi !\n");
}

// Fonction utilitaire pour nettoyer une structure command_line_t entre deux tests
// (Note: Idéalement, il faudrait une fonction free_command_line dans ton projet)
void reset_cmdl(command_line_t *cmdl)
{
	// Fermeture des FDs ouverts par le parsing précédent
	close_fds(cmdl);

	// Réinitialisation basique (attention, ça ne free pas les strdup des argv/path !)
	// Dans un vrai projet, il faudrait boucler sur commands[] pour free path et argv.
	init_command_line(cmdl);
}

void test_parse_command_line()
{
	printf("Démarrage des tests unitaires pour parse_command_line...\n");

	command_line_t *cmdl = malloc(sizeof(command_line_t));
	if (!cmdl)
		exit(1);

	// --- TEST 1 : Commande simple ---
	init_command_line(cmdl);
	const char *line1 = strdup("ls -l -a");

	assert(parse_command_line(cmdl, line1) == 0);
	assert(cmdl->num_commands == 1); // Index 0 utilisé, donc 1 commande
	assert(strcmp(cmdl->commands[0].path, "ls") == 0);
	assert(strcmp(cmdl->commands[0].argv[1], "-l") == 0);
	assert(strcmp(cmdl->commands[0].argv[2], "-a") == 0);
	assert(cmdl->commands[0].argv[3] == NULL);

	printf("[PASS] Test 1 : Commande simple\n");
	reset_cmdl(cmdl);

	// --- TEST 2 : Séquence (;) ---
	// Note: Ta fonction separate_s gère les espaces autour de ';', on teste sans espace
	const char *line2 = "echo un;echo deux";

	assert(parse_command_line(cmdl, line2) == 0);
	// On s'attend à avoir command[0]="echo" et command[1]="echo"
	// Attention: ta logique de num_commands dépend de add_processus.
	// Si add_processus incrémente num_commands, on devrait avoir 2 commandes utilisées.

	assert(strcmp(cmdl->commands[0].path, "echo") == 0);
	assert(strcmp(cmdl->commands[0].argv[1], "un") == 0);

	assert(strcmp(cmdl->commands[1].path, "echo") == 0);
	assert(strcmp(cmdl->commands[1].argv[1], "deux") == 0);

	printf("[PASS] Test 2 : Séquence (;)\n");
	reset_cmdl(cmdl);

	// --- TEST 3 : Redirection sortie (>) ---
	// On doit vérifier que le fichier est créé et que stdout_fd n'est plus 1
	const char *line3 = "ls > test_out.txt";

	assert(parse_command_line(cmdl, line3) == 0);
	assert(cmdl->commands[0].stdout_fd > 2); // Doit être un FD ouvert

	// Vérification que le fichier existe bien
	assert(access("test_out.txt", F_OK) == 0);
	unlink("test_out.txt"); // Nettoyage

	printf("[PASS] Test 3 : Redirection (>)\n");
	reset_cmdl(cmdl);

	// --- TEST 4 : Pipe (|) ---
	// Note: Ton parser ne sépare pas '|' automatiquement avec separate_s.
	// Il faut donc mettre des espaces dans le test pour l'instant : "ls | grep"
	const char *line4 = "ls | grep c";

	assert(parse_command_line(cmdl, line4) == 0);

	// Vérification de la plomberie
	// Cmd 0 (ls) doit avoir stdout != 1
	assert(cmdl->commands[0].stdout_fd != 1);
	assert(cmdl->commands[0].stdout_fd > 2);

	// Cmd 1 (grep) doit avoir stdin != 0
	assert(cmdl->commands[1].stdin_fd != 0);
	assert(cmdl->commands[1].stdin_fd > 2);

	printf("[PASS] Test 4 : Pipe (|)\n");
	reset_cmdl(cmdl);

	// --- TEST 5 : Substitution de variables ($) ---
	setenv("TEST_VAR", "mon_dossier", 1);
	const char *line5 = "cd $TEST_VAR";

	assert(parse_command_line(cmdl, line5) == 0);
	assert(strcmp(cmdl->commands[0].argv[1], "mon_dossier") == 0);

	printf("[PASS] Test 5 : Substitution ($)\n");
	reset_cmdl(cmdl);

	// --- TEST 6 : Erreur de syntaxe (Redirection sans fichier) ---
	// Doit retourner -1 et ne pas crasher
	const char *line6 = "ls >";

	// On redirige stderr pour éviter de polluer l'affichage du test
	int saved_stderr = dup(STDERR_FILENO);
	int null_fd = open("/dev/null", O_WRONLY);
	dup2(null_fd, STDERR_FILENO);

	assert(parse_command_line(cmdl, line6) == -1);

	// Restauration stderr
	dup2(saved_stderr, STDERR_FILENO);
	close(null_fd);
	close(saved_stderr);

	printf("[PASS] Test 6 : Erreur de syntaxe (> sans fichier)\n");
	reset_cmdl(cmdl);

	// --- TEST 7 : Background (&) ---
	const char *line7 = "sleep 10 &";

	assert(parse_command_line(cmdl, line7) == 0);
	assert(cmdl->commands[0].is_background == 1);

	printf("[PASS] Test 7 : Background (&)\n");

	// Nettoyage final
	reset_cmdl(cmdl);
	free(cmdl);

	printf("Tous les tests pour parse_command_line ont réussi !\n");
}

int main()
{
	print_test_result("test_trim", test_trim());
	print_test_result("test_replace", test_replace());
	print_test_result("test_clean", test_clean());
	test_strcut();
	test_separate_s();
	test_substenv();
	test_parse_command_line();

	return 0;
}
