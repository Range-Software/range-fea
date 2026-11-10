# Multifizyka

Ten samouczek pokazuje, jak rozwiązać kilka sprzężonych problemów na jednym modelu. Ponownie używany jest model **Beam** (belka) z samouczka **Analiza naprężeń i odkształceń**. Tym razem przez belkę płynie prąd elektryczny, który ją nagrzewa (ciepło Joule'a), a ciepło powoduje, że belka się rozszerza i odkształca.

Potrzebne są trzy typy problemów, rozwiązywane w tej kolejności:

1. **Electro-statics** (elektrostatyka) – oblicza potencjał elektryczny, pole elektryczne, gęstość prądu i ciepło Joule'a wytwarzane przez prąd.
2. **Heat transfer** (wymiana ciepła) – oblicza rozkład temperatury wywołany ciepłem Joule'a.
3. **Stress analysis** (analiza naprężeń) – oblicza rozszerzalność cieplną (przemieszczenia) i wynikające z niej naprężenia.

Każdy problem korzysta z wyników poprzedniego, dlatego kolejność problemów ma znaczenie.

## 1. Wczytaj model

**Menu:** _Plik -> Otwórz model_

W oknie dialogowym **Otwórz model** wybierz **Beam.rbm** i kliknij **Open** (Otwórz).

Ponieważ do modelu nie przypisano jeszcze typu problemu, po zakończeniu wczytywania automatycznie otworzy się okno dialogowe **Wybierz problem**.

## 2. Wybierz problemy

W oknie dialogowym **Wybierz problem** kliknij kafelki **Electro-statics**, **Heat transfer** i **Stress analysis**. Pod kafelkami okno wyświetla **Kolejność rozwiązywania**: Electro-statics → Heat transfer → Stress analysis. **Modal analysis** jest oznaczony jako **Niezgodny**, ponieważ nie można go łączyć z innymi typami problemów. Zatwierdź przyciskiem **OK**.

![Wybierz problem – multifizyka](image-Select_problem-Multi.png)

Okno można w każdej chwili otworzyć ponownie przyciskiem **Wybierz problem** na pasku narzędzi.

## 3. Wygeneruj siatkę 3D

Wszystkie trzy problemy są rozwiązywane na siatce objętościowej.

**Menu:** _Geometria -> Objętość -> Generuj siatkę czworościenną_

Wartości domyślne w oknie **Generuj siatkę 3D** wystarczą. Kliknij **OK**. Po zakończeniu generowania w **drzewie modelu** w grupie **Objętości** pojawi się nowa encja **Volume 1**.

## 4. Przypisz materiał

Wszystkie encje modelu muszą mieć przypisany materiał. Materiał musi zawierać właściwości potrzebne dla wszystkich trzech problemów; **Iron** (żelazo) je zawiera.

1. W **drzewie modelu** rozwiń **Objętości**, kliknij **Surface**, a następnie z wciśniętym klawiszem _Shift_ kliknij **Volume 1**, aby zaznaczyć wszystkie encje.
2. Otwórz zakładkę **Materiał** po prawej stronie okna głównego.
3. Zaznacz **Iron**.

## 5. Przypisz warunki brzegowe

Ponieważ wybrano kilka typów problemów, zakładka **Warunki brzegowe** oferuje warunki wszystkich tych problemów, a jedna encja może mieć przypisanych kilka warunków – po jednym dla każdego problemu. Przypisz encjom **powierzchniowym** następujące warunki:

1. **Surface** (zewnętrzna powierzchnia belki)
    - _Simple convection_ (konwekcja prosta)
        - Convection coefficient = 100 `[W/(m^2*K)]`
        - Fluid temperature = 293,15 `[K]`
2. **Load** (powierzchnia na środku belki)
    - _Electric potential_ (potencjał elektryczny)
        - Electric potential = 0,04 `[V]`
    - _Simple convection_ (wartości jak dla **Surface**)
3. **Left** i **Right** (powierzchnie na obu końcach belki)
    - _Displacement_ (przemieszczenie)
        - Displacement in X, Y and Z direction = 0 `[m]`
    - _Electric potential_
        - Electric potential = -0,04 `[V]`
    - _Simple convection_ (wartości jak dla **Surface**)

Prąd płynie więc ze środka belki do obu końców, a wszystkie powierzchnie oddają ciepło do otaczającego powietrza przez konwekcję.

Dla każdej encji: zaznacz ją w **drzewie modelu** (przytrzymaj _Ctrl_ / _⌘ Cmd_, aby zaznaczyć **Left** i **Right**), zaznacz pole wyboru warunku na zakładce **Warunki brzegowe**, wybierz warunek, aby wyświetlić jego właściwości pod listą, a następnie kliknij dwukrotnie wartość, wpisz nową i naciśnij _Enter_. Wartości domyślne **Simple convection** (100 `[W/(m^2*K)]` i 293,15 `[K]`) oraz **Displacement** (0 `[m]`) są już prawidłowe, więc trzeba zmienić tylko **Electric potential**.

## 6. Sprawdź warunki otoczenia

Warunki otoczenia dotyczą całego modelu i nie zaznacza się ich jak warunków brzegowych. Otwórz zakładkę **Warunki otoczenia** i wybierz każdy warunek, aby sprawdzić jego wartości:

- _Gravitational acceleration_ (przyspieszenie grawitacyjne)
    - 0, 0 i -9,80665 `[m/s^2]` w kierunkach X, Y i Z
- _Temperature_ (temperatura)
    - Temperature = 293,15 `[K]`

## 7. Rozwiąż problem

**Menu:** _Rozwiązanie -> Uruchom solver_

Przed uruchomieniem solvera model musi zostać zapisany. Model **Beam** został otwarty z pliku, więc jest w nim zapisywany automatycznie; model, który nigdy nie był zapisany, najpierw wyświetla okno **Zapisz model**. Następnie wyświetlane jest okno **Uruchom solver**.

Okno wyświetla trzy ostrzeżenia: **Electro-statics: No implicit boundary condition assigned.**, **Heat transfer: No explicit boundary condition assigned.** i **Stress analysis: No implicit boundary condition assigned.** Dla tej konfiguracji są one oczekiwane i można je zignorować. Kliknij **OK**, aby uruchomić solver.

Solver działa w tle, a jego wyjście jest wyświetlane na zakładce **Wyjście procesu**. Problemy są rozwiązywane jeden po drugim w kolejności zadań; obliczenia są zakończone, gdy wyjście kończy się wierszem **SOLVER - END**.

## 8. Utwórz pole wektorowe

**Pole wektorowe** to encja wyświetlania, która rysuje zmienną wektorową w postaci strzałek. Tutaj służy do pokazania pola elektrycznego wewnątrz belki.

**Menu:** _Geometria -> Pole wektorowe -> Utwórz pole wektorowe_

W oknie **Edytor pól wektorowych** wybierz **Volume 1** na liście **Model** oraz **Electric field** na liście **Zmienne** i pozostaw zaznaczone **Pole wektorowe 3D**. Lista **Zmienne** zawiera zmienne wektorowe obliczone dla wybranej encji (**Electric field**, **Current density**, **Heat flux**, **Displacement** i **Force**).

![Utwórz pole wektorowe](image-Create_vector_field.png)

Kliknij **OK**, aby utworzyć pole wektorowe. Zostanie ono dodane do **drzewa modelu** w grupie **Pola wektorowe** pod nazwą **Electric field**, a obliczone wyniki można na nim wyświetlać jak na każdej innej encji modelu.

## 9. Wyświetl wyniki

Wyniki są wyświetlane na zakładce **Wyniki** na encji zaznaczonej w **drzewie modelu**, tak samo jak w samouczku **Analiza naprężeń i odkształceń**.

### 9.1. Temperatura i odkształcenie objętości

1. W **drzewie modelu** zaznacz **Objętości** (powierzchnie pozostaw niezaznaczone), a następnie zaznacz **Volume 1**.
2. Na zakładce **Wyniki** wybierz z menu rozwijanego **Temperature - Scalar**, zaznacz **Mapa kolorów** i **Pokaż legendę**. Jeśli wyświetlany zakres nie odpowiada wynikom, kliknij **Przywróć według danych**.
3. Wybierz z menu rozwijanego **Displacement - Vector [3]**, zaznacz **Kształt odkształcony** i ustaw **Skalę odkształcenia i strzałek** na **20**, aby odkształcenie było widoczne (obliczone przemieszczenia wynoszą tylko około 1,2 mm).
4. Ponownie wybierz **Temperature - Scalar**, aby legenda pokazywała temperaturę.

Temperatura wynosi od około 468 K do 558 K; belka jest najcieplejsza w środku obu zewnętrznych części i wygina się do góry, ponieważ unieruchomione końce nie pozwalają jej swobodnie się wydłużyć.

![Temperatura na objętości](image-Applied_temperature_on_volume.png)

### 9.2. Temperatura na polu wektorowym

1. W **drzewie modelu** odznacz **Objętości**, zaznacz **Pola wektorowe** i zaznacz **Electric field**.
2. Na zakładce **Wyniki** wybierz **Temperature - Scalar**, zaznacz **Mapa kolorów** i **Pokaż legendę** (jeśli nie są już zaznaczone). Strzałki są teraz pokolorowane według temperatury w miejscu, w którym się znajdują.
3. We **Właściwościach wyświetlania** pod drzewem zaznacz **Groty strzałek**, aby pokazać kierunek pola elektrycznego. Pole elektryczne jest skierowane od środka belki (wyższy potencjał) do obu końców.

![Temperatura na polu wektorowym](image-Applied_temperature_on_vector_field.png)
