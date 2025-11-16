# Interpréteur iCalendar

Interpréteur de fichiers **.ics** en C avec gestion d'événements (`ADD` / `MODIFY` / `REMOVE` / `LIST` / `SEARCH` / `CLEAR`) et **menu interactif Bash** pour faciliter l’utilisation.

---

## 🚀 Commandes disponibles

| Commande  | Description                      | Exemple                                                              |
|----------|----------------------------------|----------------------------------------------------------------------|
| `ADD`    | Ajouter un événement             | `ADD TITLE "Cours INF" DESCRIPTION "TD réseaux" LOCATION "Amphi A" DATE 20/11/2025 TIME 10:00 DURATION 90` |
| `MODIFY` | Modifier un événement existant   | `MODIFY TITLE "Cours INF" DATE 20/11/2025 TIME 13:00`                |
| `REMOVE` | Supprimer un événement           | `REMOVE TITLE "Sport" DATE 20/11/2025`                               |
| `LIST`   | Lister les événements            | `LIST ALL` ou `LIST FROM DATE 01/12/2025 TO DATE 31/12/2025`         |
| `SEARCH` | Rechercher par mot-clé           | `SEARCH "muscu"`                                                     |
| `CLEAR`  | Vider complètement le calendrier | `CLEAR`                                                              |

> 🔎 Remarque : pour `MODIFY` et `REMOVE`, **le titre et la date** sont obligatoires pour identifier l’événement.

---

## ⚙️ Installation et compilation

Assure-toi d’avoir `gcc` et `make` installés, puis :

```bash
make clean
make
