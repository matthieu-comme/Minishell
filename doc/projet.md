# Projet minishell

### Objectifs
L'objectif du projet est d'implémenter un shell Unix très élémentaire.
Il permettra au minimum de gérer :
* L'exécution de commandes simples avec leurs arguments
  * En avant plan : _cmd arg1 arg2 ..._
  * En arrière plan : _cmd arg1 arg2 ... &_
  * L'exécution avec inversion succès/échec : _! cmd arg1 arg2 ..._
  
* L'exécution inconditionnelle d'une suite de commandes :
  * En avant plan : _cmd1 arg1\_1 arg1\_2 ... ; cmd2 arg2\_1 arg2\_2 ... ; ... ; cmdn argn\_1 argn\_2 ..._
  * En arrière plan : _cmd1 arg1\_1 arg1\_2 ... & cmd2 arg2\_1 arg2\_2 ... & ... & cmdn argn\_1 argn\_2 ..._
  * La combinaision des deux : _cmd1 arg1\_1 arg1\_2 ... ; cmd2 arg2\_1 arg2\_2 ... & cmd3 arg3\_1 arg3\_2 ... ; ... & cmdn argn\_1 argn\_2 ..._
  
* Les exécutions conditionnelles de suites de commandes :
  * Exécution en cas de succès : _cmd1 arg1\_1 arg1\_2 && cmd2 arg2\_1 arg2\_2 ... && cmdn argn\_1 argn\_2 ..._
  * Exécution en cas d'échec : _cmd1 arg1\_1 arg1\_2 || cmd2 arg2\_1 arg2\_2 ... || cmdn argn\_1 argn\_2 ..._
  * Avec inversion éventuelle : _! cmd1 arg1\_1_
  * La combinaison des trois : _cmd1 arg1\_1 arg1\_2 && ! cmd2 arg2\_1 arg2\_2 || cmd3 arg3\_1 arg3\_2 ... && cmdn argn\_1 argn\_2 ..._
  
* Les combinaisons de suites de commandes conditionnelles/inconditionnelles : _cmd1 arg1\_1 arg1\_2 ... ; cmd2 arg2\_1 arg2\_2 ... && cmd3 arg3\_1 arg3\_2 ... || cmd4 arg4\_1 arg4\_2 & ... & cmdn argn/_1 argn\_2 ..._

* Les redirections des IOs des processus :
  * La redirection de l'entrée standard : _cmd arg1 arg2 ... < input_
  * La redirection de la sortie standard en mode "remplacement" : _cmd arg1 arg2 ... > output_
  * La redirection de la sortie standard en mode "ajout" : _cmd arg1 arg2 ... >> output_
  * La redirection de la sortie d'erreur en mode "remplacement" : _cmd arg1 arg2 ... > error_
  * La redirection de la sortie d'erreur en mode "ajout" : _cmd arg1 arg2 ... >> error_
  * La redirection de la sortie standard vers la sortie d'erreur : _cmd arg1 arg2 ... >&2
  * La redirection de la sortie d'erreur vers la sortie standard : _cmd arg1 arg2 ... 2>&1_
  * Les redirections via un tube : _cmd1 arg1\_1 arg1\_2 ... | cmd2 arg2\_1 arg2\_2 ..._
  * Les compositions de redirections : _cmd1 arg1\_1 arg1\_2 ... 2>&1 < input | cmd2 arg2\_1 arg2\_2 ... 2>> error > output ..._

* Les commandes intégrées avec gestion des redirections éventuelles :
  * cd : déplacement du Current Working Directory (CWD) dans le système de fichier
  * exit [val] : sortie du shell avec le code (optionnel) _val_.
  * export _VAR=val_ : définition de variables d'environnement _VAR_.
  * unset _VAR_ : suppression de la variable d'environnement _VAR_.
  * pwd : affichage du CWD.

* Les combinaisons de tout ce qui précède : _cmd1 arg1\_1 arg1\_2 ... < input && cmd2 arg2\_1 arg2\_2 ... | cmd3 arg3\_1 arg3\_2 ... 2>> error > output & cmd4 arg4\_1 arg4\_2 ... || cmd5 arg5\_1 arg5\_2 ... ; ..._

### Code fourni
Pour la réalisation du projet un "squelette" du code vous est fourni. **Son utilisation est obligatoire.**
Les fichiers suivants sont fournis :
* [`include/processus.h`](@ref include/processus.h) et [`src/processus.c`](@ref src/processus.c) : définition des structures et fonctions de gestion des processus et de leurs enchaînements en fonction du flux d'instruction.
* [`include/parser.h`](@ref include/parser.h) et [`src/parser.c`](@ref src/parser.c) : fonctions d'analyse et de traduction des commandes en flux d'instructions dans les structure de [`include/processus.h`](@ref include/processus.h).
* [`include/builtins.h`](@ref include/builtins.h) et [`src/builtins.c`](@ref src/builtins.c) : gestion des commandes intégrées du shell.
* [`src/main.c`](@ref src/main.c) : implémentation de la boucle générale du shell.

La fonction parse_command_line() contient des exemples pour l'implémentation des opérateurs du shell (';' et '<'), il s'agit donc essentiellement de bien comprendre la structure de données et réfléchir à ce que signifie un opérateur sur l'initialisation de celle-ci.

### Compilation et création de la documentation
Un fichier Makefile est fourni afin de simplifier la compilation du projet. Une commande `make` à la racine du projet provoque la compilation de celui-ci avec la production d'un exécutable nommé `minishell`.
Une commande `make doc` crée la documentation dans le répertoire `doc/` via l'outil [`doxygen`](http://www.doxygen.nl).

* `$ make` : compilation des sources et création de l'exécutable `minishell`
* `$ make doc` : création de la documentation dans le répertoire `doc/html` si l'outil [`doxygen`](http://www.doxygen.nl) est installé.
* `$ make clean` : suppression de tous les fichiers objets `.o`
* `$ make deepclean` : suppression de tous les fichiers objets, de l'exécutable `minishell` et de la documentation.

### Rendu
Tous les fichiers sources rendus doivent comporter les noms et prénoms **des deux binômes** dans l'en-tête du fichier.
Vous nommerez le répertoire du projet de la manière suivante : `<numéro de binôme>_Nom1_Nom2`
Et l'archive de celui-ci : `<numéro de binôme>_Nom1_Nom2.[zip|tar.gz|tar.bz2|tar.xz]`
Le format de l'archive doit être **exclusivement** `.zip`, `.tar.gz`, `tar.bz2` ou `tar.xz` **à l'exclusion de tout autre format**.

Par exemple, le binôme n°2 constitué de Jean Martin et Marie Dupont, rend un projet regroupé dans le répertoire `02_Martin_Dupont` compressé dans l'archive nommée `02_Martin_Dupont.tar.xz`.

Vous prendrez soin de ne rendre que les fichiers sources (i.e. ni l'exécutable ni les fichiers objets, ni la documentation créée par [`doxygen`](http://www.doxygen.nl)).
