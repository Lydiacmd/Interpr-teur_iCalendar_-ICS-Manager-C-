#include "analyse_syntaxique.h"
#include "analyse_lexical.h"
#include "calendrier_ics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h> // Ajouté pour ftruncate

int main(int argc, char *argv[])
{
   // Vérifier les arguments
   if (argc != 2)
   {
      printf("Usage: %s <fichier_commandes>\n", argv[0]);
      return 1;
   }

   // Ouvrir le fichier d'entrée
   FILE *fichier_entre = fopen(argv[1], "r");
   if (fichier_entre == NULL)
   {
      printf("Impossible d'ouvrir le fichier %s\n", argv[1]);
      return 1;
   }

   // Lire le contenu du fichier
   fseek(fichier_entre, 0, SEEK_END);
   long taille = ftell(fichier_entre);
   rewind(fichier_entre);

   char *buffer = malloc(taille + 1);
   if (buffer == NULL)
   {
      printf("Erreur d'allocation mémoire\n");
      fclose(fichier_entre);
      return 1;
   }

   size_t lu = fread(buffer, 1, taille, fichier_entre);
   buffer[lu] = '\0';
   fclose(fichier_entre);

   // Afficher l'analyse syntaxique
   printf("Analyse syntaxique du fichier %s:\n", argv[1]);
   printf("=====================================\n");

   // Initialiser le lexer et parser
   Lexer lexer;
   lexer_init(&lexer, buffer);

   ASTNode *root = parse(&lexer);

   // Afficher l'arbre syntaxique
   if (root != NULL)
   {
      printf("Commande: %s\n", buffer);
      print_ast(root, 0);
   }
   else
   {
      printf("Erreur d'analyse\n");
      free(buffer);
      return 1;
   }

   // Vérifier si le fichier ICS existe déjà pour déterminer le mode d'ouverture
   int fichier_existe = (access("calendrier.ics", F_OK) != -1);

   // Ouvrir le fichier ICS en mode approprié (création ou mise à jour)
   FILE *fichier_ics;
   if (!fichier_existe)
   {
      // Créer un nouveau fichier ICS
      fichier_ics = fopen("calendrier.ics", "w");
      if (fichier_ics == NULL)
      {
         printf("Impossible de créer le fichier calendrier.ics\n");
         free_ast(root);
         free(buffer);
         return 1;
      }

      // Écrire l'en-tête du fichier ICS
      fprintf(fichier_ics, "BEGIN:VCALENDAR\r\n");
      fprintf(fichier_ics, "VERSION:2.0\r\n");
      fprintf(fichier_ics, "PRODID:-//MesEvenements//FR\r\n"); // Pas de caractères spéciaux
      fprintf(fichier_ics, "CALSCALE:GREGORIAN\r\n");
      fprintf(fichier_ics, "END:VCALENDAR\r\n");
   }
   else
   {
      // Ouvrir le fichier existant en mode lecture/écriture
      fichier_ics = fopen("calendrier.ics", "r+");
      if (fichier_ics == NULL)
      {
         printf("Impossible d'ouvrir le fichier calendrier.ics\n");
         free_ast(root);
         free(buffer);
         return 1;
      }
   }

   // Initialiser le générateur de nombres aléatoires
   srand(time(NULL));

   // Variable pour suivre si le calendrier a été modifié
   int calendrier_modifie = 0;

   // Traiter la commande à partir de l'AST
   traiter_commande(root, fichier_ics, &calendrier_modifie);

   fclose(fichier_ics);
   printf("\nOpération terminée avec succès.\n");

   // Libérer la mémoire
   free_ast(root);
   free(buffer);

   return 0;
}