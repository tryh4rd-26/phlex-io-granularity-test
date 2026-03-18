#include <ROOT/RNTupleReader.hxx>
#include <TSystem.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "Hit.hpp"
#include "Wire.hpp"

class NaiveScanBenchmark {
public:
    struct TimingResult {
        int numEvents;
        double scanTimeMs;
        long long correlations; // Number of hit-wire EventID matches
    };

    static TimingResult benchmarkScan(const std::string& hitsFile,
                                      const std::string& wiresFile,
                                      int numEvents) {
        auto hitsReader = ROOT::RNTupleReader::Open("aos_hits", hitsFile);
        auto wiresReader = ROOT::RNTupleReader::Open("aos_wires", wiresFile);
        if (!hitsReader || !wiresReader) {
            std::cerr << "Error opening files: " << hitsFile << " or " << wiresFile << std::endl;
            return {numEvents, -1.0, 0};
        }

        const auto nHitEntries = hitsReader->GetNEntries();
        const auto nWireEntries = wiresReader->GetNEntries();
        const auto maxEvents = std::min<std::size_t>(static_cast<std::size_t>(numEvents), nHitEntries);

        std::cout << "Total hit entries: " << nHitEntries
                  << ", Total wire entries: " << nWireEntries << std::endl;

        auto hitView = hitsReader->GetView<std::vector<HitIndividual>>("hits");
        auto wireView = wiresReader->GetView<std::vector<WireIndividual>>("wires");

        const auto startTime = std::chrono::high_resolution_clock::now();

        long long correlations = 0;
        for (std::size_t i = 0; i < maxEvents; ++i) {
            const auto& hits = hitView(i);
            for (std::size_t j = 0; j < nWireEntries; ++j) {
                const auto& wires = wireView(j);
                if (!hits.empty() && !wires.empty()) {
                    for (const auto& hit : hits) {
                        for (const auto& wire : wires) {
                            if (hit.EventID == wire.EventID) {
                                correlations++;
                            }
                        }
                    }
                }
            }
        }

        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        return {numEvents, static_cast<double>(duration.count()), correlations};
    }
};

int main() {
    gSystem->Load("libWireDict");

    const std::string hitsFile = "output/aos_event_perData.root";
    const std::string wiresFile = "output/aos_event_perData.root";

    if (!std::filesystem::exists(hitsFile) || !std::filesystem::exists(wiresFile)) {
        std::cerr << "Error: One or both input files do not exist." << std::endl;
        std::cerr << "Run main hitwire benchmark first to generate the necessary files." << std::endl;
        return 1;
    }

    std::cout << "******************" << std::endl;
    std::cout << R"( ██████   █████            ███                          ███████████                    █████                                          █████
░░██████ ░░███            ░░░                          ░░███░░░░░███                  ░░███                                          ░░███
 ░███░███ ░███   ██████   ████  █████ █████  ██████     ░███    ░███  ██████   ██████  ░███████   █████████████    ██████   ████████  ░███ █████
 ░███░░███░███  ░░░░░███ ░░███ ░░███ ░░███  ███░░███    ░██████████  ███░░███ ███░░███ ░███░░███ ░░███░░███░░███  ░░░░░███ ░░███░░███ ░███░░███
 ░███ ░░██████   ███████  ░███  ░███  ░███ ░███████     ░███░░░░░███░███████ ░███ ░░░  ░███ ░███  ░███ ░███ ░███   ███████  ░███ ░░░  ░██████░
 ░███  ░░█████  ███░░███  ░███  ░░███ ███  ░███░░░      ░███    ░███░███░░░  ░███  ███ ░███ ░███  ░███ ░███ ░███  ███░░███  ░███      ░███░░███
 █████  ░░█████░░████████ █████  ░░█████   ░░██████     ███████████ ░░██████ ░░██████  ████ █████ █████░███ █████░░████████ █████     ████ █████
░░░░░    ░░░░░  ░░░░░░░░ ░░░░░    ░░░░░     ░░░░░░     ░░░░░░░░░░░   ░░░░░░   ░░░░░░  ░░░░ ░░░░░ ░░░░░ ░░░ ░░░░░  ░░░░░░░░ ░░░░░     ░░░░ ░░░░░  )"
              << std::endl;
    std::cout << "******************" << std::endl;

    const std::vector<int> eventCounts = {100, 1000};
    std::vector<NaiveScanBenchmark::TimingResult> results;

    for (int numEvents : eventCounts) {
        std::cout << "Scanning " << numEvents << " events..." << std::endl;
        const auto result = NaiveScanBenchmark::benchmarkScan(hitsFile, wiresFile, numEvents);
        results.push_back(result);
        std::cout << "  Time: " << std::fixed << std::setprecision(2) << result.scanTimeMs << " ms\n";
        std::cout << "  Correlations found: " << result.correlations << "\n\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "NAIVE SCAN RESULTS SUMMARY\n";
    std::cout << "========================================\n\n";
    std::cout << std::left << std::setw(15) << "Events"
              << std::setw(20) << "Scan Time (ms)"
              << std::setw(20) << "Correlations\n";
    std::cout << std::string(55, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(15) << r.numEvents
                  << std::setw(20) << std::fixed << std::setprecision(2) << r.scanTimeMs
                  << std::setw(20) << r.correlations << "\n";
    }

    return 0;
}