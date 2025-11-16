#include "calendrier_ics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>


void traiter_commande(ASTNode *node, FILE *fichier_ics, int *calendrier_modifie){
   if (node == NULL || node->type != CMD){
      printf("Erreur : Noeud indalide \n");
      return ;
   }
   switch (node->data.cmd.type){
   case CMD_Add:
      ajouter_evenement(node, fichier_ics);
      *calendrier_modifie =1;
      break;
   case CMD_Modify:
      modifier_evenement(node, fichier_ics);
      *calendrier_modifie = 1;
      break;
   case CMD_Remove:
      supprimer_evenement(node, fichier_ics);
      break;
   case CMD_List:
      lister_evenement(node, fichier_ics);
      *calendrier_modifie = 1;
      break;
   case CMD_Search:
      rechercher_evenements(node, fichier_ics);
      break;
   case CMD_Clear:
      vider_calendrier(fichier_ics);
      *calendrier_modifie = 1;
      break;

   default:
      printf("Commande non reconnue\n");
      break;
   }
}

void ajouter_evenement(ASTNode *node, FILE *fichier_ics)
{
    char titre[256] = "Sans titre";
    char description[1024] = "";
    char lieu[256] = "";
    char date[11] = ""; // JJ/MM/AAAA
    char heure[6] = ""; // HH:MM
    int duree = 60;     // minutes par défaut

    Parameter *param = node->data.cmd.parameters;
    while (param != NULL)
    {
        switch (param->type)
        {
        case P_title:
            strcpy(titre, param->value.data.strig);
            break;
        case P_description:
            strcpy(description, param->value.data.strig);
            break;
        case P_location:
            strcpy(lieu, param->value.data.strig);
            break;
        case P_Date:
            strcpy(date, param->value.data.date);
            break;
        case P_Time:
            strcpy(heure, param->value.data.time);
            break;
        case P_duration:
            duree = param->value.data.number;
            break;
        default:
            break;
        }
        param = param->next;
    }

    if (strlen(date) == 0 || strlen(heure) == 0)
    {
        printf("Erreur: date et heure obligatoires pour ADD\n");
        return;
    }

    // Générer UID et DTSTAMP
    char uid[100];
    generer_uid(uid, sizeof(uid));

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char dtstamp[32];
    snprintf(dtstamp, sizeof(dtstamp),
             "%04d%02d%02dT%02d%02d%02dZ",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    // 1) Charger les événements existants
    int nombre_evenements = 0;
    Evenement *evenements = charger_evenements(fichier_ics, &nombre_evenements);
    if (!evenements && nombre_evenements == 0)
    {
        // Aucun événement, ce n'est pas une erreur
        evenements = NULL;
    }

    // 2) Agrandir le tableau pour ajouter un nouvel événement
    Evenement *tmp = realloc(evenements, (nombre_evenements + 1) * sizeof(Evenement));
    if (!tmp)
    {
        printf("Erreur d'allocation mémoire pour l'ajout d'un événement\n");
        free(evenements);
        return;
    }
    evenements = tmp;

    Evenement *ev = &evenements[nombre_evenements];
    memset(ev, 0, sizeof(Evenement));

    strncpy(ev->uid, uid, sizeof(ev->uid) - 1);
    strncpy(ev->titre, titre, sizeof(ev->titre) - 1);
    strncpy(ev->description, description, sizeof(ev->description) - 1);
    strncpy(ev->lieu, lieu, sizeof(ev->lieu) - 1);
    strncpy(ev->date, date, sizeof(ev->date) - 1);
    strncpy(ev->heure, heure, sizeof(ev->heure) - 1);
    ev->duree = duree;
    strncpy(ev->date_creation, dtstamp, sizeof(ev->date_creation) - 1);

    nombre_evenements++;

    // 3) Réécrire complètement le fichier avec TOUS les événements
    rewind(fichier_ics);
    ftruncate(fileno(fichier_ics), 0);

    fprintf(fichier_ics, "BEGIN:VCALENDAR\r\n");
    fprintf(fichier_ics, "VERSION:2.0\r\n");
    fprintf(fichier_ics, "PRODID:-//MesEvenements//FR\r\n");
    fprintf(fichier_ics, "CALSCALE:GREGORIAN\r\n");

    sauvegarder_evenements(fichier_ics, evenements, nombre_evenements);

    fprintf(fichier_ics, "END:VCALENDAR\r\n");

    free(evenements);

    printf("Événement ajouté avec succès: %s\n", titre);
}

// Modifier event
void modifier_evenement(ASTNode *node,FILE *fichier_ics){
   char titre_a_modifier[256] = "";
   
   char nouveau_titre[256] = "";
   char nouveau_lieu[256] = "";
   char nouvelle_description[1024] = "";
   char nouvelle_date[11] = "";
   char nouvelle_heure[6] = "";
   int  nouvelle_duree = -1;

   Parameter *param = node->data.cmd.parameters;
   while (param != NULL){
      switch (param->type){
      case P_title:
         if (strlen(titre_a_modifier) == 0){
            strcpy(titre_a_modifier,param->value.data.strig);
         }else{
            strcpy(nouveau_titre,param->value.data.strig);
         }
            break;
      case P_description: 
         strcpy(nouvelle_description,param->value.data.strig);
         break;
      case P_location:
         strcpy(nouveau_lieu, param->value.data.strig);
         break;
      case P_Date:
         strcpy(nouvelle_date, param->value.data.date);
         break;
      case P_Time:
         strcpy(nouvelle_heure, param->value.data.time);
         break;
      case P_duration:
         nouvelle_duree = param->value.data.number;
         break;
      default:
         break;
      }
      param = param->next;
   }

   if (strlen(titre_a_modifier)== 0 ){
      printf("Erreur: Titre requis pour identifier l'événement à modifier\n");
      return; 
   }

   int nombre_evenements = 0; 
   Evenement *evenements = charger_evenements(fichier_ics,&nombre_evenements);
   if (evenements == NULL){
      printf("Erreur: Impossible de charger les événements\n");
      return; 
   }

   int index_a_modifier = -1 ; 
   for (int i = 0; i < nombre_evenements;i++){
      if(strcmp(evenements[i].titre,titre_a_modifier)== 0){
         index_a_modifier = i;
         break; 
      }
   }
   if (index_a_modifier == -1){
      printf("Erreur : Événement '%s' non trouvé\n", titre_a_modifier);
      free(evenements);
      return; 
   }

   // Modifier l'événement
   if (strlen(nouveau_titre) > 0)
      strcpy(evenements[index_a_modifier].titre, nouveau_titre);

   if (strlen(nouvelle_description) > 0)
      strcpy(evenements[index_a_modifier].description, nouvelle_description);

   if (strlen(nouveau_lieu) > 0)
      strcpy(evenements[index_a_modifier].lieu, nouveau_lieu);

   if (strlen(nouvelle_date) > 0)
      strcpy(evenements[index_a_modifier].date, nouvelle_date);

   if (strlen(nouvelle_heure) > 0)
      strcpy(evenements[index_a_modifier].heure, nouvelle_heure);

   if (nouvelle_duree >= 0)
      evenements[index_a_modifier].duree = nouvelle_duree;

   // Sauvegarder les events modifier
   rewind(fichier_ics); // retorune au debut
   ftruncate(fileno(fichier_ics),0); // Vider le fichier
   
   // Reecrire
   fprintf(fichier_ics, "BEGIN:VCALENDAR\r\n");
   fprintf(fichier_ics, "VERSION:2.0\r\n");
   fprintf(fichier_ics, "PRODID:-//MesÉvénements//FR\r\n");
   fprintf(fichier_ics, "CALSCALE:GREGORIAN\r\n");
   sauvegarder_evenements(fichier_ics, evenements, nombre_evenements);
   fprintf(fichier_ics, "END:VCALENDAR\r\n");
   free(evenements);

   printf("Evenement '%s' modifier avec succes\n",titre_a_modifier);
}

// Supprimer un event 
void supprimer_evenement(ASTNode *node, FILE *fichier_ics){
   char titre_a_supprimer[256] = "";
   Parameter *param = node->data.cmd.parameters; 
   while (param != NULL){
      if (param->type == P_title){
         strcpy(titre_a_supprimer,param->value.data.strig);
         break;
      }
      param = param->next;
   }

   if (strlen(titre_a_supprimer) == 0)
   {
      printf("Erreur: Titre requis pour identifier l'événement à supprimer\n");
      return;
   }

   int nombre_evenements = 0;
   Evenement *evenements = charger_evenements(fichier_ics, &nombre_evenements);

   if (evenements == NULL)
   {
      printf("Erreur: Impossible de charger les événements\n");
      return;
   }

   int index_a_supprimer = -1;
   for (int i = 0; i < nombre_evenements; i++)
   {
      if (strcmp(evenements[i].titre, titre_a_supprimer) == 0)
      {
         index_a_supprimer = i;
         break;
      }
   }
   if (index_a_supprimer == -1)
   {
      printf("Erreur: Événement '%s' non trouvé\n", titre_a_supprimer);
      free(evenements);
      return;
   }
   for (int i = index_a_supprimer; i < nombre_evenements - 1; i++)
   {
      evenements[i] = evenements[i + 1];
   }
   nombre_evenements--;

   // sauvgarder
   rewind(fichier_ics);
   ftruncate(fileno(fichier_ics), 0); // Vider le fichier

   fprintf(fichier_ics, "BEGIN:VCALENDAR\r\n");
   fprintf(fichier_ics, "VERSION:2.0\r\n");
   fprintf(fichier_ics, "PRODID:-//MesÉvénements//FR\r\n");
   fprintf(fichier_ics, "CALSCALE:GREGORIAN\r\n");

   sauvegarder_evenements(fichier_ics, evenements, nombre_evenements);
   fprintf(fichier_ics, "END:VCALENDAR\r\n");
   free(evenements);

   printf("Événement '%s' supprimé avec succès\n", titre_a_supprimer);
}


// Lister les evenements 
void lister_evenement(ASTNode *node, FILE *fichier_ics){
   char date_on[11] = "";
   char date_from[11] = "";
   char date_to[11] = "";
   int all = 0;

   Parameter *param = node->data.cmd.parameters;
   while (param != NULL){

      switch (param->type){
      case P_all:
         /* code */
         all =1; 
         break;
      case P_on:
         strcpy(date_on, param->value.data.date);
         break;
      case P_from:
         strcpy(date_from, param->value.data.date);
         break;
      case P_to:
         strcpy(date_to, param->value.data.date);
         break;
      default:
         break;
      }
      param = param->next;
   }

   // Si aucun filtre n'est spécifié, lister tous les événements
   if (strlen(date_on) == 0 && strlen(date_from) == 0 && strlen(date_to) == 0 && !all)
   {
      all = 1;
   }

   int nombre_evenements = 0;
   Evenement *evenements = charger_evenements(fichier_ics, &nombre_evenements); 
   if (evenements == NULL){
      printf("Erreur: Impossible de charger les événements\n");
      return;
   }

   printf("\n--- Liste des événements ---\n");
   if(nombre_evenements == 0){
      printf("Pas d'événements.\n");
   }else{
      int nb_affiches = 0;
      for (int i = 0; i < nombre_evenements; i++)
      {
         int afficher = 0;
         if (all){
            afficher = 1;
         }else if (strlen(date_from)>0 && strcmp(evenements[i].date,date_on)==0){
            afficher = 1;
         }
         else if (strlen(date_from) > 0 && strlen(date_to) > 0){
            if (comparer_dates(evenements[i].date, date_from) >= 0 &&
                comparer_dates(evenements[i].date, date_to) <= 0){
               afficher = 1;
            }
         }
         else if (strlen(date_from) > 0 && comparer_dates(evenements[i].date, date_from) >= 0) {
            afficher =1;
         }
         else if (strlen(date_to) > 0 && comparer_dates(evenements[i].date, date_to) <= 0){
            afficher =1;
         }

         if (afficher){
            nb_affiches++;
            printf("\nÉvénement #%d:\n", nb_affiches);
            printf("  Titre: %s\n", evenements[i].titre);
            if (strlen(evenements[i].description) > 0)
               printf("  Description: %s\n", evenements[i].description);
            if (strlen(evenements[i].lieu) > 0)
               printf("  Lieu: %s\n", evenements[i].lieu);
            printf("  Date: %s\n", evenements[i].date);
            printf("  Heure: %s\n", evenements[i].heure);
            printf("  Durée: %d minutes\n", evenements[i].duree);
         }
      }
      if (nb_affiches == 0){
         printf("Aucun événement ne correspond aux critères de filtrage.\n");
      }
      
   }

   free(evenements);

}

// Rechercher des événements selon des critères
void rechercher_evenements(ASTNode *node, FILE *fichier_ics)
{
   char terme_recherche[256] = "";

   Parameter *param = node->data.cmd.parameters;
   while (param != NULL)
   {
      if (param->type == P_description)
      {
         strcpy(terme_recherche, param->value.data.strig);
         break;
      }
      param = param->next;
   }

   if (strlen(terme_recherche) == 0)
   {
      printf("Erreur: Terme de recherche requis\n");
      return;
   }

   int nombre_evenements = 0;
   Evenement *evenements = charger_evenements(fichier_ics, &nombre_evenements);

   if (evenements == NULL)
   {
      printf("Erreur: Impossible de charger les événements\n");
      return;
   }

   printf("\n--- Résultats de la recherche pour '%s' ---\n", terme_recherche);

   if (nombre_evenements == 0)
   {
      printf("Aucun événement trouvé.\n");
   }
   else
   {
      int nb_resultats = 0;

      for (int i = 0; i < nombre_evenements; i++)
      {
         // Vérifier si l'événement correspond au terme de recherche
         if (strstr(evenements[i].titre, terme_recherche) != NULL ||
             strstr(evenements[i].description, terme_recherche) != NULL ||
             strstr(evenements[i].lieu, terme_recherche) != NULL)
         {
            nb_resultats++;
            printf("\nÉvénement #%d:\n", nb_resultats);
            printf("  Titre: %s\n", evenements[i].titre);
            if (strlen(evenements[i].description) > 0)
               printf("  Description: %s\n", evenements[i].description);
            if (strlen(evenements[i].lieu) > 0)
               printf("  Lieu: %s\n", evenements[i].lieu);
            printf("  Date: %s\n", evenements[i].date);
            printf("  Heure: %s\n", evenements[i].heure);
            printf("  Durée: %d minutes\n", evenements[i].duree);
         }
      }

      if (nb_resultats == 0)
      {
         printf("Aucun événement ne correspond au terme de recherche.\n");
      }
   }

   free(evenements);
}

// Vider le calendrier
void vider_calendrier(FILE *fichier_ics)
{
   rewind(fichier_ics);
   ftruncate(fileno(fichier_ics), 0); // Vider le fichier

   // Réécrire l'en-tête
   fprintf(fichier_ics, "BEGIN:VCALENDAR\r\n");
   fprintf(fichier_ics, "VERSION:2.0\r\n");
   fprintf(fichier_ics, "PRODID:-//MesÉvénements//FR\r\n");
   fprintf(fichier_ics, "CALSCALE:GREGORIAN\r\n");
   fprintf(fichier_ics, "END:VCALENDAR\r\n");

   printf("Calendrier vidé avec succès.\n");
}

Evenement *charger_evenements(FILE *fichier_ics, int *nombre_evenements)
{
   // Allouer un tableau initial
   Evenement *evenements = NULL;
   int capacite = 0;
   *nombre_evenements = 0;

   long position_initiale = ftell(fichier_ics);

   // Revenir au début du fichier
   rewind(fichier_ics);

   char ligne[1024];
   int dans_evenement = 0;
   Evenement evenement_courant;

   // Lire le fichier ligne par ligne
   while (fgets(ligne, sizeof(ligne), fichier_ics) != NULL)
   {
      // Supprimer les caractères de fin de ligne
      char *fin = strchr(ligne, '\r');
      if (fin)
         *fin = '\0';
      fin = strchr(ligne, '\n');
      if (fin)
         *fin = '\0';

      if (strcmp(ligne, "BEGIN:VEVENT") == 0)
      {
         dans_evenement = 1;
         memset(&evenement_courant, 0, sizeof(Evenement));
         continue;
      }
      else if (strcmp(ligne, "END:VEVENT") == 0)
      {
         dans_evenement = 0;

         // Ajouter l'événement au tableau
         if (*nombre_evenements >= capacite)
         {
            capacite = capacite == 0 ? 10 : capacite * 2;
            evenements = realloc(evenements, capacite * sizeof(Evenement));
            if (!evenements)
            {
               printf("Erreur d'allocation mémoire\n");
               return NULL;
            }
         }

         evenements[*nombre_evenements] = evenement_courant;
         (*nombre_evenements)++;
         continue;
      }

      if (dans_evenement)
      {
         if (strncmp(ligne, "UID:", 4) == 0)
         {
            strcpy(evenement_courant.uid, ligne + 4);
         }
         else if (strncmp(ligne, "SUMMARY:", 8) == 0)
         {
            strcpy(evenement_courant.titre, ligne + 8);
         }
         else if (strncmp(ligne, "DESCRIPTION:", 12) == 0)
         {
            strcpy(evenement_courant.description, ligne + 12);
         }
         else if (strncmp(ligne, "LOCATION:", 9) == 0)
         {
            strcpy(evenement_courant.lieu, ligne + 9);
         }
         else if (strncmp(ligne, "DTSTART:", 8) == 0)
         {
            // Format: AAAAMMJJTHHMM00Z
            char *value_start = ligne + 8;
            char date_str[11];
            char heure_str[6];

            // Extraire l'année, le mois et le jour
            int annee, mois, jour, heure, minute;
            sscanf(value_start, "%4d%2d%2dT%2d%2d", &annee, &mois, &jour, &heure, &minute);

            // Reformater en JJ/MM/AAAA
            sprintf(date_str, "%02d/%02d/%04d", jour, mois, annee);
            strcpy(evenement_courant.date, date_str);

            // Reformater en HH:MM
            sprintf(heure_str, "%02d:%02d", heure, minute);
            strcpy(evenement_courant.heure, heure_str);
         }
         else if (strncmp(ligne, "DTEND:", 6) == 0)
         {
            // Format: AAAAMMJJTHHMM00Z
            char *value_end = ligne + 6;
            int annee_fin, mois_fin, jour_fin, heure_fin, minute_fin;
            sscanf(value_end, "%4d%2d%2dT%2d%2d", &annee_fin, &mois_fin, &jour_fin, &heure_fin, &minute_fin);

            // Récupérer les heures et minutes du début
            int annee_debut, mois_debut, jour_debut, heure_debut, minute_debut;
            sscanf(evenement_courant.date, "%d/%d/%d", &jour_debut, &mois_debut, &annee_debut);
            sscanf(evenement_courant.heure, "%d:%d", &heure_debut, &minute_debut);

            // Calculer la durée en minutes (simple, ne gère pas les changements de jour)
            if (jour_fin == jour_debut && mois_fin == mois_debut && annee_fin == annee_debut)
            {
               int duree = (heure_fin - heure_debut) * 60 + (minute_fin - minute_debut);
               if (duree < 0)
                  duree += 24 * 60; // Si on passe minuit
               evenement_courant.duree = duree;
            }
            else
            {
               // Par défaut si les dates diffèrent
               evenement_courant.duree = 60;
            }
         }
         else if (strncmp(ligne, "DTSTAMP:", 8) == 0)
         {
            strcpy(evenement_courant.date_creation, ligne + 8);
         }
      }
   }
   fseek(fichier_ics, position_initiale, SEEK_SET);

   return evenements;
}
// Sauvegarder tous les événements dans un fichier ICS
void sauvegarder_evenements(FILE *fichier_ics, Evenement *evenements, int nombre_evenements)
{
   for (int i = 0; i < nombre_evenements; i++)
   {
      // Extraire les composants de la date
      int jour, mois, annee;
      sscanf(evenements[i].date, "%d/%d/%d", &jour, &mois, &annee);

      // Extraire les composants de l'heure
      int heures, minutes;
      sscanf(evenements[i].heure, "%d:%d", &heures, &minutes);

      // Calculer l'heure de fin
      int fin_heures = heures;
      int fin_minutes = minutes + evenements[i].duree;
      while (fin_minutes >= 60)
      {
         fin_minutes -= 60;
         fin_heures++;
      }
      fin_heures %= 24;

      // Écrire l'événement
      fprintf(fichier_ics, "BEGIN:VEVENT\r\n");
      fprintf(fichier_ics, "UID:%s\r\n", evenements[i].uid);
      fprintf(fichier_ics, "SUMMARY:%s\r\n", evenements[i].titre);

      if (strlen(evenements[i].description) > 0)
      {
         fprintf(fichier_ics, "DESCRIPTION:%s\r\n", evenements[i].description);
      }

      if (strlen(evenements[i].lieu) > 0)
      {
         fprintf(fichier_ics, "LOCATION:%s\r\n", evenements[i].lieu);
      }

      // Date et heure de début
      fprintf(fichier_ics, "DTSTART:%04d%02d%02dT%02d%02d00Z\r\n",
              annee, mois, jour, heures, minutes);

      // Date et heure de fin
      fprintf(fichier_ics, "DTEND:%04d%02d%02dT%02d%02d00Z\r\n",
              annee, mois, jour, fin_heures, fin_minutes);

      // Horodatage de création
      if (strlen(evenements[i].date_creation) > 0)
      {
         fprintf(fichier_ics, "DTSTAMP:%s\r\n", evenements[i].date_creation);
      }
      else
      {
         // Générer un nouvel horodatage si nécessaire
         time_t now = time(NULL);
         struct tm *t = localtime(&now);
         fprintf(fichier_ics, "DTSTAMP:%04d%02d%02dT%02d%02d%02dZ\r\n",
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);
      }

      fprintf(fichier_ics, "END:VEVENT\r\n");
   }
}

// Générer un UID unique
void generer_uid(char *uid, size_t taille)
{
   time_t now = time(NULL);
   struct tm *t = localtime(&now);
   snprintf(uid, taille, "%04d%02d%02d%02d%02d%02d-%06d@calendrier",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, rand() % 1000000);
}

// Convertir une date du format JJ/MM/AAAA au format AAAAMMJJ
void convertir_date(const char *date_input, char *date_output)
{
   int jour, mois, annee;
   sscanf(date_input, "%d/%d/%d", &jour, &mois, &annee);
   sprintf(date_output, "%04d%02d%02d", annee, mois, jour);
}

// Convertir une heure du format HH:MM au format HHMM00
void convertir_heure(const char *heure_input, char *heure_output)
{
   int heures, minutes;
   sscanf(heure_input, "%d:%d", &heures, &minutes);
   sprintf(heure_output, "%02d%02d00", heures, minutes);
}

// Calculer l'heure de fin en fonction de l'heure de début et de la durée
void calculer_heure_fin(const char *debut, int duree, char *fin)
{
   int heures, minutes;
   sscanf(debut, "%d:%d", &heures, &minutes);

   int fin_minutes = minutes + duree;
   int fin_heures = heures + (fin_minutes / 60);
   fin_minutes %= 60;
   fin_heures %= 24;

   sprintf(fin, "%02d%02d00", fin_heures, fin_minutes);
}

// Vérifier si un événement correspond à des critères de recherche
int evenement_correspond(Evenement *ev, const char *titre, const char *lieu, const char *date_debut, const char *date_fin)
{
   if (titre != NULL && strlen(titre) > 0)
   {
      if (strstr(ev->titre, titre) == NULL)
         return 0;
   }

   if (lieu != NULL && strlen(lieu) > 0)
   {
      if (strstr(ev->lieu, lieu) == NULL)
         return 0;
   }

   if (date_debut != NULL && strlen(date_debut) > 0)
   {
      if (comparer_dates(ev->date, date_debut) < 0)
         return 0;
   }

   if (date_fin != NULL && strlen(date_fin) > 0)
   {
      if (comparer_dates(ev->date, date_fin) > 0)
         return 0;
   }

   return 1;
}

// Comparer deux dates au format JJ/MM/AAAA
int comparer_dates(const char *date1, const char *date2)
{
   int jour1, mois1, annee1, jour2, mois2, annee2;
   sscanf(date1, "%d/%d/%d", &jour1, &mois1, &annee1);
   sscanf(date2, "%d/%d/%d", &jour2, &mois2, &annee2);

   if (annee1 != annee2)
      return annee1 - annee2;
   if (mois1 != mois2)
      return mois1 - mois2;
   return jour1 - jour2;
}