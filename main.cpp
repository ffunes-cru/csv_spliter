#include <complex>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <utility>
#include <stdlib.h>
#include <stdexcept>
//#include <argp.h>

#if defined(_WIN32) // any windows system
    #include <windows.h>
    #include <shobjidl.h>
#endif

using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
fs::path PickDialog(
    FILEOPENDIALOGOPTIONS pickType,
    const wchar_t* title = nullptr,
    const std::vector<COMDLG_FILTERSPEC>& rgSpec = {}
) {
    fs::path selectedPath = "";

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cout << "Error al inicializar COM" << std::endl;
        return "";
    }

    IFileOpenDialog *pfd = NULL;

    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

    if (SUCCEEDED(hr) && pfd != NULL) {

        DWORD dwFlags;
        if (SUCCEEDED(pfd->GetOptions(&dwFlags))) {
            pfd->SetOptions(dwFlags | pickType);
        }

        if (title != nullptr) {
            pfd->SetTitle(title);
        }

        if (pickType != FOS_PICKFOLDERS){
            pfd->SetFileTypes(rgSpec.size(), rgSpec.data());
        }

        hr = pfd->Show(NULL);

        if (SUCCEEDED(hr)) {
            IShellItem *psiResult = NULL;
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr) && psiResult != NULL) {
                PWSTR pszFilePath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr) && pszFilePath != NULL) {
                    selectedPath = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                psiResult->Release();
            }
        }
        pfd->Release();
    } else {
        std::cout << "Error al crear CoCreateInstance: " << std::hex << hr << std::endl;
    }

    CoUninitialize();
    std::cout << selectedPath;
    return selectedPath;
}
#endif

class CsvClass {       // The class
    private:
        vector<string> lines;
        std::error_code ec;
        fs::path csv_filepath;
        fs::path output_dir;

        void readFile(){
            if (csv_filepath.empty() || !fs::exists(csv_filepath, ec)) {
                cout << "No existe el archivo o no fue seleccionado";
                return;
            };

            ifstream Csv_File(csv_filepath);
            lines.reserve(500);

            string linea;
            while (getline (Csv_File, linea)) {
                lines.push_back(std::move(linea));
            }
            Csv_File.close();
        }

        void printHelp(std::string_view program_name = "csv_spliter") {
            std::cout
                << "Uso: " << program_name << " [ARCHIVO_CSV] [DIRECTORIO_SALIDA]\n\n"
                << "CSV Spliter - Herramienta para dividir archivos CSV grandes.\n\n"
                << "Ejemplos de uso:\n"
                << "  " << program_name << " datos.csv <Raiz>\n"
                << "  " << program_name << " -i datos.csv -o ./salida -v\n"
                << "  " << program_name << " <De esta manera se abre el WINDOWS GUI>\n\n"
                << "Use -h para mostrar esta ayuda";
        }

        void parseArgs(std::vector<std::string_view> args){
            if (args[0] == "-h") {
                printHelp();
                return;
            }

            fs::path arg1 = args[0];
            fs::path arg2 = args[1];

            if (fs::is_regular_file(arg1, ec)) {
                if (arg1.extension() != ".csv") {cout << "Debe ingresar un archivo .csv"; return; }
                csv_filepath = std::move(arg1);
            } else {
                cout << "Debe ingresar una archivo o ruta valida\n\n";
                printHelp();
                return;
            }
            if (fs::is_directory(arg2, ec)) {
                output_dir = std::move(arg2);
            } else {
                cout << "Debe ingresar un directorio valido\n\n";
                printHelp();
                return;
            }
        }

        #ifdef _WIN32
        vector<COMDLG_FILTERSPEC> rgSpec = {
            { L"Archivos CSV (*.csv)", L"*.csv" },
            { L"Todos los archivos (*.*)", L"*.*" }
        };
        #endif

    public:

        CsvClass(vector<string> lines) {
            this->lines = lines;
        }

        CsvClass(int argc, char* argv[]) {

            #ifdef _WIN32
            csv_filepath = PickDialog(FOS_FORCEFILESYSTEM, L"Seleccione un csv", rgSpec);
            output_dir   = PickDialog(FOS_PICKFOLDERS ,L"Seleccione la carpeta destino");
            readFile();
            #endif

            if (argc > 1 && argc <= 3) {
                std::vector<std::string_view> args(argv + 1, argv + argc);
                parseArgs(args);
                readFile();
            } else {
                printHelp();
            }
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

        void saveText() {
            if (fs::exists(output_dir)) { cout << "No existe la carpeta o no fue seleccionado"; return; };
            ofstream outcsv(output_dir, std::ios::out | std::ios::binary);
            for (const auto& line : lines) {
                    outcsv << line << '\n';
            }
        }
};

int main(int argc, char* argv[]) {

    CsvClass main_instance(argc, argv);
    vector<CsvClass> split_csv = std::move(main_instance.splitCsv(125));
    split_csv[1].printText();

}
