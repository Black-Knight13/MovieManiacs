//
// Created by jackson turnbull on 4/16/25.
//
#ifndef RBTREE_H
#define RBTREE_H
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

using namespace std;

// Colors for Red-Black Tree
enum Color { RED, BLACK };

// Movie structure to store essential information
struct Movie {
    int movieId;
    string title;
    vector<string> genres;
    unordered_map<int, float> userRatings; // userId -> rating

    Movie(int id = 0, string t = "") : movieId(id), title(std::move(t)) {}
};

// User structure to store user preferences
struct User {
    int userId;
    unordered_map<int, float> movieRatings; // movieId -> rating

    User(int id = 0) : userId(id) {}
};

// Node structure for Red-Black Tree
struct MovieNode {
    Movie movie;
    Color color;
    MovieNode *left, *right, *parent;

    MovieNode(Movie m) : movie(std::move(m)), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Red-Black Tree class for storing movies
class MovieRBTree {
private:
    MovieNode *root;
    MovieNode *NIL;
public:
    MovieNode* getNIL() { return NIL; }
    // ← add the following declarations here:
    MovieRBTree();
    ~MovieRBTree();
    void insert(const Movie&);
    MovieNode* search(int movieId);
    std::vector<Movie> inOrder();
    void remove(int movieId);

private:
    // and also declare these helpers:
    void deleteTree(MovieNode *node);
    void rotateLeft(MovieNode *x);
    void rotateRight(MovieNode *x);
    void fixInsert(MovieNode *k);
    MovieNode* searchTreeHelper(MovieNode *node, int movieId);
    void inOrderHelper(MovieNode *node, std::vector<Movie> &movies);
    MovieNode* minimum(MovieNode *node);
    void transplant(MovieNode *u, MovieNode *v);
    void fixDelete(MovieNode *x);
    void deleteNodeHelper(MovieNode *node, int movieId);
};


// Constructor
MovieRBTree::MovieRBTree() {
    NIL = new MovieNode(Movie());
    NIL->color = BLACK;
    NIL->left = nullptr;
    NIL->right = nullptr;
    root = NIL;
}

// Destructor
MovieRBTree::~MovieRBTree() {
    deleteTree(root);
    delete NIL;
}

// Helper to delete entire tree
void MovieRBTree::deleteTree(MovieNode *node) {
    if (node != NIL) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

// Left rotation
void MovieRBTree::rotateLeft(MovieNode *x) {
    MovieNode *y = x->right;
    x->right = y->left;

    if (y->left != NIL) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == nullptr) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

// Right rotation
void MovieRBTree::rotateRight(MovieNode *x) {
    MovieNode *y = x->left;
    x->left = y->right;

    if (y->right != NIL) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == nullptr) {
        root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;
}

// Fix Red-Black Tree properties after insertion
void MovieRBTree::fixInsert(MovieNode *k) {
    MovieNode *u;
    while (k->parent != nullptr && k->parent->color == RED) {
        if (k->parent == k->parent->parent->right) {
            u = k->parent->parent->left;
            if (u->color == RED) {
                u->color = BLACK;
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->left) {
                    k = k->parent;
                    rotateRight(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                rotateLeft(k->parent->parent);
            }
        } else {
            u = k->parent->parent->right;
            if (u->color == RED) {
                u->color = BLACK;
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->right) {
                    k = k->parent;
                    rotateLeft(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                rotateRight(k->parent->parent);
            }
        }
        if (k == root) {
            break;
        }
    }
    root->color = BLACK;
}

// Insert a movie into the Red-Black Tree
void MovieRBTree::insert(const Movie& movie) {
    MovieNode *node = new MovieNode(movie);
    node->left = NIL;
    node->right = NIL;

    MovieNode *y = nullptr;
    MovieNode *x = root;

    while (x != NIL) {
        y = x;
        if (node->movie.movieId < x->movie.movieId) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    node->parent = y;
    if (y == nullptr) {
        root = node;
    } else if (node->movie.movieId < y->movie.movieId) {
        y->left = node;
    } else {
        y->right = node;
    }

    if (node->parent == nullptr) {
        node->color = BLACK;
        return;
    }

    if (node->parent->parent == nullptr) {
        return;
    }

    fixInsert(node);
}

// Search for a movie by ID
MovieNode* MovieRBTree::searchTreeHelper(MovieNode *node, int movieId) {
    if (node == NIL || node->movie.movieId == movieId) {
        return node;
    }

    if (movieId < node->movie.movieId) {
        return searchTreeHelper(node->left, movieId);
    }
    return searchTreeHelper(node->right, movieId);
}

MovieNode* MovieRBTree::search(int movieId) {
    return searchTreeHelper(root, movieId);
}

// Get all movies in order
void MovieRBTree::inOrderHelper(MovieNode *node, vector<Movie> &movies) {
    if (node != NIL) {
        inOrderHelper(node->left, movies);
        movies.push_back(node->movie);
        inOrderHelper(node->right, movies);
    }
}

vector<Movie> MovieRBTree::inOrder() {
    vector<Movie> movies;
    inOrderHelper(root, movies);
    return movies;
}

// Find minimum node (leftmost leaf)
MovieNode* MovieRBTree::minimum(MovieNode *node) {
    while (node->left != NIL) {
        node = node->left;
    }
    return node;
}

// Replace one subtree with another
void MovieRBTree::transplant(MovieNode *u, MovieNode *v) {
    if (u->parent == nullptr) {
        root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

// Fix tree after deletion
void MovieRBTree::fixDelete(MovieNode *x) {
    MovieNode *s;
    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            s = x->parent->right;
            if (s->color == RED) {
                s->color = BLACK;
                x->parent->color = RED;
                rotateLeft(x->parent);
                s = x->parent->right;
            }

            if (s->left->color == BLACK && s->right->color == BLACK) {
                s->color = RED;
                x = x->parent;
            } else {
                if (s->right->color == BLACK) {
                    s->left->color = BLACK;
                    s->color = RED;
                    rotateRight(s);
                    s = x->parent->right;
                }

                s->color = x->parent->color;
                x->parent->color = BLACK;
                s->right->color = BLACK;
                rotateLeft(x->parent);
                x = root;
            }
        } else {
            s = x->parent->left;
            if (s->color == RED) {
                s->color = BLACK;
                x->parent->color = RED;
                rotateRight(x->parent);
                s = x->parent->left;
            }

            if (s->right->color == BLACK && s->left->color == BLACK) {
                s->color = RED;
                x = x->parent;
            } else {
                if (s->left->color == BLACK) {
                    s->right->color = BLACK;
                    s->color = RED;
                    rotateLeft(s);
                    s = x->parent->left;
                }

                s->color = x->parent->color;
                x->parent->color = BLACK;
                s->left->color = BLACK;
                rotateRight(x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}

// Delete a node helper
void MovieRBTree::deleteNodeHelper(MovieNode *node, int movieId) {
    MovieNode *z = NIL;
    MovieNode *x, *y;

    while (node != NIL) {
        if (node->movie.movieId == movieId) {
            z = node;
        }

        if (node->movie.movieId <= movieId) {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    if (z == NIL) {
        return;
    }

    y = z;
    Color y_original_color = y->color;

    if (z->left == NIL) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == NIL) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    delete z;

    if (y_original_color == BLACK) {
        fixDelete(x);
    }
}

// Remove a movie by ID
void MovieRBTree::remove(int movieId) {
    deleteNodeHelper(root, movieId);
};

#endif //RBTREE_H