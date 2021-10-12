#!/bin/bash

#znaczne zmiany zaszły 23.08.2021
#argumenty to:
#   program (znajdujący się w aktualnym folderze)
#   folder z testami (ścieżka względna lub bezwzględna)
#   (opcjonalnie) napis "v" -- spowoduje sprawdzanie pod valgrindem (tak naprawdę liczy się pierwsza litera napisu)

if [ $# -lt 2 ]
then
    echo "za mało argumentów"
    exit 1
fi

valgrind=""
if [[ $# -ge 3 ]] && [[ ${3:0:1} == v || ${3:0:1} == V ]]
then
    valgrind="valgrind -q --error-exitcode=123 --leak-check=full --errors-for-leak-kinds=possible,definite "
fi
    # takie wywołanie valgrinda zapewnia, że jeśli program zrobi exit,
    # i to spowoduje "wyciek reachable", to kod wyjścia pozostanie programu

    # zatem:
    # kod 123 będzie oznaczał wyciek pamięci lub inny błąd wykryty przez valgrind
    # uwaga: ponieważ przy dereferencji nulla terminuje system, wartość kodu na wyjściu
    # będzie inna - czyli ten błąd nie będzie się liczył jako wykryty przez valgrinda

    # kod 1 - testowany program sam się zakończył tym kodem
    #(dla similar_lines: stwierdził problemy z pamięcią)

    # kod 0 - ok
    # inne - nieprzewidziany błąd (np. dereferencja nulla)

for f in $2/*.in
do
	out=$(mktemp tmpXXX)
	err=$(mktemp tmpXXX)
	$valgrind./$1 <$f 1>$out 2>$err	# jeśli exitcode == 0 lub 1, to err powinien mieć znaczenie
									# jeśli exitcode == 123 - err raczej nie ma znaczenia, bo valgrind tam nakrzyczy
									# (przy -q krzyczy dokładnie wtedy, gdy zwraca error-exitcode - chyba)
	exitcode=$?

	diff "${f%in}out" "$out" &>/dev/null 2>&1
	outdiff=$?	# outdiff równe 1 oznacza różnicę wyniku z plikiem .out, równe 0 - wynik .out jest dobry
				# outdiff większe niż jeden oznacza błąd porównywania

    echo ""

	if [[ $outdiff > 1 ]]
	then
		#prawdopodobnie {f%in}out nie istnieje, szukamy tego pliku z końcówką 0, 1,..., 9
        for i in $(eval "ls ${f%in}out* 2>/dev/null")    #zamiast * może być [0-9] lub [0-9]* -- tak jest ok
        do                                               #lub nawet +(1|2|3|4|5|6|7|8|9|0)
            if (( $outdiff >= 1 ))
            then
                diff "$i" "$out" &>/dev/null 2>&1
                outdiff2=$?
                if [[ $outdiff2 == 0 ]]
                then
                    echo "poprawny out dla $i"
                    outdiff=$outdiff2
                elif [[ $outdiff2 == 1 ]]
                then
                    outdiff=$outdiff2
                fi

            fi
        done
	fi

	diff "${f%in}err" "$err" &>/dev/null 2>&1
	errdiff=$?  # wprowadzamy errdiff tak jak outdiff

	if [[ $errdiff > 1 ]]
	then
		#prawdopodobnie ${f%in}err nie istnieje, szukamy tego pliku z końcówką 0, 1,..., 9
        for i in $(eval "ls ${f%in}err* 2>/dev/null")   #zamiast * może być [0-9] lub [0-9]* -- tak jest ok
        do
            if (( $errdiff >= 1 ))
            then
                diff "$i" "$err" &>/dev/null 2>&1
                errdiff2=$?
                if [[ $errdiff2 == 0 ]]
                then
                    echo "poprawny err dla $i"
                    errdiff=$errdiff2
                elif [[ $errdiff2 == 1 ]]
                then
                    errdiff=$errdiff2
                fi
            fi
        done
	fi
    zmienna=0

	p="PLIK: $f"
    odp=""
    x=""
    czout=""
    czerr=""
	if [[ $exitcode == 0 ]]
	then
	    if [[ $outdiff == 0 && $errdiff == 0 ]]
	    then
	        odp="\nPOWODZENIE"
	    else
	        #czout
    		if [[ $outdiff == 0 ]]
            then
                czout="\nout: OK"
            elif [[ $outdiff == 1 ]]
            then
                czout="\nout: BŁĄD"
            else
                czout="\nout: BRAK PLIKU do porównania"
            fi

            #czerr
        	if [[ $errdiff == 0 ]]
            then
                czerr="\nerr: OK"
            elif [[ $outdiff == 1 ]]
            then
                czerr="\nerr: BŁĄD"
            else
                czerr="\nerr: BRAK PLIKU do porównania"
            fi
       fi

	elif [[ $exitcode == 1 ]]	#dotąd wykluczyliśmy $exitcode == 0
	then
		odp="$\nprogramowi zabrakło pamięci"

	else	#$exitcode != 0, 1

		if [[ $exitcode == 123 ]]
		then
			odp="\n$valgrind wykrył wyciek pamięci lub inny błąd wskazujący na nieokreślone działanie programu"     # jest szansa, że chodzi o odwołanie do niezainicjalizowanej wartości
			                                                                                                        # ale nie może chodzić o dereferencję nulla (patrz kom. na poczatku)
                                                                                                                    # możnaby dodać jakąś funkcjonalność, żeby to oblukać,
                                                                                                                    # bo w $err jest info o jaki błąd chodzi (niestety jest to plik tymczasowy)
		else	#$exitcode != 0, 1, 123
			odp="$\ndoszło do nieprzewidzianego błędu"  #i tu możnaby dodać tę samą funkcjonalność (może puścić 2> do konsoli? - to nieautomatyczne) # chyba rozwiązaniem jest nieusuwanie tmp
		fi

        czerr="\nerr: trudno powiedzieć"

		if [[ $outdiff == 0 ]]
		then
			czout="\nout: OK"
		elif [[ $outdiff == 1 ]]
		then
			czout="\nout: BŁĄD"
        else
            czout="\nout: BRAK PLIKU do porównania"
		fi

	fi

	echo -e "$p$odp$czout$czerr"

	rm "$out"
	rm "$err"

done
