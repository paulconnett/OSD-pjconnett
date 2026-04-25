//

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using us = std::chrono::microseconds;

// -------- Global sink to defeat over-optimization --------
static volatile std::uint64_t g_sink = 0;

// -------- RNG, deterministic unless you pass a different seed --------
struct RNG {
    std::mt19937_64 eng;

    explicit RNG(std::uint64_t seed = 0xC0FFEE123456789ULL) : eng(seed) {}

    template <typename T>
    T uniform(T lo, T hi)
    {
        std::uniform_int_distribution<long long> dist((long long)lo, (long long)hi);
        return (T)dist(eng);
    }
};

// -------- Department Entry --------
struct DepartmentEntry {
    int phoneNumber;
    std::string department;
};

// -------- Dataset, built once and not inside timed regions --------
struct Dataset {
    std::vector<int> cache_vec;                       // data for cache busting
    std::vector<int> lookup_numbers;                  // numbers used for repeated lookup tests

    std::map<int, DepartmentEntry> department_map;    // scattered memory layout
    std::vector<DepartmentEntry> department_vec;      // adjacent memory layout

    int selectedPhoneNumber = -1;
    std::string selectedDepartment;

    explicit Dataset(const std::string& filename = "departments.txt",
                     std::size_t total_lookup_count = 1000000,
                     std::size_t bust_size = 32000000)
    {
        load_departments(filename);

        // Sort vector by phone number so vector lookup can use binary search.
        std::sort(department_vec.begin(), department_vec.end(),
            [](const DepartmentEntry& a, const DepartmentEntry& b) {
                return a.phoneNumber < b.phoneNumber;
            });

        // Build a fixed number of lookup values from the loaded file.
        // This keeps timing measurable without allocating too much memory.
        lookup_numbers.reserve(total_lookup_count);

        for (std::size_t i = 0; i < total_lookup_count; ++i) {
            std::size_t index = i % department_vec.size();
            lookup_numbers.push_back(department_vec[index].phoneNumber);
        }

        // Build a large vector used only to disturb cache state before each benchmark.
        cache_vec.resize(bust_size);

        for (std::size_t i = 0; i < cache_vec.size(); ++i) {
            cache_vec[i] = (int)i;
        }
    }

    void load_departments(const std::string& filename)
    {
        std::ifstream fin(filename);

        if (!fin) {
            std::cerr << "Could not open " << filename << "\n";
            std::cerr << "Expected format per line: phone_number department_name\n";
            std::exit(1);
        }

        int phoneNumber;
        std::string department;

        while (fin >> phoneNumber >> department) {
            DepartmentEntry entry;
            entry.phoneNumber = phoneNumber;
            entry.department = department;

            department_map.emplace(phoneNumber, entry);
            department_vec.push_back(entry);
        }

        if (department_vec.empty()) {
            std::cerr << "No department data loaded.\n";
            std::exit(1);
        }
    }

    void choose_extension()
    {
        int minExtension = department_vec.front().phoneNumber;
        int maxExtension = department_vec.front().phoneNumber;

        for (std::size_t i = 1; i < department_vec.size(); ++i) {
            if (department_vec[i].phoneNumber < minExtension) {
                minExtension = department_vec[i].phoneNumber;
            }

            if (department_vec[i].phoneNumber > maxExtension) {
                maxExtension = department_vec[i].phoneNumber;
            }
        }

        std::cout << "Enter an extension from " << minExtension
                  << " to " << maxExtension << ": ";
        std::cin >> selectedPhoneNumber;

        while (selectedPhoneNumber < minExtension || selectedPhoneNumber > maxExtension) {
            std::cout << "Invalid extension. Enter an extension from "
                      << minExtension << " to " << maxExtension << ": ";
            std::cin >> selectedPhoneNumber;
        }

        auto it = department_map.find(selectedPhoneNumber);

        if (it != department_map.end()) {
            selectedDepartment = it->second.department;
        } else {
            std::cerr << "Extension was not found.\n";
            std::exit(1);
        }
    }
};

