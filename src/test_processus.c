#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/processus.h"
#include <errno.h>

void test_init_processus()
{
    printf("Démarrage du test unitaire pour init_processus...\n");

    assert(init_processus(NULL) == -1);
    printf("[PASS] Gestion du pointeur NULL\n");

    processus_t *proc = malloc(sizeof(processus_t));
    if (!proc)
    {
        perror("malloc");
        exit(1);
    }

    // simule une mémoire réutilisée
    memset(proc, 0xFF, sizeof(processus_t));

    // vérifie que la mémoire est bien sale
    assert(proc->pid != 0);
    assert(proc->stdin_fd != 0);

    int ret = init_processus(proc);
    assert(ret == 0);

    assert(proc->pid == 0);
    assert(proc->argv[0] == NULL);
    assert(proc->envp[0] == NULL);
    assert(proc->path == NULL);

    assert(proc->stdin_fd == 0);
    assert(proc->stdout_fd == 1);
    assert(proc->stderr_fd == 2);

    assert(proc->status == 0);
    assert(proc->is_background == 0);
    assert(proc->invert == 0);

    assert(proc->start_time.tv_sec == 0);
    assert(proc->end_time.tv_sec == 0);

    assert(proc->cf == NULL);

    printf("[PASS] Initialisation correcte des champs\n");

    free(proc);
    printf("Tous les tests pour init_processus ont réussi !\n");
}

void test_launch_processus()
{
    printf("\nDémarrage des tests unitaires pour launch_processus...\n");

    processus_t *proc = malloc(sizeof(processus_t));
    if (!proc)
        exit(1);
    init_processus(proc);

    // --- TEST 1 : Exécution simple (ls) ---
    proc->argv[0] = "ls";
    // On redirige stdout vers /dev/null pour ne pas polluer l'écran
    proc->stdout_fd = open("/dev/null", O_WRONLY);

    assert(launch_processus(proc) == 0);
    assert(proc->status == 0); // ls doit réussir

    // Vérification que les temps ont été remplis
    assert(proc->start_time.tv_sec != 0);
    assert(proc->end_time.tv_sec != 0);

    printf("[PASS] Test 1 : Exécution simple (ls)\n");

    // --- TEST 2 : Commande inexistante ---
    init_processus(proc); // Reset
    proc->argv[0] = "commande_qui_n_existe_pas_12345";
    proc->stderr_fd = open("/dev/null", O_WRONLY); // Silence erreur

    assert(launch_processus(proc) == 0); // La fonction réussit (le fork a marché)
    assert(proc->status == 127);         // Mais le code retour est 127 (not found)

    printf("[PASS] Test 2 : Commande inexistante (Status 127)\n");

    // --- TEST 3 : Redirection de sortie (Capture) ---
    init_processus(proc);
    int pfd[2];
    pipe(pfd);

    proc->argv[0] = "echo";
    proc->argv[1] = "HelloPipe";
    proc->argv[2] = NULL;
    proc->stdout_fd = pfd[1]; // Ecriture dans le tube

    assert(launch_processus(proc) == 0);

    close(pfd[1]); // Fermeture écriture côté père
    char buffer[50];
    memset(buffer, 0, sizeof(buffer));
    read(pfd[0], buffer, sizeof(buffer) - 1);
    close(pfd[0]);

    // echo ajoute un saut de ligne, on vérifie le début
    assert(strncmp(buffer, "HelloPipe", 9) == 0);
    printf("[PASS] Test 3 : Redirection stdout fonctionnelle\n");

    // --- TEST 4 : Vérification du Timing (sleep) ---
    init_processus(proc);
    proc->argv[0] = "sleep";
    proc->argv[1] = "1";
    proc->argv[2] = NULL;

    assert(launch_processus(proc) == 0);

    // Calcul de la durée en secondes
    double start = proc->start_time.tv_sec + (proc->start_time.tv_nsec / 1e9);
    double end = proc->end_time.tv_sec + (proc->end_time.tv_nsec / 1e9);
    double duration = end - start;

    // On vérifie que la durée est d'au moins 1.0 seconde (avec une petite marge d'erreur système)
    assert(duration >= 1.0);
    // On vérifie que ça n'a pas pris 10 secondes non plus
    assert(duration < 2.0);

    printf("[PASS] Test 4 : Mesure du temps (sleep 1s -> %.2fs)\n", duration);

    free(proc);
    printf("Tous les tests pour launch_processus ont réussi !\n");
}

