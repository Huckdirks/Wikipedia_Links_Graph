// Files to include
#include "include/menu.hpp"

// Initialize menu
int menu::init_menu() {
    bool flag{};
    system("clear");

    // For some Goddamn reason when I try to call the python script it runs on a separate thread which completely breaks literally everything (It doesn't even let me properly close out of my terminal), so I gotta go back to the shitty way of calling it ¯\_(ツ)_/¯
    // Get current directory
    const fs::path PYTHON_DIR{fs::current_path()/"source"/"python/main.py"};
    const std::string RUN_PYTHON{"python3 " + PYTHON_DIR.string()};
    system(RUN_PYTHON.c_str());

    // If I figure out a function that possibly runs all the files instead of PyRun_SimpleFile() that would be good, because I think execution is suspended for the running of the first file (main.py), but as soon as it runs a function from a separate file it resumes execution, which breaks everything

    /* // Initialize python script
    const std::string PYTHON_PATH{fs::current_path().string() + "/source/python/"};
    const std::string MAIN_PY_PATH{PYTHON_PATH + "main.py"};
    FILE *python_file{};

    Py_Initialize();

    // Add any other files besides main.py in Wikipedia_Graph/source/python/ to the python path
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString(PYTHON_PATH.c_str()));

    try {
        python_file = fopen(MAIN_PY_PATH.c_str(), "r");
        // Run the main python script & finalize python
        PyRun_SimpleFile(python_file, MAIN_PY_PATH.c_str());
        //Py_Finalize();
    } catch (const std::exception &E) {
        std::cerr << "\nError loading python script\n";
        return EXIT_FAILURE;
    }
    //PyRun_SimpleFile(python_file, MAIN_PY_PATH.c_str());
    // Wait for the python script to finish
    PyRun_SimpleString("import time\n"
                       "from threading import enumerate\n"
                       "while True:\n"
                       "    if not any(t for t in enumerate() if t.daemon):\n"
                       "        break\n"
                       "    time.sleep(0.1)");

    Py_Finalize();
    fclose(python_file); */

    if (load()){
        std::cout << "\nError loading graph\n";
        return EXIT_FAILURE;
    }
    // Keep the menu running as long as the user doesn't quit
    do flag = interface();
    while (!flag);
    return EXIT_SUCCESS;
}


// Menu interface
int menu::interface() {
    std::cout << "\n\n\nWelcome to the Wiki Graph\n\nPress 1 to find the info on a single page\n2 to view info on the whole wiki\n3 to find the most linked to pages\n4 to find all the pages linking to a page\n0 or Q to quit\n";
    const int CHOICE{selector()};

    switch (CHOICE) {
        case 0:
            system("clear");
            std::cout << "\nGoodbye!!!\n";
            return EXIT_FAILURE;

        case 1:
            return display_page();

        case 2:
            return display_wiki_info();

        case 3:
            return display_top_n();

        case 4:
            return display_linked_to();

        default:
            system("clear");
            std::cout << "\nInvalid input\n";
            break;
    }
    return EXIT_SUCCESS;
}


// Menu selector
int menu::selector() {
    std::string num;
    unsigned int n{};
    bool flag{};

    do {
        flag = true;
        std::cin >> num;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (num.length() == 1 && (toupper(num[0]) == 'Q' || num[0] == '0'))
            return 0;

        for (const char &c : num) {
            if (!isdigit(c)) {
                std::cout << "\nPlease enter a number\nTry again\n";
                flag = false;
                break;
            }
        }

        if (flag) {
            n = std::stoi(num);
            if (n > 4) {
                std::cout << "\nPlease enter a number between 0 and 4\nTry again\n";
                flag = false;
            }
        }
    } while (!flag);

    return n;
}