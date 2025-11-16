/**
 * calendrier_ics.h
 * Gestion du format iCalendar
 */

#ifndef CALENDRIER_ICS_H
#define CALENDRIER_ICS_H

#include "analyse_syntaxique.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Structure pour représenter un événement
typedef struct
{
   char uid[100];
   char titre[256];
   char description[1024];
   char lieu[256];
   char date[11];          // Format: JJ/MM/AAAA
   char heure[6];          // Format: HH:MM
   int duree;              // En minutes
   char date_creation[20]; // Format: AAAAMMJJTHHMM00Z
} Evenement;

// Traiter une commande à partir de l'AST
void traiter_commande(ASTNode *node, FILE *fichier_ics, int *calendrier_modifie);

// Ajouter un événement au calendrier
void ajouter_evenement(ASTNode *node, FILE *fichier_ics);

// Modifier un événement existant
void modifier_evenement(ASTNode *node, FILE *fichier_ics);

// Supprimer un événement
void supprimer_evenement(ASTNode *node, FILE *fichier_ics);

// Lister les événements selon les filtres
void lister_evenement(ASTNode *node, FILE *fichier_ics);

// Rechercher des événements selon les critères
void rechercher_evenements(ASTNode *node, FILE *fichier_ics);

// Vider le calendrier
void vider_calendrier(FILE *fichier_ics);

// Charger tous les événements depuis un fichier ICS
Evenement *charger_evenements(FILE *fichier_ics, int *nombre_evenements);

// Sauvegarder tous les événements dans un fichier ICS
void sauvegarder_evenements(FILE *fichier_ics, Evenement *evenements, int nombre_evenements);

// Générer un UID unique
void generer_uid(char *uid, size_t taille);

// Convertir une date du format JJ/MM/AAAA au format AAAAMMJJ
void convertir_date(const char *date_input, char *date_output);

// Convertir une heure du format HH:MM au format HHMM00
void convertir_heure(const char *heure_input, char *heure_output);

// Calculer l'heure de fin en fonction de l'heure de début et de la durée
void calculer_heure_fin(const char *debut, int duree, char *fin);

// Vérifier si un événement correspond à des critères de recherche
int evenement_correspond(Evenement *ev, const char *titre, const char *lieu, const char *date_debut, const char *date_fin);

// Comparer deux dates au format JJ/MM/AAAA
int comparer_dates(const char *date1, const char *date2);

#endif /* CALENDRIER_ICS_H */