// -------- Cache buster: random touches on a large vector --------
// Not a perfect cache flush, but it is portable and useful for this demonstration.
static void cache_buster(Dataset& ds, RNG& rng, std::size_t touches = 5000000)
{
    std::size_t n = ds.cache_vec.size();
    volatile std::uint64_t sum = 0;

    for (std::size_t i = 0; i < touches; ++i) {
        std::size_t index = rng.uniform<std::size_t>(0, n - 1);
        sum += (unsigned)ds.cache_vec[index];
    }

    g_sink += sum;
}

// -------- Benchmark harness plumbing --------
struct ResultRow {
    std::string test_name;
    int repeats;
    double usec;
};

struct Test {
    std::string name;
    std::string desc;
    std::function<std::uint64_t(Dataset& ds, RNG& rng)> body;
};

static std::vector<Test> make_tests()
{
    std::vector<Test> tests;

    // 1) Iterate over scattered std::map records.
    tests.push_back({
        "map_iter",
        "Iterate over std::map department records.",
        [](Dataset& ds, RNG&) -> std::uint64_t {
            std::uint64_t sum = 0;

            for (const auto& pair : ds.department_map) {
                sum += (unsigned)pair.second.phoneNumber;
                sum += (unsigned)pair.second.department.size();
            }

            g_sink += sum;
            return sum;
        }
    });

    // 2) Iterate over adjacent std::vector records.
    tests.push_back({
        "vec_iter",
        "Iterate over std::vector department records.",
        [](Dataset& ds, RNG&) -> std::uint64_t {
            std::uint64_t sum = 0;

            for (std::size_t i = 0; i < ds.department_vec.size(); ++i) {
                sum += (unsigned)ds.department_vec[i].phoneNumber;
                sum += (unsigned)ds.department_vec[i].department.size();
            }

            g_sink += sum;
            return sum;
        }
    });

    // 3) Repeated selected extension lookup using std::map.
    tests.push_back({
        "map_lookup",
        "Repeated selected extension lookup using std::map.",
        [](Dataset& ds, RNG&) -> std::uint64_t {
            std::uint64_t sum = 0;

            for (std::size_t i = 0; i < ds.lookup_numbers.size(); ++i) {
                auto it = ds.department_map.find(ds.selectedPhoneNumber);

                if (it != ds.department_map.end()) {
                    sum += (unsigned)it->second.phoneNumber;
                    sum += (unsigned)it->second.department.size();
                }
            }

            g_sink += sum;
            return sum;
        }
    });

    // 4) Repeated selected extension lookup using sorted std::vector and binary search.
    tests.push_back({
        "vec_lookup",
        "Repeated selected extension lookup using sorted std::vector.",
        [](Dataset& ds, RNG&) -> std::uint64_t {
            std::uint64_t sum = 0;

            for (std::size_t i = 0; i < ds.lookup_numbers.size(); ++i) {
                auto it = std::lower_bound(ds.department_vec.begin(), ds.department_vec.end(),
                    ds.selectedPhoneNumber,
                    [](const DepartmentEntry& entry, int phoneNumber) {
                        return entry.phoneNumber < phoneNumber;
                    });

                if (it != ds.department_vec.end() && it->phoneNumber == ds.selectedPhoneNumber) {
                    sum += (unsigned)it->phoneNumber;
                    sum += (unsigned)it->department.size();
                }
            }

            g_sink += sum;
            return sum;
        }
    });

    return tests;
}

// -------- Timing helper --------
template <class F>
static double time_us(F&& function)
{
    auto start = Clock::now();
    function();
    auto end = Clock::now();

    return (double)std::chrono::duration_cast<us>(end - start).count();
}

// -------- CLI helper --------
static void print_usage(const std::vector<Test>& tests)
{
    std::cerr << "Usage: bench_mem [--all|--list|TEST ...] [--seed=N]\n\n";
    std::cerr << "Tests:\n";

    for (auto& test : tests) {
        std::cerr << "  " << std::left << std::setw(12)
                  << test.name << " - " << test.desc << "\n";
    }

    std::cerr << "\nExamples:\n";
    std::cerr << "  ./bench_mem --all\n";
    std::cerr << "  ./bench_mem map_iter vec_iter --seed=12345\n";
}

