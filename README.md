# Interprteur iCalendar
Interpréteur .ics en C avec gestion d'événements (ADD/MODIFY/REMOVE/LIST/SEARCH/CLEAR) et menu interactif Bash.


## 🚀 Commandes disponibles

| Commande | Description | Exemple |
|----------|-------------|---------|
| `ADD` | Ajouter un événement | `ADD TITLE "Cours" DATE 20/11/2025 TIME 10:00 DURATION 90` |
| `MODIFY` | Modifier un événement | `MODIFY TITLE "Cours" TIME 13:00` |
| `REMOVE` | Supprimer un événement | `REMOVE TITLE "Sport"` |
| `LIST` | Lister les événements | `LIST ALL` ou `LIST FROM 01/12/2025 TO 31/12/2025` |
| `SEARCH` | Rechercher | `SEARCH "muscu"` |
| `CLEAR` | Vider le calendrier | `CLEAR` |

## ⚙️ Installation et Utilisation

### 1. Compilation
```bash
make clean && make
```

### 2. Utilisation

# 🌟 Mode interactif (recommandé) :
```bash
./calendrier_user.sh
```
## ✨ Interface Utilisateur : `calendrier_user.sh`

Le projet inclut un **menu interactif en Bash** qui facilite l'utilisation de l'interpréteur sans avoir à mémoriser la syntaxe des commandes.

# Fonctionnalités du menu :
- ✅ **Interface guidée** : questions/réponses pour chaque paramètre
- ✅ **Validation automatique** : génération de commandes correctes
- ✅ **Pas de syntaxe à retenir** : le script génère les commandes pour vous
- ✅ **Gestion complète** : toutes les opérations disponibles via le menu

```bash
chmod +x calendrier_user.sh
./calendrier_user.sh
```


# 📝 Mode fichier :
```bash
./calendrier fichier_commandes.txt
```

**📤 Import dans Google Calendar :**
Le fichier `calendrier.ics` généré peut être directement importé dans :
- Google Calendar
- Outlook
- Apple Calendar
- Tout client iCalendar (RFC 5545)

## 💡 Exemples de commandes (mode fichier)

```bash
# Ajouter un événement complet
ADD TITLE "Cours INF" DESCRIPTION "TD réseaux" LOCATION "Amphi A" DATE 20/11/2025 TIME 10:00 DURATION 90

# Modifier un événement
MODIFY TITLE "Cours INF" TIME 13:00

# Supprimer un événement
REMOVE TITLE "Sport"

# Lister tous les événements
LIST ALL

# Lister avec filtre de dates
LIST FROM 01/12/2025 TO 31/12/2025

# Rechercher
SEARCH "muscu"

# Vider le calendrier
CLEAR
```

## 📁 Structure du projet

```
projet_final/
├── analyse_lexical.c / .h        # Analyseur lexical
├── analyse_syntaxique.c / .h     # Analyseur syntaxique LL(1)
├── calendrier_ics.c / .h         # Moteur d'exécution ICS
├── main.c                        # Point d'entrée
├── Makefile                      
├── calendrier.ics                # Fichier ICS généré
├── calendrier_user.sh            # 🌟 Menu interactif (interface utilisateur)
├── cmd_temp.txt                  # Commande temporaire générée par le menu
├── tests_corrects/               # Jeux de tests valides
├── tests_incorrects/             # Jeux de tests invalides
├── screenshots/                  # Captures d'écran
└── README.md
```

## 🔧 Architecture technique

### Étapes de traitement :
1. **Analyse lexicale** : découpage du texte en tokens
2. **Analyse syntaxique LL(1)** : validation de la grammaire
3. **Construction d'un AST** : représentation arborescente de la commande
4. **Exécution** : application de la commande au fichier `calendrier.ics`

### Technologies :
- **Langage** : C (ANSI C)
- **Parsing** : Analyseur lexical + syntaxique LL(1)
- **Format** : iCalendar (.ics) — RFC 5545
- **Interface** : Bash script pour l'expérience utilisateur
- **Compilation** : Make

## 👤 Auteur - Belabbas Lydia

**Projet universitaire** — Licence 3 Informatique  
**Université Grenoble Alpes (UGA)**

Réalisé dans le cadre du cours d'analyse lexicale/syntaxique et conception d'interpréteur.

---

⭐ **Compatible** : Google Calendar | Outlook | Apple Calendar  
🔥 **User-friendly** : Menu interactif Bash pour une utilisation simplifiée  
✅ **Testé** : Jeux de tests corrects et incorrects inclus


