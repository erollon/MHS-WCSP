#include "wcsp_solver.hh"
#include "config.hh"
#include <random>

HsOption HsConfig::hsOption = HS_GREEDY;

vector<vector<int>> read_partitions(string partition_file, const Wcsp& wcsp) {//int nfuncs) {
    // Particiones que están en partition_file
    fstream file(partition_file);
    if (not file.is_open()) {
      cerr << "Error: File " << partition_file << "cannot be opened" << endl;
      exit(EXIT_FAILURE);
    }

    int nfuncs = wcsp.functions.size();
    vector<vector<int>> part;
    part.reserve(nfuncs);
    vector<bool> done(nfuncs, false); // <-- checking
    int x;
    int fs = 0;
    while (file >> x) {
        // line ends with -1
        vector<int> line;
        while (x != -1) {
            assert(x >= 0 and x < nfuncs);
            assert(not done[x]);
            done[x] = true;
            ++fs;
            if (wcsp.costs[x].size() > 1) line.push_back(x);  // soft
            file >> x;
        }
        if (line.size() > 0) part.push_back(line);
    }
    file.close();

    if (fs != nfuncs) { cout << "ERROR: invalid set of partitions." << endl; exit(0); }

    cout << "Sizes of original partitions:";
    for (int i = 0; i < part.size(); ++i) cout << " " << part[i].size();
    cout << endl;

    return part;
}

void restrict_size(vector<vector<int>>& part, int m_size) {
    if (m_size  == -1) return;

    for (int i = 0; i < part.size(); ++i) {
        if (part[i].size() > m_size) { // breaks partition
            part.push_back(vector<int>(part[i].begin() + m_size, part[i].end()));
            part[i].erase(part[i].begin() + m_size, part[i].end());
        }
    }

    cout << "Sizes of partitions when restricted:";
    for (int i = 0; i < part.size(); ++i) cout << " " << part[i].size();
    cout << endl;
}

void printHelp(string p) {
    cout << "USAGE:" << endl;
    cout << "\t" << p << " -f filename [options]" << endl;
    cout << "\t\t -p partition_file : file with functions in each partition" << endl;
    cout << "\t\t\t if partition_file == 'none' then bacchus and globals compacted" << endl;
    cout << "\t\t\t if partition_file == 'all' all functions in one cluster" << endl;
    cout << "\t\t -s int : max size de las particiones (default: -1 ==> w/o restriction)"  << endl;
    cout << "\t\t -ac : abstract cores" << endl;
    cout << "\t\t\t sólo tiene sentido si se indica -p : agrupa por pesos estratificados las funciones del cluster" << endl;
    cout << "\t\t -g n type: case study problem generator" << endl;
    cout << "\t\t\t n : int (number of variables)" << endl;
    cout << "\t\t\t type : int (= 0: all diferent; = 1: greater than)" << endl;
    cout << "\t\t -t number: hs-min = 1, hs-lazy = 2, hs-greedy = 3 (default), hs-max = 4" << endl;

    cout << "AAAI 24:" << endl;
    cout << "./main -g n TYPE [-p none] --> modelo original, bacchus" << endl;
    cout << "./main -g n TYPE [-p none] -ac --> symbolic merging of all functions (abstract cores en donde peso w se considera en todas las funciones)" << endl;
    cout << "./main -g n TYPE -p all --> numerical merging of all functions" << endl;
    cout << "    con la opcion -t se elige la forma en que se mejora el core" << endl;
}

int main(int argc, char const *argv[]) {
    string filename, partition_file;
    int p_size = -1;
    bool abstract_core = false;
    bool generator = false;
    int gen_type = -1;
    int gen_n = -1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i],"-h") == 0) {
            printHelp(argv[0]);
            exit(0);
        }
        else if (strcmp(argv[i],"-f") == 0) filename = argv[i + 1];
        else if (strcmp(argv[i],"-p") == 0) partition_file = argv[i + 1];
        else if (strcmp(argv[i],"-s") == 0) p_size = atoi(argv[i + 1]);
        else if (strcmp(argv[i],"-ac") == 0) abstract_core = true;
        else if (strcmp(argv[i],"-t") == 0) HsConfig::hsOption = static_cast<HsOption>(atoi(argv[i + 1]));
        else if (strcmp(argv[i],"-g") == 0) {
            generator = true;
            gen_n = atoi(argv[i + 1]);
            gen_type = atoi(argv[i + 2]);
        }
    }

    //cout << "checking hsoption .. " << HsConfig::hsOption << endl;

    // ---- checking options:
    if (filename.size() == 0) {
        if (not generator) {
            cout << "Error: missing wcsp file or -g option." << endl;
            exit(0);
        }
    }
    else {
        if (generator) {
            cout << "Error: incompatible options -f -g" << endl;
            exit(0);
        }
    }
    if (generator) {
        if (0 != gen_type and gen_type != 1) {
            cout << "Error: incorrect case study problem type" << endl;
            exit(0);
        }
        if (partition_file.size() != 0 and partition_file != "none" and partition_file != "all") {
            cout << "Error: case study problem partition style should be none or all" << endl;
            exit(0);
        }
        if (partition_file == "all" and abstract_core) {
            cout << "Error: incompatible types -p all -ac" << endl;
            exit(0);
        }
    }

    Wcsp wcsp;
    WcspSolver* solver = nullptr;

    if (generator) {  // case study
        wcsp.create_case_study(gen_n, gen_type);
        if (partition_file == "all") {  // numerical merging of all functions
            vector<vector<int>> part;
            part.push_back({});
            for (int i = 0; i < wcsp.nfuncs; ++i) part[0].push_back({i});
            solver = new WcspSolver(wcsp, part);
        }
        else {  // orig or symbolic merging
            solver = new WcspSolver(wcsp);
            if (abstract_core) solver->case_study_abstract_core(); // symbolic merging of all functions
        }
    }
    else {  // instance file
        wcsp.read(filename);
        if (partition_file.size() == 0) solver = new WcspSolver(wcsp); // orig
        else {
            vector<vector<int>> part;
            if (partition_file == "all") {
                part.push_back({});
                for (int i = 0; i < wcsp.nfuncs; ++i) part[0].push_back({i});
            }
            else if (partition_file == "none") {
                for (int i = 0; i < wcsp.nfuncs; ++i) part.push_back({i});
            }
            else part = read_partitions(partition_file, wcsp);
            restrict_size(part, p_size);

            if (abstract_core) part = wcsp.partition_abstract_core(part); // symbolic over partitions
                                                                          // else numerical over partitions
            solver = new WcspSolver(wcsp, part);
        }
    }
    assert(solver);

    auto start = high_resolution_clock::now();
    Cost opt = solver->solve();
    auto stop = high_resolution_clock::now();
    delete solver;

    auto duration = duration_cast<microseconds>(stop - start);
    double time = duration.count() / 1000000.0;
    cout << "Optimum: " << opt << " in " << time << " seconds." << endl;
}