// -------- Main --------
int main(int argc, char** argv)
{
    std::vector<Test> tests = make_tests();

    bool run_all = false;
    std::uint64_t seed = 0xDEADBEEF42ULL;
    std::vector<std::string> selected;

    // Parse args.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--list") {
            print_usage(tests);
            return 0;
        } else if (arg == "--all") {
            run_all = true;
        } else if (arg.rfind("--seed=", 0) == 0) {
            seed = std::stoull(arg.substr(7));
        } else {
            selected.push_back(arg);
        }
    }

    if (!run_all && selected.empty()) {
        print_usage(tests);
        return 1;
    }

    // Filter selected tests.
    std::vector<Test> to_run;

    if (run_all) {
        to_run = tests;
    } else {
        for (auto& name : selected) {
            auto it = std::find_if(tests.begin(), tests.end(),
                [&](const Test& test) {
                    return test.name == name;
                });

            if (it != tests.end()) {
                to_run.push_back(*it);
            } else {
                std::cerr << "Unknown test: " << name << "\n";
                print_usage(tests);
                return 2;
            }
        }
    }

    // Build dataset outside timed region.
    Dataset ds;
    RNG rng(seed);

    // Let user choose which extension to look up.
    ds.choose_extension();

    std::cout << "\nSelected Extension:   " << ds.selectedPhoneNumber << "\n";
    std::cout << "Department:           " << ds.selectedDepartment << "\n\n";

    // Repeats reveal caching effects, same as the original harness style.
    const int repeats_set[3] = {1, 5, 100};

    std::vector<ResultRow> results;
    results.reserve(to_run.size() * 3);

    // Run benchmarks.
    for (const auto& test : to_run) {
        for (int repeats : repeats_set) {
            cache_buster(ds, rng);

            std::uint64_t checksum = 0;

            double elapsed_us = time_us([&]() {
                for (int r = 0; r < repeats; ++r) {
                    checksum += test.body(ds, rng);
                }
            });

            g_sink += checksum;

            results.push_back({test.name, repeats, elapsed_us});

            std::cout << std::left << std::setw(12) << test.name
                      << " x" << std::setw(3) << repeats
                      << " -> " << std::setw(10) << (std::uint64_t)elapsed_us << " us"
                      << "  (checksum=" << checksum << ")\n";
        }
    }

    std::cout << "\n=== Summary (microseconds) ===\n";
    std::cout << std::left << std::setw(16) << "test"
              << std::right << std::setw(12) << "x1"
              << std::setw(12) << "x5"
              << std::setw(12) << "x100"
              << std::setw(12) << "x5/x1"
              << std::setw(12) << "x100/x1"
              << "\n";

    struct Row {
        double x1 = -1;
        double x5 = -1;
        double x100 = -1;
    };

    std::map<std::string, Row> table;

    for (const auto& result : results) {
        auto& row = table[result.test_name];

        if (result.repeats == 1) {
            row.x1 = result.usec;
        }

        if (result.repeats == 5) {
            row.x5 = result.usec;
        }

        if (result.repeats == 100) {
            row.x100 = result.usec;
        }
    }

    for (const auto& [name, row] : table) {
        double r51 = (row.x1 > 0) ? row.x5 / row.x1 : -1;
        double r1001 = (row.x1 > 0) ? row.x100 / row.x1 : -1;

        std::cout << std::left << std::setw(16) << name
                  << std::right << std::setw(12) << (std::uint64_t)row.x1
                  << std::setw(12) << (std::uint64_t)row.x5
                  << std::setw(12) << (std::uint64_t)row.x100
                  << std::setw(12) << std::fixed << std::setprecision(2) << r51
                  << std::setw(12) << std::fixed << std::setprecision(2) << r1001
                  << "\n";
    }

    std::cout << "\n[anti-opt sink] " << g_sink << "\n";

    return 0;
}
