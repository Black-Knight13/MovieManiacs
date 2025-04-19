#ifndef FILTERING_H
#define FILTERING_H
#include <cmath>
#include <queue>
#include <map>
#include <iomanip>
#include <fstream>
#include <random>
#include "RBTree.h"
using namespace std;

class CollaborativeFiltering {
private:
    MovieRBTree movieTree;
    unordered_map<int, User> users;

    // Find users with similar taste (based on Pearson correlation)
    vector<pair<int, float>> findSimilarUsers(int movieId, int k) {
        auto node = movieTree.search(movieId);
        if (node == nullptr || node == movieTree.getNIL()) {
            return {};
        }

        vector<pair<int, float>> similarities;
        const auto& targetRatings = node->movie.userRatings;

        // Calculate similarity for each user who rated this movie
        for (const auto& [userId, rating] : targetRatings) {
            if (users.find(userId) == users.end()) continue;

            const User& user = users[userId];
            float similarity = calculatePearsonCorrelation(user.movieRatings, targetRatings);
            similarities.push_back({userId, similarity});
        }

        // Sort by similarity (descending)
        sort(similarities.begin(), similarities.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });

        // Return top k similar users
        if (static_cast<size_t>(k) < similarities.size()) {
            similarities.resize(k);
        }

        return similarities;
    }

    // Calculate Pearson correlation between two users' ratings
    float calculatePearsonCorrelation(const unordered_map<int, float>& ratings1,
                                     const unordered_map<int, float>& ratings2) {
        vector<float> r1, r2;

        // Find movies rated by both users
        for (const auto& [movieId, rating1] : ratings1) {
            auto it = ratings2.find(movieId);
            if (it != ratings2.end()) {
                r1.push_back(rating1);
                r2.push_back(it->second);
            }
        }

        int n = r1.size();
        if (n < 5) { // Need at least 5 common ratings for meaningful correlation
            return 0.0f;
        }

        // Calculate means
        float sum1 = 0, sum2 = 0;
        for (int i = 0; i < n; i++) {
            sum1 += r1[i];
            sum2 += r2[i];
        }
        float mean1 = sum1 / n;
        float mean2 = sum2 / n;

        // Calculate correlation
        float numerator = 0, denom1 = 0, denom2 = 0;
        for (int i = 0; i < n; i++) {
            float diff1 = r1[i] - mean1;
            float diff2 = r2[i] - mean2;
            numerator += diff1 * diff2;
            denom1 += diff1 * diff1;
            denom2 += diff2 * diff2;
        }

        if (denom1 == 0 || denom2 == 0) return 0.0f;

        return numerator / (sqrt(denom1) * sqrt(denom2));
    }
