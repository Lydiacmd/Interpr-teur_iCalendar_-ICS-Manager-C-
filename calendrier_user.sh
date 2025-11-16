#!/bin/bash

# Petit "frontend" en bash pour l'interpréteur ./calendrier

CMD_FILE="cmd_temp.txt"

run_cmd() {
    local cmd="$1"
    echo "$cmd" > "$CMD_FILE"
    echo
    echo "→ Commande envoyée :"
    echo "$cmd"
    echo
    ./calendrier "$CMD_FILE"
}

while true; do
    echo
    echo "============================="
    echo "  MENU CALENDRIER iCalendar"
    echo "============================="
    echo "1) Ajouter un ou plusieurs événements (ADD)"
    echo "2) Modifier un événement (MODIFY)"
    echo "3) Supprimer un événement (REMOVE)"
    echo "4) Lister les événements (LIST)"
    echo "5) Rechercher des événements (SEARCH)"
    echo "6) Vider le calendrier (CLEAR)"
    echo "0) Quitter"
    echo "============================="
    read -p "Ton choix : " choice

    case "$choice" in
        1)
            # ADD multiple
            read -p "Combien d'événements veux-tu ajouter ? " n
            if ! [[ "$n" =~ ^[0-9]+$ ]] || [ "$n" -le 0 ]; then
                echo "Nombre invalide."
                continue
            fi

            for (( i=1; i<=n; i++ )); do
                echo
                echo "=== Événement $i / $n ==="
                read -r -p "Titre : " title
                read -r -p "Description : " desc
                read -r -p "Lieu : " location
                read -r -p "Date (JJ/MM/AAAA) : " date
                read -r -p "Heure (HH:MM) : " time
                read -r -p "Durée (minutes) : " duration

                cmd="ADD TITLE \"$title\" DESCRIPTION \"$desc\" LOCATION \"$location\" DATE $date TIME $time DURATION $duration"
                run_cmd "$cmd"
            done
            ;;

        2)
            # MODIFY
            echo
            echo "=== Modifier un événement ==="
            read -r -p "Titre de l'événement à modifier (obligatoire) : " old_title
            read -r -p "Date actuelle de l'événement (JJ/MM/AAAA, obligatoire) : " date

            if [ -z "$old_title" ] || [ -z "$date" ]; then
                echo "Titre et date sont obligatoires pour MODIFY."
                continue
            fi

            # Champs optionnels
            read -r -p "Nouveau lieu (laisser vide si inchangé) : " new_loc
            read -r -p "Nouvelle description (laisser vide si inchangé) : " new_desc
            read -r -p "Nouvelle heure (HH:MM, laisser vide si inchangé) : " new_time
            read -r -p "Nouvelle durée (minutes, laisser vide si inchangé) : " new_dur

            cmd="MODIFY TITLE \"$old_title\" DATE $date"

            [ -n "$new_loc" ]   && cmd="$cmd LOCATION \"$new_loc\""
            [ -n "$new_desc" ]  && cmd="$cmd DESCRIPTION \"$new_desc\""
            [ -n "$new_time" ]  && cmd="$cmd TIME $new_time"
            if [ -n "$new_dur" ]; then
                cmd="$cmd DURATION $new_dur"
            fi

            run_cmd "$cmd"
            ;;

        3)
            # REMOVE
            echo
            echo "=== Supprimer un événement ==="
            read -r -p "Titre de l'événement à supprimer (obligatoire) : " title
            read -r -p "Date de l'événement (JJ/MM/AAAA, obligatoire) : " date

            if [ -z "$title" ] || [ -z "$date" ]; then
                echo "Titre et date sont obligatoires pour REMOVE."
                continue
            fi

            cmd="REMOVE TITLE \"$title\" DATE $date"
            run_cmd "$cmd"
            ;;

        4)
            # LIST
            echo
            echo "=== Lister les événements ==="
            echo "1) Tous (ALL)"
            echo "2) Un jour précis (ON DATE)"
            echo "3) À partir d'une date (FROM DATE)"
            echo "4) Entre deux dates (FROM ... TO ...)"
            read -p "Choix : " lchoice

            case "$lchoice" in
                1)
                    cmd="LIST ALL"
                    ;;
                2)
                    read -r -p "Date (JJ/MM/AAAA) : " d_on
                    cmd="LIST ON DATE $d_on"
                    ;;
                3)
                    read -r -p "Date de début (JJ/MM/AAAA) : " d_from
                    cmd="LIST FROM DATE $d_from"
                    ;;
                4)
                    read -r -p "Date de début (JJ/MM/AAAA) : " d_from
                    read -r -p "Date de fin (JJ/MM/AAAA) : " d_to
                    cmd="LIST FROM DATE $d_from TO DATE $d_to"
                    ;;
                *)
                    echo "Choix invalide."
                    continue
                    ;;
            esac

            run_cmd "$cmd"
            ;;

        5)
            # SEARCH
            echo
            echo "=== Rechercher des événements ==="
            read -r -p "Terme de recherche (titre / description / lieu) : " term
            if [ -z "$term" ]; then
                echo "Terme vide, recherche annulée."
                continue
            fi
            cmd="SEARCH \"$term\""
            run_cmd "$cmd"
            ;;

        6)
            # CLEAR
            echo
            read -p "⚠️  Tu es sûre de vouloir VIDER tout le calendrier ? (oui/non) : " rep
            if [ "$rep" = "oui" ] || [ "$rep" = "o" ]; then
                cmd="CLEAR"
                run_cmd "$cmd"
            else
                echo "CLEAR annulé."
            fi
            ;;

        0)
            echo "Bye 👋"
            exit 0
            ;;

        *)
            echo "Choix invalide."
            ;;
    esac

done
