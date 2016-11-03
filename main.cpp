/*
 * Authors:     Maciej Gontar mg277344
 *              Tomasz Gąsior tg370797
 */
#include <iostream>
#include <regex>

/*
 * Krotka 'record' przechowuje:
 * 1. Rok wydania znaczka.
 * 2. Nazwę poczty.
 * 3. Część całkowitą ceny znaczka.
 * 4. Część dziesietną ceny znaczka.
 *    Przechowujemy ją jako string aby zachować wiodące zera.
 * 5. Opis znaczka.
 * 6. Znak separatora ceny. Przecinek lub kropka.
 */
typedef std::tuple<int, std::string, long, std::string, std::string, char> record;
typedef std::vector<record> Database;

/*
 * Przeciążenie operatora, wyświetla krotkę rekord w oczekiwanym formacie.
 */
std::ostream& operator<<(std::ostream& out, const record& record_) {
    out << std::get<0>(record_) << " " << std::get<1>(record_) << " "
        << std::get<2>(record_);
    if(std::get<5>(record_) != '\0') {
        out << std::get<5>(record_) << std::get<3>(record_);
    }
    out << " " << std::get<4>(record_);
    return out;
}

void printError(int lineNumber, const std::string& line) {
    std::cerr << "Error in line " << lineNumber << ":" << line << std::endl;
}

void printResult(record e) {
    std::cout << e << std::endl;
}

/*
 * Funkcja wyszukuje rekordy graniczne lb i ub
 * oraz wyświetla wszystkie rekordy pomiędzy nimi
 * wliczając lb, wyłączając ub
 */
void printRange(int yearFrom, int yearTo, Database& database) {
    //std::vector<record>::iterator lb, ub;
    auto lb = std::lower_bound(database.begin(), database.end(),
                               record(yearFrom, "", 0, "", "", '\0'));
    auto ub = std::upper_bound(database.begin(), database.end(),
                               record(yearTo+1, "", 0, "", "", '\0'));
    for_each(lb, ub, printResult);
}

/*
 * Funkcja tworzy rekord i dodaje go do kolekcji rekordów
 * Grupy 4 i 5 (znak delimitera i część dziesiętna liczby) są opcjonalne
 */
void addRecord(std::vector<record>& database, std::smatch& match) {
    std::string description = match[1].str();
    std::string name = match[7].str();
    int year = std::stoi(match[6].str());
    long price1 = std::stol(match[2].str());
    std::string price2 = match[5].matched ? match[5].str() : "";
    char delimiter = match[4].matched ? match[4].str()[0] : '\0';
    database.emplace_back(year, name, price1, price2, description, delimiter);
}

/*
 * Funkcja sprawdza czy napis odpowiada dwum 4-cyfrowym liczbom.
 * Jeśli nie odpowiada lub liczby są w kolejności malejącej to wypisuje
 * odpowiedni komunikat o błędzie.
 * W przeciwnym przypadku realizuje zapytanie.
 * Jeśli jest to pierwsze poprawne zapytanie, funckja dodatkowo ustawia flagę i
 * sortuje dane.
 */
void processQuery(Database& database, const std::regex queryPat,
                  const std::string& preprocessedLine, const std::string& line,
                  const int lineNumber, bool& reading) {
    std::smatch match;
    bool isQuery = std::regex_match(preprocessedLine, match, queryPat);
    if (isQuery && std::stoi(match[1].str()) <= std::stoi(match[2].str())) {
        if (reading) {
            reading = false;
            std::sort(database.begin(), database.end());
        }
        printRange(std::stoi(match[1].str()), std::stoi(match[2].str()), database);
    }
    else {
        printError(lineNumber, line);
    }
}

/*
 * Funkcja sprawdza czy napis odpowiada wejściowemu formatowi danych.
 * Jeśli tak to dodaje dane do kontenera.
 * W przeciwnym przypadku wypisuje komunikat o błędzie lub realizuje zapytanie.
 */
void processData(Database& database, const std::regex dataPat, const std::regex queryPat,
                 const std::string& preprocessedLine, const std::string& line,
                 const int lineNumber, bool& reading) {
    std::smatch match;
    bool valid = std::regex_match(preprocessedLine, match, dataPat);
    bool isPriceValid = valid && ((match[2].str().length() != 1 && match[2].str()[0] != '0') ||
            (match[2].str().length() == 1 && match[2].str()[0] == '0' && match[4].matched) ||
            (match[2].str().length() == 1 && match[2].str()[0] != '0'));
    if (valid && isPriceValid) {
        addRecord(database, match);
    }
    else {
        processQuery(database, queryPat, preprocessedLine,  line, lineNumber, reading);
    }
}

int main() {
    std::vector<record> database;
    //Nazwa znaczka, składa się z dowolnych znaków i spacji, kończy się znakiem
    const std::string descrPat {"([[:print:]\\s]+\\b)"};
    //Wartość znaczka, składa się z dowolnych cyfr niezaczynających się zerem
    const std::string pricePat1 {"([\\d]+)"};
    //Opcjonalnie: wartość znaczka części dziesiętne, wraz z przecinkiem lub kropką
    const std::string pricePat2 {"(([.|,])([\\d]{1,4}))?"};
    //Rok wydania, lata dopuszczane 1000-2999
    const std::string yearPat {"([1,2][\\d]{3})"};
    //Nazwa poczty, składa się z dowolnych znaków nie będących cyframi
    const std::string namePat {"([[:print:]\\s^[:digit:]]+\\b)"};
    const std::regex dataPat {"\\s?" + descrPat + "\\s" + pricePat1 +
                              pricePat2 + "\\s" + yearPat + "\\s" + namePat + "\\s?"};
    const std::regex queryPat {"\\s?" + yearPat + "\\s" + yearPat + "\\s?"};
    const std::regex replace_pattern {"\\s+"};
    int lineNum = 0;
    bool reading = true;
    std::string line;
    while (getline(std::cin, line)) {
        ++lineNum;
        std::string procLine = std::regex_replace(line, replace_pattern, " ");
        if (reading) {
            processData(database, dataPat, queryPat, procLine, line, lineNum, reading);
        }
        else {
            processQuery(database, queryPat, procLine,  line, lineNum, reading);
        }
    }
    return 0;
}