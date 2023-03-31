// Files to include
#include "wiki_loader.hpp"

// MAKE LOAD ACCEPT A PATH TO THE DIRECTORY OF FILES TO LOAD SO IT ISN'T HARD CODED

// Load all articles into the graph
int wiki_loader::load() {
    // system("clear");
    auto start{std::chrono::system_clock::now()};

    // Change directory to Articles-p
    const fs::path main_dir{fs::current_path()};
    fs::current_path(main_dir.parent_path().parent_path() / "data/load/");
    if (!fs::exists("Articles-p")) {
        std::cout << "\n\nNo directory to load from found\n\n";
        return 0;
    }
    fs::current_path("Articles-p");

    // Get all file names ending in .ndjson in directory
    std::vector<std::string> file_names;
    for (const auto &file : fs::directory_iterator(main_dir.parent_path().parent_path() / "data/load/Articles-p")) {
        if (file.path().extension() == ".ndjson" && fs::file_size(file) != 0)
            file_names.push_back(file.path().filename());
    }

    if (file_names.empty()) {
        std::cout << "\n\nNo files found in directory\n\n";
        return 0;
    }

    std::set<std::string> titles;
    std::cout << "\nLoading " << file_names.size() << " files...\n";

    // Progress bar
    indicators::BlockProgressBar title_bar{indicators::option::BarWidth{80}, indicators::option::Start{"["}, indicators::option::End{"]"}, indicators::option::ShowElapsedTime{true}, indicators::option::ShowRemainingTime{true}, indicators::option::ForegroundColor{indicators::Color::red}, indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
    int progress{};
    // Hide cursor
    indicators::show_console_cursor(false);
    auto set_load_start{std::chrono::system_clock::now()};

    // Load in each file's titles to a set to make it ordered
    for (const auto &file : file_names) {
        const double percent{100 * ((double)progress / (file_names.size() - 1))};
        if (title_bar.is_completed() || percent >= 100) {
            title_bar.set_option(indicators::option::ShowRemainingTime{false});
            title_bar.set_option(indicators::option::ForegroundColor{indicators::Color::green});
        }
        title_bar.set_progress(percent);
        file_in.open(file);
        file_in.peek();

        // Check if file is empty or if not able to open
        if (!file_in || file_in.eof())
            std::cout << "\n\nNo Loading " << file << "\n\n";
        else {
            while (!file_in.eof())
                load_title(titles);
        }

        if (file_in.is_open())
            file_in.close();
        ++progress;
    }

    if (titles.empty()) {
        indicators::show_console_cursor(true);
        std::cout << "\n\nNo titles found in files\n\n";
        return 0;
    }

    // Show cursor
    indicators::show_console_cursor(true);
    std::cout << termcolor::reset << "\n\nLoading " << titles.size() << " titles into the graph...\n";
    indicators::BlockProgressBar graph_titles_bar{indicators::option::BarWidth{80}, indicators::option::Start{"["}, indicators::option::End{"]"}, indicators::option::ShowElapsedTime{true}, indicators::option::ShowRemainingTime{true}, indicators::option::ForegroundColor{indicators::Color::red}, indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
    progress = 0;
    indicators::show_console_cursor(false);
    auto title_load_start{std::chrono::system_clock::now()};

    // Load in each title to the graph
    for (const auto &title : titles) {
        const double percent{100 * ((double)progress / (titles.size() - 1))};
        if (progress % 10000 == 0 || percent >= 100) {  // Only update progress bar every 1000 titles to save time
            if (graph_titles_bar.is_completed() || percent >= 100) {
                graph_titles_bar.set_option(indicators::option::ShowRemainingTime{false});
                graph_titles_bar.set_option(indicators::option::ForegroundColor{indicators::Color::green});
            }
            graph_titles_bar.set_progress(percent);
        }
        graph_vertex page;
        page.title = title;
        graph->push_back(page);
        ++progress;
    }

    if (graph->empty()) {
        indicators::show_console_cursor(true);
        std::cout << "\n\nNo titles loaded into graph\n\n";
        return 0;
    }

    indicators::show_console_cursor(true);
    std::cout << termcolor::reset << "\n\nLoading links into graph (Adjacent Nodes)...\n";

    indicators::BlockProgressBar links_bar{indicators::option::BarWidth{80}, indicators::option::Start{"["}, indicators::option::End{"]"}, indicators::option::ShowElapsedTime{true}, indicators::option::ShowRemainingTime{true}, indicators::option::ForegroundColor{indicators::Color::red}, indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
    progress = 0;
    indicators::show_console_cursor(false);
    auto link_load_start{std::chrono::system_clock::now()};
    // Load in each file's links
    for (const auto &file : file_names) {
        file_in.open(file);
        file_in.peek();

        // Check if file is empty or if not able to open
        if (!file_in || file_in.eof())
            std::cout << "\n\nNo Loading " << file << "\n\n";
        else {
            while (!file_in.eof())
                load_links(links_bar, progress);
        }

        if (file_in.is_open())
            file_in.close();
    }

    indicators::show_console_cursor(true);
    auto file_load_time{std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count()};
    std::cout << termcolor::reset << "\n\n" << graph->size() << " articles loaded from file in " << file_load_time << " seconds, or " << (float)file_load_time / 60 << " minutes!!!\n\n";
    fs::current_path(main_dir);
    return graph->size();
}


// Load in a title to the set
inline void wiki_loader::load_title(std::set<std::string> &titles) {
    std::string JSON_line;
    std::getline(file_in, JSON_line);
    file_in.peek();
    const json JSON = json::parse(JSON_line);  // { } Initialization breaks this ¯\_(ツ)_/¯

    std::string title{JSON[0].get<std::string>()};
    titles.insert(title);
    return;
}


// Load in a link to the graph
inline void wiki_loader::load_links(indicators::BlockProgressBar &bar, int &progress) {
    const double percent{100 * ((double)progress / (graph->size() - 1))};
    if (progress % 1000 == 0 || percent >= 100) {  // Only update progress bar every 1000 titles to save time
        if (!bar.is_completed()) {
            if (percent >= 100) {
                bar.set_option(indicators::option::ShowRemainingTime{false});
                bar.set_option(indicators::option::ForegroundColor{indicators::Color::green});
            }
            bar.set_progress(percent);
        }
    }
    std::string JSON_line;
    std::getline(file_in, JSON_line);
    file_in.peek();
    const json JSON = json::parse(JSON_line);  // { } Initialization breaks this ¯\_(ツ)_/¯

    std::string title{JSON[0].get<std::string>()};
    graph_vertex *page{graph->find(title)};

    if (page != nullptr) {
        for (const auto &link : JSON[1]) {
            graph_vertex *adjacent_page{graph->find(link.get<std::string>())};
            if (adjacent_page != nullptr) {
                page->adjacent.push_back(adjacent_page);
                ++adjacent_page->links_to;
            }
        }
        page->adjacent.shrink_to_fit();  // Shrink the adjacent vector to fit the number of adjacent nodes (good for memory 👍)
    }
    ++progress;
    return;
}