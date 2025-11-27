#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "../include/builtins.h"
#include "../include/processus.h"
#include <linux/limits.h>

void test_is_builtin()
{
    printf("Démarrage des tests unitaires pour is_builtin...\n");

    processus_t *cmd = malloc(sizeof(processus_t));
    init_processus(cmd);

    assert(is_builtin(NULL) == 0);
    printf("[PASS] Test 1 : Structure NULL\n");

    cmd->argv[0] = NULL;
    assert(is_builtin(cmd) == 0);
    printf("[PASS] Test 2 : Commande vide (argv[0] NULL)\n");

    char *valid_cmds[] = {"cd", "exit", "export", "unset", "pwd"};
    int nb_valid = 5;

    for (int i = 0; i < nb_valid; i++)
    {
        cmd->argv[0] = valid_cmds[i];
        if (is_builtin(cmd) != 1)
        {
            fprintf(stderr, "[FAIL] Echec pour la commande valide : %s\n", valid_cmds[i]);
            exit(1);
        }
    }
    printf("[PASS] Test 3 : Commandes built-in reconnues\n");

    char *invalid_cmds[] = {"ls", "grep", "echo", "cat", "./script.sh", "CD", "Exit"};
    int nb_invalid = 7;

    for (int i = 0; i < nb_invalid; i++)
    {
        cmd->argv[0] = invalid_cmds[i];
        if (is_builtin(cmd) != 0)
        {
            fprintf(stderr, "[FAIL] Faux positif pour la commande : %s\n", invalid_cmds[i]);
            exit(1);
        }
    }
    printf("[PASS] Test 4 : Commandes externes ignorées\n");

    free(cmd);

    printf("Tous les tests pour is_builtin ont réussi !\n\n");
}

void test_builtin_cd()
{
    printf("Démarrage des tests unitaires pour builtin_cd...\n");

    char initial_cwd[PATH_MAX];
    char current_cwd[PATH_MAX];
    char old_home[PATH_MAX];

    // sauvegarde de l'env
    if (getcwd(initial_cwd, sizeof(initial_cwd)) == NULL)
    {
        perror("getcwd init");
        exit(1);
    }
    strcpy(old_home, getenv("HOME"));

    processus_t *cmd = malloc(sizeof(processus_t));
    if (!cmd)
        exit(1);
    init_processus(cmd);

    // redirection de stderr pour ne pas flood le terminal
    cmd->stderr_fd = open("/dev/null", O_WRONLY);

    cmd->argv[0] = "cd";
    cmd->argv[1] = "/tmp";
    cmd->argv[2] = NULL;

    assert(builtin_cd(cmd) == 0);

    getcwd(current_cwd, sizeof(current_cwd));
    assert(strcmp(current_cwd, "/tmp") == 0);
    printf("[PASS] Test 1 : cd /tmp\n");

    cmd->argv[1] = "/dossier/qui/n/existe/pas/12345";

    assert(builtin_cd(cmd) == -1);

    // on doit toujours être dans /tmp
    getcwd(current_cwd, sizeof(current_cwd));
    assert(strcmp(current_cwd, "/tmp") == 0);
    printf("[PASS] Test 2 : cd vers dossier invalide\n");

    setenv("HOME", initial_cwd, 1);
    cmd->argv[1] = NULL;

    assert(builtin_cd(cmd) == 0);

    getcwd(current_cwd, sizeof(current_cwd));
    assert(strcmp(current_cwd, initial_cwd) == 0);
    printf("[PASS] Test 3 : cd HOME (retour départ)\n");

    unsetenv("HOME");
    cmd->argv[1] = NULL;

    assert(builtin_cd(cmd) == -1);
    printf("[PASS] Test 4 : cd sans HOME défini\n");

    if (cmd->stderr_fd >= 0)
        close(cmd->stderr_fd);
    free(cmd);

    // restauration de l'environnement
    setenv("HOME", old_home, 1);
    chdir(initial_cwd);

    printf("Tous les tests pour builtin_cd ont réussi !\n\n");
}