void test_init_control_flow()
{
    printf("\nDémarrage du test unitaire pour init_control_flow...\n");

    assert(init_control_flow(NULL) == -1);
    printf("[PASS] Gestion du pointeur NULL\n");

    control_flow_t *cf = malloc(sizeof(control_flow_t));
    if (!cf)
    {
        perror("malloc");
        exit(1);
    }

    // simule une mémoire qui contient n'importe quoi
    memset(cf, 0xFF, sizeof(control_flow_t));

    // verification que la mémoire est bien sale
    assert(cf->proc != NULL);
    assert(cf->cmdl != NULL);

    int ret = init_control_flow(cf);

    assert(ret == 0);
    assert(cf->proc == NULL);
    assert(cf->unconditionnal_next == NULL);
    assert(cf->on_success_next == NULL);
    assert(cf->on_failure_next == NULL);
    assert(cf->cmdl == NULL);

    printf("[PASS] Nettoyage de la structure (bzero fonctionne)\n");

    free(cf);
    printf("Tous les tests pour init_control_flow ont réussi !\n");
}

void test_add_processus()
{
    printf("\nDémarrage des tests unitaires pour add_processus...\n");

    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);

    // On initialise bien la cmdl pour que num_commands soit à 0
    init_command_line(cmdl);

    // --- TEST 1 : Premier processus (Pas de précédent) ---
    processus_t *p1 = add_processus(cmdl, UNCONDITIONAL);

    assert(p1 != NULL);
    assert(cmdl->num_commands == 1);
    assert(p1 == &cmdl->commands[0]); // Doit être à l'index 0

    // Vérification liaison bidirectionnelle
    assert(p1->cf == &cmdl->flow[0]);
    assert(cmdl->flow[0].proc == p1);

    printf("[PASS] Test 1 : Ajout premier processus\n");

    // --- TEST 2 : Chaînage Inconditionnel (;) ---
    processus_t *p2 = add_processus(cmdl, UNCONDITIONAL);

    assert(p2 != NULL);
    assert(cmdl->num_commands == 2);

    // Vérifie que le processus 1 pointe vers le 2 via "unconditionnal"
    assert(cmdl->flow[0].unconditionnal_next == &cmdl->flow[1]);
    // Les autres pointeurs doivent rester NULL
    assert(cmdl->flow[0].on_success_next == NULL);
    assert(cmdl->flow[0].on_failure_next == NULL);

    printf("[PASS] Test 2 : Chaînage UNCONDITIONAL\n");

    // --- TEST 3 : Chaînage Succès (&&) ---
    processus_t *p3 = add_processus(cmdl, ON_SUCCESS);

    assert(p3 != NULL);
    // Le précédent (p2, index 1) doit pointer vers p3 (index 2) via success
    assert(cmdl->flow[1].on_success_next == &cmdl->flow[2]);
    assert(cmdl->flow[1].unconditionnal_next == NULL);

    printf("[PASS] Test 3 : Chaînage ON_SUCCESS\n");

    // --- TEST 4 : Chaînage Echec (||) ---
    processus_t *p4 = add_processus(cmdl, ON_FAILURE);

    assert(p4 != NULL);
    // Le précédent (p3, index 2) doit pointer vers p4 (index 3) via failure
    assert(cmdl->flow[2].on_failure_next == &cmdl->flow[3]);

    printf("[PASS] Test 4 : Chaînage ON_FAILURE\n");

    // --- TEST 5 : Saturation du tableau (Overflow) ---
    // On remplit jusqu'à MAX_CMDS
    // On est déjà à 4 commandes. On boucle jusqu'à MAX_CMDS.
    for (int i = 4; i < MAX_CMDS; i++)
    {
        assert(add_processus(cmdl, UNCONDITIONAL) != NULL);
    }

    assert(cmdl->num_commands == MAX_CMDS);

    // Tentative d'ajout supplémentaire
    assert(add_processus(cmdl, UNCONDITIONAL) == NULL);
    assert(cmdl->num_commands == MAX_CMDS); // Ne doit pas avoir bougé

    printf("[PASS] Test 5 : Gestion tableau plein (MAX_CMDS)\n");

    free(cmdl);
    printf("Tous les tests pour add_processus ont réussi !\n");
}

