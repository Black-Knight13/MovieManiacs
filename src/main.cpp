//
// Created by jackson turnbull on 4/16/25.
//

#include "Filtering.h"
#include "RBTree.h"
#include "RecommendationSystem.h"


void printMenu() {
    cout << "\n====== MovieManiacs Recommendation System ======\n";
    cout << "1. Get recommendations by movie title\n";
    cout << "2. Run performance benchmark\n";
    cout << "3. Test Red-Black Tree operations\n";
    cout << "4. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    RecommendationSystem sys;
    if (!sys.initialize("../Movie Data/movies.csv", "rankings.csv")) {
        return 1;
    }

    int choice;
    while (true) {
        printMenu();
        cin >> choice;
        cin.ignore();
        if (choice == 1) {
            cout << "Enter a movie title: ";
            string title;
            getline(cin, title);
            sys.getRecommendationsByTitle(title);
        } else if (choice == 2) {
            sys.runPerformanceBenchmark();
        } else if (choice == 3) {
            sys.testTreeOperations();
        } else if (choice == 4) {
            cout << "Thank you for using MovieManaics, goodbye!\n";
            break;
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;

}
