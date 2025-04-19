//
// Created by jackson turnbull on 4/16/25.
//

#include "Filtering.h"
#include "RBTree.h"
#include "RecommendationSystem.h"

int main() {
    RecommendationSystem sys;
    if (!sys.initialize("movies.csv", "ratings.csv")) {
        return 1;
    }

    cout << "Enter a movie title: ";
    string title;
    getline(cin, title);
    sys.getRecommendationsByTitle(title);
    return 0;
}
