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
#include <unordered_map>

class LazyIndexBenchmark {
    public:
    using wireIndex = std::unordered_map
        < unsigned int, //event ID
          std::vector<std::size_t> // wire container position array
        >;

        struct timingResult {
            int numEvents;
            double indexBuildTimeMs; //one time first pass
            double lookupTimeMs; // all lookups using hashmap
            double totalTimeMs; // combined time
            long long correlations; // same as before
        };

        static timingResult benchmarkWithIndex(
            const std::string& hitsFile,
            const std::string& wiresFile,
            int numEvents
        ) {
            auto hitsReader = ROOT::RNTupleReader::Open("aos_hits", hitsFile);
            auto wiresReader = ROOT::RNTupleReader::Open("aos_wires",wiresFile);
            if (!hitsReader || !wiresReader) {
                std::cerr << "Error opening files: " << hitsFile << " or " << wiresFile << std::endl;
                return {numEvents, -1.0, -1.0, -1.0, 0};
            }

            const auto nHitEntries = hitsReader->GetNEntries();
            const auto nWireEntries = wiresReader->GetNEntries();
            const auto maxEvents = std::min<std::size_t>(static_cast<std::size_t>(numEvents), nHitEntries);
            std::cout << "Total hit entries: " << nHitEntries
                      << ", Total wire entries: " << nWireEntries << std::endl;
            
            auto hitView = hitsReader->GetView<std::vector<HitIndividual>>("hits");
            auto wireView = wiresReader->GetView<std::vector<WireIndividual>>("wires");

            const auto indexStartTime = std::chrono::high_resolution_clock::now();

            wireIndex wireIndexMap;

            //index build

            for (std::size_t j = 0; j < nWireEntries; ++j) {
                const auto& wires = wireView(j);
                if (!wires.empty()) {
                    for (const auto& wire : wires) {
                        wireIndexMap[wire.EventID].push_back(j);
                    }
                }
            }

            const auto indexEndTime = std::chrono::high_resolution_clock::now();
            const auto indexBuildDuration = std::chrono::duration_cast<std::chrono::milliseconds>(indexEndTime - indexStartTime);

            std::cout << "Index built with " << wireIndexMap.size() << " unique EventIDs." << std::endl;
            std::cout << "Index build time: " << indexBuildDuration.count() << " ms" << std::endl;

            
            //index lookup 

            const auto lookupStartTime = std::chrono::high_resolution_clock::now();
            long long correlations = 0;
            for (std::size_t i = 0; i < maxEvents; ++i) {
                const auto& hits = hitView(i);
                for (const auto& hit : hits) {
                    auto it = wireIndexMap.find(hit.EventID);
                    if (it != wireIndexMap.end()) {
                        correlations += it->second.size();
                    }
                }


        }

        const auto lookupEndTime = std::chrono::high_resolution_clock::now();
        const auto lookupDuration = std::chrono::duration_cast<std::chrono::milliseconds>(lookupEndTime - lookupStartTime);

        const double totalDuration = indexBuildDuration.count() + lookupDuration.count();
        return {numEvents, static_cast<double>(indexBuildDuration.count()), static_cast<double>(lookupDuration.count()), totalDuration, correlations};
        }

    };   


