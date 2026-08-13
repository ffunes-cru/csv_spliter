#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <utility>

using namespace std;
namespace fs = std::filesystem;

class CsvClass {       // The class
    private:
        vector<string> lines;
        void readFile(fs::path filepath){
            ifstream Csv_File(filepath);
            lines.reserve(500);

            string linea;
            while (getline (Csv_File, linea)) {
                lines.push_back(std::move(linea));
            }
            Csv_File.close();
        }

    public:

        CsvClass(fs::path filepath) {
            readFile(filepath);
        }

        CsvClass(vector<string> lines) {
            this->lines = lines;
        }

        vector<CsvClass> splitCsv(int split_len) {
            vector<CsvClass> output;
            int last_offset = 0;
            for (int i = 0 ; i < lines.size() / split_len ; i++) {
                last_offset = i * split_len;
                auto it = lines.begin() + last_offset;
                vector<string> split_lo(it, it + split_len);
                CsvClass temp(split_lo);
                output.push_back(std::move(temp));
            }
            vector<string> split_hi(lines.begin() + last_offset, lines.end());
            CsvClass temp(split_hi);
            output.push_back(std::move(temp));
            return output;
        }

        void printText() {
            for (string line : lines) {
              cout << line << "\n";
            }
        }

        void saveText(string name) {
            ofstream outcsv(name);
            outcsv << lines;
        }
};

int main() {
    CsvClass csv("GRINGA.csv");
    vector<CsvClass> split_csv = std::move(csv.splitCsv(125));
    split_csv[1].printText();
    // for (CsvClass ccsv : split_csv) {
    //     ccsv.printText();
    // }
}
