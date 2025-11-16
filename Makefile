CC = gcc
CFLAGS = -Wall -Wextra -g

# Tous les exécutables à construire
all: calendrier lexical_test syntaxique_test

# Dépendances pour les fichiers objets
main.o: main.c analyse_syntaxique.h analyse_lexical.h calendrier_ics.h
	$(CC) $(CFLAGS) -c $<

analyse_lexical.o: analyse_lexical.c analyse_lexical.h
	$(CC) $(CFLAGS) -c $<

analyse_syntaxique.o: analyse_syntaxique.c analyse_syntaxique.h analyse_lexical.h
	$(CC) $(CFLAGS) -c $<

calendrier_ics.o: calendrier_ics.c calendrier_ics.h analyse_syntaxique.h analyse_lexical.h
	$(CC) $(CFLAGS) -c $<

# Fichiers spécifiques pour les tests
lexical_test.o: lexical_test.c analyse_lexical.h
	$(CC) $(CFLAGS) -c $<

syntaxique_test.o: syntaxique_test.c analyse_syntaxique.h analyse_lexical.h
	$(CC) $(CFLAGS) -c $<

# Edition de liens pour chaque exécutable
calendrier: main.o analyse_lexical.o analyse_syntaxique.o calendrier_ics.o
	$(CC) $^ -o $@

lexical_test: lexical_test.o analyse_lexical.o
	$(CC) $^ -o $@

syntaxique_test: syntaxique_test.o analyse_lexical.o analyse_syntaxique.o
	$(CC) $^ -o $@

# Nettoyage des fichiers
clean:
	rm -f calendrier lexical_test syntaxique_test *.o *.d

# Nettoyage complet
distclean: clean
	rm -f calendrier.ics

# Exécution du programme principal
run: calendrier
	./calendrier commandes.txt

# Exécution des tests
run_lexical: lexical_test
	./lexical_test test_lexical.txt

run_syntaxique: syntaxique_test
	./syntaxique_test test_syntaxique.txt


.PHONY: all clean distclean run run_lexical run_syntaxique example