public:
    // CSV parsing helper function
    vector<string> parseCSVLine(const string& line) {
        vector<string> result;
        bool inQuotes = false;
        string field;

        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                result.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }

        result.push_back(field); // Add the last field
        return result;
    }

    // Load movies and user ratings from CSV files
    bool loadData(const string& moviesFile, const string& ratingsFile) {
        // Load movies
        ifstream movieStream(moviesFile);
        if (!movieStream.is_open()) {
            cerr << "Error opening movies file: " << moviesFile << endl;
            return false;
        }

        string line;
        // Skip header
        getline(movieStream, line);

        while (getline(movieStream, line)) {
            vector<string> fields = parseCSVLine(line);
            if (fields.size() >= 3) {
                int movieId = stoi(fields[0]);
                string title = fields[1];

                // Parse genres (comma-separated)
                vector<string> genres;
                string genresStr = fields[2];
                size_t pos = 0;
                while ((pos = genresStr.find('|')) != string::npos) {
                    genres.push_back(genresStr.substr(0, pos));
                    genresStr.erase(0, pos + 1);
                }
                if (!genresStr.empty()) {
                    genres.push_back(genresStr);
                }

                Movie movie(movieId, title);
                movie.genres = genres;

                movieTree.insert(movie);
            }
        }

        // Load ratings
        ifstream ratingStream(ratingsFile);
        if (!ratingStream.is_open()) {
            cerr << "Error opening ratings file: " << ratingsFile << endl;
            return false;
        }

        // Skip header
        getline(ratingStream, line);

        while (getline(ratingStream, line)) {
            vector<string> fields = parseCSVLine(line);
            if (fields.size() >= 3) {
                int userId = stoi(fields[0]);
                int movieId = stoi(fields[1]);
                float rating = stof(fields[2]);

                // Update user ratings
                if (users.find(userId) == users.end()) {
                    users[userId] = User(userId);
                }
                users[userId].movieRatings[movieId] = rating;

                // Update movie ratings
                auto node = movieTree.search(movieId);
                if (node != nullptr && node != movieTree.getNIL()) {
                    node->movie.userRatings[userId] = rating;
                }
            }
        }

        cout << "Loaded " << movieTree.inOrder().size() << " movies and " << users.size() << " users" << endl;
        return true;
    }

    // Get movie recommendations for a user based on a movie they liked
    vector<pair<Movie, float>> getRecommendations(int movieId, int numRecs = 5) {
        auto startTime = chrono::high_resolution_clock::now();

        // Find similar users who liked this movie
        vector<pair<int, float>> similarUsers = findSimilarUsers(movieId, 20);

        // Use a map to accumulate weighted ratings
        map<int, pair<float, float>> movieScores; // movieId -> (weighted sum, similarity sum)

        // For each similar user, get their highly rated movies
        for (const auto& [userId, similarity] : similarUsers) {
            if (similarity <= 0) continue; // Skip negatively correlated users

            const User& user = users[userId];
            for (const auto& [recMovieId, rating] : user.movieRatings) {
                // Skip the input movie and low ratings
                if (recMovieId == movieId || rating < 3.5) continue;

                // Weight the rating by user similarity
                auto& scoreData = movieScores[recMovieId];
                scoreData.first += similarity * rating;
                scoreData.second += similarity;
            }
        }

        // Use a priority queue to get top N recommendations
        using MovieScore = pair<float, int>; // score, movieId
        priority_queue<MovieScore> pq;

        for (const auto& [recMovieId, scoreData] : movieScores) {
            if (scoreData.second > 0) {
                float normalizedScore = scoreData.first / scoreData.second;
                pq.push({normalizedScore, recMovieId});
            }
        }

        // Extract top recommendations
        vector<pair<Movie, float>> recommendations;
        int count = 0;
        while (!pq.empty() && count < numRecs) {
            auto [score, recMovieId] = pq.top();
            pq.pop();

            auto node = movieTree.search(recMovieId);
            if (node != nullptr && node != movieTree.getNIL()) {
                recommendations.push_back({node->movie, score});
                count++;
            }
        }

        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

        cout << "Recommendation generation took " << duration << " ms" << endl;

        return recommendations;
    }

    // Performance analysis
    void analyzePerformance(int numTests = 100) {
        vector<int> movieIds = getRandomMovieIds(numTests);
        long long totalTime = 0;

        for (int movieId : movieIds) {
            auto startTime = chrono::high_resolution_clock::now();

            getRecommendations(movieId, 5);

            auto endTime = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
            totalTime += duration;
        }

        cout << "Average recommendation time: " << (totalTime / numTests) << " ms" << endl;

        // Memory usage analysis
        size_t totalMovieRatings = 0;
        for (auto& node : movieTree.inOrder()) {
            totalMovieRatings += node.userRatings.size();
        }

        size_t totalUserRatings = 0;
        for (const auto& [userId, user] : users) {
            totalUserRatings += user.movieRatings.size();
        }

        cout << "Memory usage statistics:" << endl;
        cout << "Total movie ratings: " << totalMovieRatings << endl;
        cout << "Total user ratings: " << totalUserRatings << endl;
        cout << "Approximate memory for ratings: "
             << (totalMovieRatings + totalUserRatings) * sizeof(pair<int, float>) / (1024 * 1024)
             << " MB" << endl;
    }

    // Get some random movie IDs for testing
    vector<int> getRandomMovieIds(int count) {
        vector<Movie> allMovies = movieTree.inOrder();
        vector<int> ids;

        if (allMovies.empty()) return ids;

        // Pick random movies
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, allMovies.size() - 1);

        for (int i = 0; i < count && !allMovies.empty(); i++) {
            int index = distrib(gen);
            ids.push_back(allMovies[index].movieId);
        }

        return ids;
    }

    // Get all movie ids
    vector<int> getAllMovieIds() {
        vector<Movie> allMovies = movieTree.inOrder();
        vector<int> ids;
        ids.reserve(allMovies.size());

        for (const auto& movie : allMovies) {
            ids.push_back(movie.movieId);
        }

        return ids;
    }

    // expose the raw MovieNode* search (for content filtering)
    MovieNode* getMovieNode(int movieId) {
        return movieTree.search(movieId);
    }

    // get all movies (in‑order) for iterating in content filtering
    vector<Movie> getAllMovies() {
        return movieTree.inOrder();
    }
};

#endif //FILTERING_H