void test_next_processus()
{
    printf("\nDémarrage des tests unitaires pour next_processus...\n");

    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);
    init_command_line(cmdl);

    // --- TEST 1 : Tableau vide ---
    // num_commands = 0. Le prochain doit être à l'index 0.
    processus_t *next = next_processus(cmdl);

    assert(next != NULL);
    assert(next == &cmdl->commands[0]);
    printf("[PASS] Test 1 : Tableau vide (Index 0)\n");

    // --- TEST 2 : Après un ajout ---
    // On simule l'ajout d'une commande (on incrémente manuellement pour ce test)
    cmdl->num_commands++; // Maintenant = 1

    next = next_processus(cmdl);
    assert(next != NULL);
    assert(next == &cmdl->commands[1]); // Doit être l'index 1
    printf("[PASS] Test 2 : Après 1 ajout (Index 1)\n");

    // --- TEST 3 : Tableau plein (MAX_CMDS) ---
    // On remplit artificiellement le compteur jusqu'au max
    cmdl->num_commands = MAX_CMDS;

    next = next_processus(cmdl);
    assert(next == NULL); // Doit renvoyer NULL car plus de place
    printf("[PASS] Test 3 : Tableau plein (NULL)\n");

    // --- TEST 4 : Robustesse (NULL) ---
    assert(next_processus(NULL) == NULL);
    printf("[PASS] Test 4 : Gestion pointeur NULL\n");

    free(cmdl);
    printf("Tous les tests pour next_processus ont réussi !\n");
}

void test_add_fd()
{
    printf("\nDémarrage des tests unitaires pour add_fd...\n");

    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);

    // IMPORTANT : init_command_line met tous les descriptors à -1.
    // Sans ça, le test échouerait car add_fd croirait le tableau plein.
    init_command_line(cmdl);

    // --- TEST 1 : Ajout nominal ---
    int fd1 = 42;
    assert(add_fd(cmdl, fd1) == 0);

    // Vérifie qu'il est bien à la première position
    assert(cmdl->opened_descriptors[0] == 42);
    // Vérifie que la suivante est toujours libre
    assert(cmdl->opened_descriptors[1] == -1);

    printf("[PASS] Test 1 : Ajout simple\n");

    // --- TEST 2 : Ajout multiple ---
    int fd2 = 84;
    assert(add_fd(cmdl, fd2) == 0);

    // Doit s'être mis à la suite
    assert(cmdl->opened_descriptors[1] == 84);
    printf("[PASS] Test 2 : Ajout multiple\n");

    // --- TEST 3 : Valeurs invalides ---
    assert(add_fd(cmdl, -1) == -1); // FD négatif
    assert(add_fd(NULL, 10) == -1); // Structure NULL

    // Vérifie que rien n'a bougé
    assert(cmdl->opened_descriptors[2] == -1);
    printf("[PASS] Test 3 : Rejet invalides\n");

    // --- TEST 4 : Saturation (Tableau plein) ---
    // On remplit tout le reste du tableau
    int max_descriptors = MAX_CMDS * 3 + 1;

    // On commence à l'index 2 car 0 et 1 sont déjà pris
    for (int i = 2; i < max_descriptors; i++)
    {
        assert(add_fd(cmdl, i + 100) == 0);
    }

    // Maintenant le tableau est plein à craquer.
    // Tenter d'ajouter un fd supplémentaire doit échouer.
    assert(add_fd(cmdl, 9999) == -1);

    printf("[PASS] Test 4 : Saturation (Full)\n");

    free(cmdl);
    printf("Tous les tests pour add_fd ont réussi !\n");
}

