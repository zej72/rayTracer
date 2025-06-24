#!/bin/bash

# Nazwa katalogu budowania
BUILD_DIR="build"

# Katalog główny projektu (tam, gdzie jest CMakeLists.txt i pliki źródłowe)
PROJECT_ROOT=$(dirname "$0")

# Przejdź do katalogu głównego projektu
cd "$PROJECT_ROOT" || exit

echo "--- Konfiguracja CMake ---"
# Utwórz katalog budowania, jeśli nie istnieje i przejdź do niego
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit

# Skonfiguruj projekt za pomocą CMake
# .. oznacza, że CMakeLists.txt jest w katalogu nadrzędnym
cmake ..

# Sprawdź, czy konfiguracja CMake przebiegła pomyślnie
if [ $? -ne 0 ]; then
    echo "Błąd: Konfiguracja CMake nie powiodła się."
    exit 1
fi

echo "--- Kompilacja programu ---"
# Skompiluj projekt
make

# Sprawdź, czy kompilacja przebiegła pomyślnie
if [ $? -ne 0 ]; then
    echo "Błąd: Kompilacja programu nie powiodła się."
    exit 1
fi

echo "--- Uruchamianie programu ---"
# Uruchom skompilowany program
# Nazwa programu to 'rayTracer', tak jak zdefiniowano w add_executable w CMakeLists.txt
./rayTracer

# Sprawdź, czy uruchomienie programu przebiegło pomyślnie
if [ $? -ne 0 ]; then
    echo "Błąd: Program zwrócił błąd podczas uruchomienia."
    exit 1
fi

echo "--- Zakończono ---"
