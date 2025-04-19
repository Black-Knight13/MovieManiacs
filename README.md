<H1> COP3530 - Project 3 MovieRanker 🎥 </h1>

## 📌 Overview
The UI for this project can be accesed directly from your local IDE. It compares two popular recommendation algorithms—Collaborative Filtering and Content-Based Filtering—to evaluate their effectiveness in providing relevant movie suggestions. The system also leverages **Red-Black Trees (RBTs)** for fast data retrieval and **Max Heaps** to prioritize top-rated or most similar recommendations.

---

## 🚀 Features
- Input a movie title & year it was produced to get suggestions based on:
  - 🎯 **Collaborative Filtering**: Recommends movies liked by users with similar preferences.
  - 🧠 **Content-Based Filtering**: Recommends movies with similar attributes (e.g., genre).
- Efficient storage using **Red-Black Trees** for fast movie lookup.
- **Max Heap** used for ranking and retrieving top recommendations.
- Real-time performance comparison:
  - Execution time
  - Mean Absolute Error (MAE)
  - Memory usage
- Cleaned and preprocessed dataset to improve accuracy.

---

## 📂 Dataset
We use the [MovieLens 25M Dataset](https://grouplens.org/datasets/movielens/25m/) from GroupLens.  
The key files include:
- `movies.csv` – movie IDs, titles, and genres
- `ratings.csv` – user ratings for movies

---

## 🛠 Tools & Technologies
- **Language**: C++
- **IDE**: CLion
- **Libraries & Structures**:
  - Standard Template Library (STL)
  - Custom Red-Black Tree
  - Max Heap
  - CSV parsing logic
  - High-resolution timers for performance benchmarking

---

## 🧪 Algorithms Used
- **Collaborative Filtering**:
  - Based on user similarity and shared preferences.
  - Utilizes Red-Black Tree for movie data and user lookups.
- **Content-Based Filtering**:
  - Uses genre similarity and feature weighting.
  - Max Heap used to retrieve top-N similar movies.

---

## 🧑‍💻 How to Use
1. 