int main() {
    // Load the Wire dictionary shared library.
    // ROOT needs this to deserialize WireIndividual objects from the file.
    // Matches your naive scan exactly.
    gSystem->Load("libWireDict");

    // Same file paths as your naive scan
    const std::string hitsFile = "output/aos_event_perData.root";
    const std::string wiresFile = "output/aos_event_perData.root";

    // Same file existence check as your naive scan
    if (!std::filesystem::exists(hitsFile) || !std::filesystem::exists(wiresFile)) {
        std::cerr << "Error: One or both input files do not exist." << std::endl;
        std::cerr << "Run main hitwire benchmark first to generate the necessary files." << std::endl;
        return 1;
    }

    // ASCII art header matching your naive scan's style
    std::cout << "******************" << std::endl;
    std::cout << R"(
 █ █████                                          █████                █████                         █████                                  █████                                          █████     
░░███                                          ░░███                ░░███                         ░░███                                  ░░███                                          ░░███      
 ░███         ██████    █████████ █████ ████    ░███  ████████    ███████   ██████  █████ █████    ░███████   ██████  ████████    ██████  ░███████   █████████████    ██████   ████████  ░███ █████
 ░███        ░░░░░███  ░█░░░░███ ░░███ ░███     ░███ ░░███░░███  ███░░███  ███░░███░░███ ░░███     ░███░░███ ███░░███░░███░░███  ███░░███ ░███░░███ ░░███░░███░░███  ░░░░░███ ░░███░░███ ░███░░███ 
 ░███         ███████  ░   ███░   ░███ ░███     ░███  ░███ ░███ ░███ ░███ ░███████  ░░░█████░      ░███ ░███░███████  ░███ ░███ ░███ ░░░  ░███ ░███  ░███ ░███ ░███   ███████  ░███ ░░░  ░██████░  
 ░███      █ ███░░███    ███░   █ ░███ ░███     ░███  ░███ ░███ ░███ ░███ ░███░░░    ███░░░███     ░███ ░███░███░░░   ░███ ░███ ░███  ███ ░███ ░███  ░███ ░███ ░███  ███░░███  ░███      ░███░░███ 
 ███████████░░████████  █████████ ░░███████     █████ ████ █████░░████████░░██████  █████ █████    ████████ ░░██████  ████ █████░░██████  ████ █████ █████░███ █████░░████████ █████     ████ █████
░░░░░░░░░░░  ░░░░░░░░  ░░░░░░░░░   ░░░░░███    ░░░░░ ░░░░ ░░░░░  ░░░░░░░░  ░░░░░░  ░░░░░ ░░░░░    ░░░░░░░░   ░░░░░░  ░░░░ ░░░░░  ░░░░░░  ░░░░ ░░░░░ ░░░░░ ░░░ ░░░░░  ░░░░░░░░ ░░░░░     ░░░░ ░░░░░ 
                                   ███ ░███                                                                                                                                                        
                                  ░░██████                                                                                                                                                         
                                   ░░░░░░                                                                                                                                                          

    )" << std::endl;
    std::cout << "******************" << std::endl;

    const std::vector<int> eventCounts = {100, 1000};
    std::vector<LazyIndexBenchmark::timingResult> results;

    for (int numEvents : eventCounts) {
        std::cout << "Building index and looking up " 
                  << numEvents << " events..." << std::endl;

        const auto result = LazyIndexBenchmark::benchmarkWithIndex(
            hitsFile, wiresFile, numEvents
        );
        results.push_back(result);

       
        std::cout << "  Index build time: " << std::fixed << std::setprecision(2)
                  << result.indexBuildTimeMs << " ms\n";
        std::cout << "  Lookup time:      " << std::fixed << std::setprecision(2)
                  << result.lookupTimeMs << " ms\n";
        std::cout << "  Total time:       " << std::fixed << std::setprecision(2)
                  << result.totalTimeMs << " ms\n";
        std::cout << "  Correlations found: " << result.correlations << "\n\n";
    }

    
    std::cout << "LAZY INDEX RESULTS SUMMARY\n";
    std::cout << "========================================\n\n";

    
    std::cout << std::left
              << std::setw(12) << "Events"
              << std::setw(18) << "Index Build (ms)" 
              << std::setw(16) << "Lookup (ms)"       
              << std::setw(14) << "Total (ms)"       
              << std::setw(20) << "Correlations\n";

    
    std::cout << std::string(80, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(12) << r.numEvents
                  << std::setw(18) << std::fixed << std::setprecision(2)
                  << r.indexBuildTimeMs
                  << std::setw(16) << std::fixed << std::setprecision(2)
                  << r.lookupTimeMs
                  << std::setw(14) << std::fixed << std::setprecision(2)
                  << r.totalTimeMs
                  << std::setw(20) << r.correlations << "\n";
    }

   
    std::cout << "\nNOTE: Correlations count must match naive scan output exactly.\n";
    std::cout << "If counts differ, the index has a correctness bug.\n";

    return 0;
}