void test_close_fds()
{
    printf("\nDémarrage des tests unitaires pour close_fds...\n");

    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);

    // Initialisation (mets tout à -1)
    init_command_line(cmdl);

    // --- TEST 1 : Fermeture nominale ---
    // On ouvre un vrai fichier (/dev/null) pour avoir un FD valide
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    // On l'ajoute manuellement au tableau
    cmdl->opened_descriptors[0] = fd;

    // Appel de la fonction
    assert(close_fds(cmdl) == 0);

    // Vérification 1 : Le tableau est remis à jour
    assert(cmdl->opened_descriptors[0] == -1);

    // Vérification 2 : Le fichier est BIEN fermé
    // Si on essaie de le refermer, close doit renvoyer -1 et errno = EBADF (Bad File Descriptor)
    int ret = close(fd);
    assert(ret == -1);
    assert(errno == EBADF);

    printf("[PASS] Test 1 : Fermeture fichier réel\n");

    // --- TEST 2 : Gestion d'erreur (FD invalide) ---
    // On met un FD bidon qui n'est pas ouvert (ex: 9999)
    cmdl->opened_descriptors[1] = 9999;

    // close_fds doit essayer de le fermer, échouer, mais continuer.
    // Elle doit retourner -1 à la fin.
    assert(close_fds(cmdl) == -1);

    // Mais l'entrée doit quand même être remise à -1 pour éviter une boucle infinie
    assert(cmdl->opened_descriptors[1] == -1);

    printf("[PASS] Test 2 : Erreur détectée sur FD invalide\n");

    // --- TEST 3 : Robustesse (NULL) ---
    assert(close_fds(NULL) == -1);
    printf("[PASS] Test 3 : Gestion NULL\n");

    free(cmdl);
    printf("Tous les tests pour close_fds ont réussi !\n");
}

void test_init_command_line()
{
    printf("\nDémarrage des tests unitaires pour init_command_line...\n");

    // --- TEST 1 : Robustesse (NULL) ---
    assert(init_command_line(NULL) == -1);
    printf("[PASS] Test 1 : Gestion NULL\n");

    // --- TEST 2 : Nettoyage complet (Mémoire sale) ---
    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);

    // 1. On remplit la mémoire de "1" (données corrompues)
    memset(cmdl, 1, sizeof(command_line_t));

    // Vérifions que c'est bien sale
    assert(cmdl->num_commands != 0);
    assert(cmdl->opened_descriptors[0] != -1);
    assert(cmdl->command_line[0] != '\0');

    // 2. Appel de la fonction
    assert(init_command_line(cmdl) == 0);

    // 3. Vérifications

    // Compteur remis à 0
    assert(cmdl->num_commands == 0);

    // Chaîne vide
    assert(cmdl->command_line[0] == '\0');

    // Tokens NULL
    assert(cmdl->tokens[0] == NULL);
    assert(cmdl->tokens[MAX_CMD_LINE / 2] == NULL);

    // Descripteurs réinitialisés à -1
    int max_descriptors = MAX_CMDS * 3 + 1;
    for (int i = 0; i < max_descriptors; i++)
    {
        if (cmdl->opened_descriptors[i] != -1)
        {
            printf("Echec descripteur index %d : valeur %d\n", i, cmdl->opened_descriptors[i]);
            exit(1);
        }
    }

    // Sous-structures réinitialisées (via init_processus appelé en boucle)
    // On vérifie le premier et le dernier élément pour être sûr
    assert(cmdl->commands[0].pid == 0);
    assert(cmdl->commands[0].stdin_fd == 0);

    assert(cmdl->commands[MAX_CMDS - 1].pid == 0);
    assert(cmdl->commands[MAX_CMDS - 1].stdout_fd == 1);

    // Sous-structures Control Flow
    assert(cmdl->flow[0].proc == NULL);

    printf("[PASS] Test 2 : Initialisation et nettoyage complet\n");

    free(cmdl);
    printf("Tous les tests pour init_command_line ont réussi !\n");
}
void reset_cmdl(command_line_t *cmdl)
{
    close_fds(cmdl);
    init_command_line(cmdl);
}