void test_builtin_export()
{
    printf("tototot");
    printf("Démarrage des tests unitaires pour builtin_export...\n");
    printf("avant malloc");
    processus_t *cmd = malloc(sizeof(processus_t));
    printf("après malloc");
    if (!cmd)
        exit(1);
    init_processus(cmd);
    printf("après init");
    cmd->stderr_fd = open("/dev/null", O_WRONLY);

    cmd->argv[0] = "export";
    cmd->argv[1] = "TEST_EXPORT=coucou";
    cmd->argv[2] = NULL;
    printf("après var");

    assert(builtin_export(cmd) == 0);
    assert(strcmp(getenv("TEST_EXPORT"), "coucou") == 0);
    printf("[PASS] Test 1 : export VAR=val\n");

    cmd->argv[1] = "TEST_EXPORT=nouveau";
    assert(builtin_export(cmd) == 0);
    assert(strcmp(getenv("TEST_EXPORT"), "nouveau") == 0);
    printf("[PASS] Test 2 : Mise à jour variable\n");

    cmd->argv[1] = "TEST_EMPTY=";
    assert(builtin_export(cmd) == 0);
    assert(strcmp(getenv("TEST_EMPTY"), "") == 0);
    printf("[PASS] Test 3 : Valeur vide supportée\n");

    cmd->argv[1] = "INVALID_FORMAT";
    assert(builtin_export(cmd) == -1);
    assert(getenv("INVALID_FORMAT") == NULL);
    printf("[PASS] Test 4 : Rejet format sans '='\n");

    cmd->argv[1] = "=VALEUR";
    assert(builtin_export(cmd) == -1);
    printf("[PASS] Test 5 : Rejet nom variable vide\n");

    cmd->argv[1] = "VAR1=un";
    cmd->argv[2] = "VAR2=deux";
    cmd->argv[3] = NULL;

    assert(builtin_export(cmd) == 0);
    assert(strcmp(getenv("VAR1"), "un") == 0);
    assert(strcmp(getenv("VAR2"), "deux") == 0);
    printf("[PASS] Test 6 : Arguments multiples\n");

    unsetenv("TEST_EXPORT");
    unsetenv("TEST_EMPTY");
    unsetenv("VAR1");
    unsetenv("VAR2");

    if (cmd->stderr_fd >= 0)
        close(cmd->stderr_fd);
    free(cmd);
    printf("Tous les tests pour builtin_export ont réussi !\n");
}

void test_builtin_unset()
{
    printf("Démarrage des tests unitaires pour builtin_unset...\n");

    processus_t *cmd = malloc(sizeof(processus_t));
    if (!cmd)
        exit(1);
    init_processus(cmd);

    cmd->stderr_fd = open("/dev/null", O_WRONLY);

    setenv("VAR_TO_DELETE", "exists", 1);
    cmd->argv[0] = "unset";
    cmd->argv[1] = "VAR_TO_DELETE";
    cmd->argv[2] = NULL;

    assert(builtin_unset(cmd) == 0);
    assert(getenv("VAR_TO_DELETE") == NULL);
    printf("[PASS] Test 1 : Suppression simple\n");

    setenv("VAR1", "A", 1);
    setenv("VAR2", "B", 1);
    cmd->argv[1] = "VAR1";
    cmd->argv[2] = "VAR2";
    cmd->argv[3] = NULL;

    assert(builtin_unset(cmd) == 0);
    assert(getenv("VAR1") == NULL);
    assert(getenv("VAR2") == NULL);
    printf("[PASS] Test 2 : Suppression multiple\n");

    unsetenv("NON_EXISTENT");
    cmd->argv[1] = "NON_EXISTENT";
    cmd->argv[2] = NULL;

    assert(builtin_unset(cmd) == 0);
    printf("[PASS] Test 3 : Variable inexistante (Succès attendu)\n");

    // unsetenv échoue si la chaîne contient un '='
    cmd->argv[1] = "VAR=VAL";

    assert(builtin_unset(cmd) == -1);
    printf("[PASS] Test 4 : Identifiant invalide (Erreur attendue)\n");

    if (cmd->stderr_fd >= 0)
        close(cmd->stderr_fd);
    free(cmd);
    printf("Tous les tests pour builtin_unset ont réussi !\n");
}

void test_builtin_pwd()
{
    printf("Démarrage des tests unitaires pour builtin_pwd...\n");

    processus_t *cmd = malloc(sizeof(processus_t));
    if (!cmd)
        exit(1);
    init_processus(cmd);

    cmd->stderr_fd = open("/dev/null", O_WRONLY);

    // --- TEST 1 : Vérification de la sortie ---

    int pipe_fd[2];
    // pipe_fd[0] pour lire, pipe_fd[1] pour écrire
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    cmd->stdout_fd = pipe_fd[1];

    int ret = builtin_pwd(cmd);

    close(pipe_fd[1]);

    assert(ret == 0);

    char output[PATH_MAX];
    memset(output, 0, sizeof(output));
    read(pipe_fd[0], output, sizeof(output) - 1);
    close(pipe_fd[0]);

    // Nettoyage du saut de ligne final ajouté par pwd ('\n') pour comparer
    int len = strlen(output);
    if (len > 0 && output[len - 1] == '\n')
    {
        output[len - 1] = '\0';
    }

    // Vérification avec le vrai getcwd du système
    char expected[PATH_MAX];
    getcwd(expected, sizeof(expected));

    assert(strcmp(output, expected) == 0);
    printf("[PASS] Test 1 : pwd affiche le bon chemin\n");

    // Nettoyage
    if (cmd->stderr_fd >= 0)
        close(cmd->stderr_fd);
    free(cmd);
    printf("Tous les tests pour builtin_pwd ont réussi !\n");
}

int main()
{
    test_is_builtin();
    test_builtin_cd();
    test_builtin_export();
    test_builtin_unset();
    test_builtin_pwd();

    return 0;
}