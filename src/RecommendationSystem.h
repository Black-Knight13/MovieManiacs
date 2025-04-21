#ifndef RECOMMENDATIONSYSTEM_H
#define RECOMMENDATIONSYSTEM_H
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "Filtering.h"


using namespace std;

class RecommendationSystem {
    CollaborativeFiltering cfSystem;

    // Content-Based filtering still needs to be implemented | ContentBasedFiltering cbSystem

    unordered_map<string, int> titleToId; // For title lookup
    unordered_map<int, string> idToTitle; // For reverse lookup

public:
    bool initialize(const string& moviesFile, const string& ratingsFile) {
        auto startTime = chrono::high_resolution_clock::now();

        // Parse movies file first to build title maps
        if (!parseMoviesFile(moviesFile)) {
            return false;
        }

        bool success = cfSystem.loadData(moviesFile, ratingsFile);

        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();

        cout << "Data loading took " << duration << " seconds" << endl;
        cout << "Loaded " << titleToId.size() << " movies" << endl;

        return success;
    }

    // Function to parse CSV files and build the titleToId map
    bool parseMoviesFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return false;
        }

        string line;
        // Skip header
        getline(file, line);

        while (getline(file, line)) {
            stringstream ss(line);
            string idStr, title, genres;

            // Parse CSV (handling potential commas in movie titles)
            getline(ss, idStr, ',');
            getline(ss, title, ',');

            // Handle movies with commas in their titles
            if (title.front() == '"' && title.back() != '"') {
                string extraPart;
                while (getline(ss, extraPart, ',')) {
                    title += "," + extraPart;
                    if (extraPart.back() == '"') break;
                }
            }

            // Clean up title
            if (title.front() == '"' && title.back() == '"') {
                title = title.substr(1, title.length() - 2);
            }

            // Store mapping
            int id = stoi(idStr);
            titleToId[title] = id;
            idToTitle[id] = title;
        }

        return true;
    }

    // Get movie recommendations based on a title
    void getRecommendationsByTitle(const string& title) {
        auto it = titleToId.find(title);
        if (it == titleToId.end()) {
            cout << "Movie not found: " << title << endl;
            // Suggest similar titles
            suggestSimilarTitles(title);
            return;
        }

        int movieId = it->second;

        // Get recommendations using collaborative filtering
        auto startTime = chrono::high_resolution_clock::now();
        auto cfRecommendations = cfSystem.getRecommendations(movieId);
        auto endTime = chrono::high_resolution_clock::now();
        auto cfTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

        // Display recommendations
        cout << "\nCollaborative Filtering Recommendations for \"" << title << "\":" << endl;
        cout << "-----------------------------------------------------------------------------" << endl << endl;
        for (const auto& [movie, score] : cfRecommendations) {
            cout << movie.title << " (Score: " << fixed << setprecision(2) << score << ")" << endl;
        }
        cout << "Time: " << cfTime << " ms" << endl;

        // We'd add content-based filtering recommendations here as well
        auto cbRecs = getContentRecommendations(movieId);
        cout << "\nContent-Based Recommendations for \"" << title << "\":" << endl;
        cout << "-----------------------------------------------------------------------------" << endl << endl;
        for (const auto& [movie, score] : cbRecs) {
            cout << movie.title << " (Genre-Overlap Score: " << fixed << setprecision(2) << score << ")" << endl;
        }
    }

    // Suggest similar titles if the exact title isn't found
    void suggestSimilarTitles(const string& query) {
        vector<pair<string, float>> similarTitles;

        for (const auto& [title, id] : titleToId) {
            // Simple string similarity
            float similarity = calculateStringSimilarity(query, title);
            if (similarity > 0.5) { // Threshold
                similarTitles.emplace_back(title, similarity);
            }
        }

        // Sort by similarity
        sort(similarTitles.begin(), similarTitles.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });

        // Show top suggestions
        if (!similarTitles.empty()) {
            cout << "Did you mean:" << endl;
            int count = 0;
            for (const auto& [title, sim] : similarTitles) {
                cout << "  " << title << endl;
                if (++count >= 5) break;
            }
        }
    }

    // Calculate string similarity using Levenshtein distance
    float static calculateStringSimilarity(const string& s1, const string& s2) {
        // Convert to lowercase for case-insensitive comparison
        string s1_lower = s1;
        string s2_lower = s2;
        transform(s1_lower.begin(), s1_lower.end(), s1_lower.begin(), ::tolower);
        transform(s2_lower.begin(), s2_lower.end(), s2_lower.begin(), ::tolower);

        // Levenshtein distance implementation
        const size_t len1 = s1_lower.size();
        const size_t len2 = s2_lower.size();
        vector<vector<unsigned int>> diff(len1 + 1, vector<unsigned int>(len2 + 1));

        for(unsigned int i = 0; i <= len1; ++i) diff[i][0] = i;
        for(unsigned int i = 0; i <= len2; ++i) diff[0][i] = i;

        for(unsigned int i = 1; i <= len1; ++i) {
            for(unsigned int j = 1; j <= len2; ++j) {
                diff[i][j] = min({
                    diff[i - 1][j] + 1, // deletion
                    diff[i][j - 1] + 1, // insertion
                    diff[i - 1][j - 1] + (s1_lower[i - 1] == s2_lower[j - 1] ? 0 : 1) // substitution
                });
            }
        }

        // Calculate similarity as 1 - normalized distance
        float maxLen = max(len1, len2);
        if (maxLen == 0) return 1.0; // Both strings empty

        return 1.0f - (diff[len1][len2] / maxLen);
    }

    vector<pair<Movie, float>> getContentRecommendations(int movieId, int numRecs = 5) {
        // 1) Find the target movie node
        MovieNode* target = cfSystem.getMovieNode(movieId);
        if (!target) return {};

        // 2) Build a max‑heap of (score, movieId)
        struct Scored { float score; int id;
            bool operator<(Scored const& o) const { return score < o.score; }
        };
        priority_queue<Scored> heap;

        // 3) Compute simple genre‑overlap score for every other movie
        auto allMovies = cfSystem.getAllMovies();
        for (auto &m : allMovies) {
            if (m.movieId == movieId) continue;
            float score = 0;
            for (auto &g1 : target->movie.genres)
                for (auto &g2 : m.genres)
                    if (g1 == g2) score += 1.0f;  // +1 per matching genre
            heap.push({score, m.movieId});
        }

        // 4) Pop top numRecs and assemble results
        vector<pair<Movie, float>> recs;
        for (int i = 0; i < numRecs && !heap.empty(); ++i) {
            auto top = heap.top(); heap.pop();
            MovieNode* node = cfSystem.getMovieNode(top.id);
            if (node)
                recs.emplace_back( node->movie, top.score );
        }
        return recs;
    }

    // Test the Red-Black Tree operations specifically
    void testTreeOperations() {
        cout << "\nTesting Red-Black Tree operations..." << endl;
        auto allMovieIds = cfSystem.getAllMovieIds();
        if (allMovieIds.empty()) {
            cout << "No movies available for testing" << endl;
            return;
        }
        cout << "Enter a movie ID to search: ";
        int num;
        cin >> num;

        auto start = chrono::high_resolution_clock::now();
        auto result = cfSystem.getMovieNode(num);
        auto duration = chrono::duration_cast<chrono::microseconds>(
                chrono::high_resolution_clock::now() - start
        ).count();

        if (result) {
            cout << "Movie found: " << result->movie.title << endl;
        } else {
            cout << "Movie not found." << endl;
        }
        cout << "Search took " << duration << " μs" << endl;
    }

    // Run performance benchmark
    void runPerformanceBenchmark() {
        cout << "\nRunning performance benchmark..." << endl;
        cfSystem.analyzePerformance();
    }

};

#endif //RECOMMENDATIONSYSTEM_H