// Helper pour vérifier si un processus a été lancé
// On regarde si son PID a été mis à jour (non nul)
int has_run(processus_t *p)
{
    return p->pid != 0;
}

void test_launch_command_line()
{
    printf("\nDémarrage des tests unitaires pour launch_command_line...\n");

    command_line_t *cmdl = malloc(sizeof(command_line_t));
    if (!cmdl)
        exit(1);

    processus_t *p1, *p2, *p3;

    // --- TEST 1 : Séquence inconditionnelle (;) ---
    // cmd: true ; true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "true";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, UNCONDITIONAL);
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    assert(launch_command_line(cmdl) == 0);

    assert(has_run(p1));
    assert(has_run(p2));
    printf("[PASS] Test 1 : Séquence (;)\n");

    // --- TEST 2 : AND (&&) - Cas Succès ---
    // cmd: true && true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL); // Le 1er est toujours unconditional par rapport au début
    p1->argv[0] = "true";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, ON_SUCCESS);
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1));
    assert(p1->status == 0); // true renvoie 0
    assert(has_run(p2));     // p2 doit courir car p1 a réussi
    printf("[PASS] Test 2 : AND (Succès -> Suite)\n");

    // --- TEST 3 : AND (&&) - Cas Echec ---
    // cmd: false && true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "false";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, ON_SUCCESS);
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1));
    assert(p1->status != 0); // false renvoie 1
    assert(!has_run(p2));    // p2 NE DOIT PAS courir
    printf("[PASS] Test 3 : AND (Echec -> Stop)\n");

    // --- TEST 4 : OR (||) - Cas Echec (Déclenchement) ---
    // cmd: false || true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "false";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, ON_FAILURE);
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1));
    assert(has_run(p2)); // p2 court car p1 a échoué
    printf("[PASS] Test 4 : OR (Echec -> Suite)\n");

    // --- TEST 5 : OR (||) - Cas Succès (Court-circuit) ---
    // cmd: true || true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "true";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, ON_FAILURE);
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1));
    assert(!has_run(p2)); // p2 ne court PAS car p1 a réussi
    printf("[PASS] Test 5 : OR (Succès -> Skip)\n");

    // --- TEST 6 : Inversion (!) et AND ---
    // cmd: ! false && true
    // false échoue (1), ! inverse (0), donc && se déclenche
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "false";
    p1->argv[1] = NULL;
    p1->invert = 1; // Le flag '!'

    p2 = add_processus(cmdl, ON_SUCCESS); // Attaché par &&
    p2->argv[0] = "true";
    p2->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1));
    assert(p1->status != 0); // Le processus a techniquement échoué...
    // ... MAIS launch_command_line a dû inverser le statut LOCALEMENT pour le flux
    assert(has_run(p2)); // p2 DOIT courir grâce à l'inversion
    printf("[PASS] Test 6 : Inversion (! false && ...)\n");

    // --- TEST 7 : Logique complexe ---
    // cmd: false || echo "Rattrapage" && true
    reset_cmdl(cmdl);

    p1 = add_processus(cmdl, UNCONDITIONAL);
    p1->argv[0] = "false";
    p1->argv[1] = NULL;

    p2 = add_processus(cmdl, ON_FAILURE); // ||
    p2->argv[0] = "true";
    p2->argv[1] = NULL; // Simule "echo rattrapage"

    p3 = add_processus(cmdl, ON_SUCCESS); // &&
    p3->argv[0] = "true";
    p3->argv[1] = NULL;

    launch_command_line(cmdl);

    assert(has_run(p1)); // Fail
    assert(has_run(p2)); // Rattrape (Success)
    assert(has_run(p3)); // Continue car p2 Success
    printf("[PASS] Test 7 : Chaîne complexe (Fail || Success && Success)\n");

    free(cmdl);
    printf("Tous les tests pour launch_command_line ont réussi !\n");
}

int main()
{
    test_init_processus();
    test_launch_processus();
    test_init_control_flow();
    test_add_processus();
    test_next_processus();
    test_add_fd();
    test_close_fds();
    test_init_processus();
    test_launch_command_line();

    return 0;
}