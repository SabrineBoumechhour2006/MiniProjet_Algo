#include <iostream>
#include <chrono>
#include <vector>
using namespace std;

vector<pair<char, char>> moves;  // Pour stocker les mouvements

void hanoi_recursive(int n, char A, char C, char B) {
    if (n != 0) {
        hanoi_recursive(n - 1, A, B, C);
        moves.push_back({A, C});  // Move(A, C)
        hanoi_recursive(n - 1, B, C, A);
    }
}

void hanoi_iterative(int n, char A, char C, char B) {
    long long total_moves = (1LL << n) - 1;
    if (n % 2 == 0) swap(C, B);

    int stackA[64], stackB[64], stackC[64];
    int topA = n - 1, topB = -1, topC = -1;

    for (int i = 0; i < n; i++) stackA[i] = n - i;

    auto moveDisk = [&](char fromPeg, char toPeg, int &topFrom, int stackFrom[], int &topTo, int stackTo[]) {
        int disk = stackFrom[topFrom--];
        stackTo[++topTo] = disk;
        moves.push_back({fromPeg, toPeg});
    };

    for (long long i = 1; i <= total_moves; i++) {

        // Move between A and C
        if (i % 3 == 1) {
            if (topA == -1) moveDisk(C, A, topC, stackC, topA, stackA);
            else if (topC == -1) moveDisk(A, C, topA, stackA, topC, stackC);
            else if (stackA[topA] < stackC[topC]) moveDisk(A, C, topA, stackA, topC, stackC);
            else moveDisk(C, A, topC, stackC, topA, stackA);
        }

        // Move between A and B
        else if (i % 3 == 2) {
            if (topA == -1) moveDisk(B, A, topB, stackB, topA, stackA);
            else if (topB == -1) moveDisk(A, B, topA, stackA, topB, stackB);
            else if (stackA[topA] < stackB[topB]) moveDisk(A, B, topA, stackA, topB, stackB);
            else moveDisk(B, A, topB, stackB, topA, stackA);
        }

        // Move between B and C
        else {
            if (topB == -1) moveDisk(C, B, topC, stackC, topB, stackB);
            else if (topC == -1) moveDisk(B, C, topB, stackB, topC, stackC);
            else if (stackB[topB] < stackC[topC]) moveDisk(B, C, topB, stackB, topC, stackC);
            else moveDisk(C, B, topC, stackC, topB, stackB);
        }
    }
}
int main() {
    while (true) {
        int n, choice;
        cout << "Enter number of disks: ";
        cin >> n;

        cout << "\nChoose method:\n1 - Recursive\n2 - Iterative\nYour choice: ";
        cin >> choice;

        if (choice != 1 && choice != 2) {
            cout << "Invalid choice. Try again.\n";
            continue;
        }

        moves.clear();  // Vider le vecteur pour chaque test

        // --- START TOTAL TIMER ---
        auto start_total = chrono::high_resolution_clock::now();

        // 1. Timer pour le calcul de la solution (génération des mouvements)
        auto start_calc = chrono::high_resolution_clock::now();
        if (choice == 1) hanoi_recursive(n, 'A', 'C', 'B');
        else hanoi_iterative(n, 'A', 'C', 'B');
        auto end_calc = chrono::high_resolution_clock::now();
        double time_calc = chrono::duration<double>(end_calc - start_calc).count();

        // 2. Timer pour simuler l’exécution (parcourir les mouvements)
        long long execution_count = 0;
        auto start_exec = chrono::high_resolution_clock::now();
        for (auto &m : moves) {
            execution_count++;
        }
        auto end_exec = chrono::high_resolution_clock::now();
        double time_exec = chrono::duration<double>(end_exec - start_exec).count();

        // --- END TOTAL TIMER ---
        auto end_total = chrono::high_resolution_clock::now();
        double time_total = chrono::duration<double>(end_total - start_total).count();

        // Affichage des résultats
        cout << "\n------ RESULTS ------\n";
        cout << "Method: " << (choice == 1 ? "Recursive" : "Iterative") << endl;
        cout << "Disks: " << n << endl;
        cout << "Total moves: " << moves.size() << endl;
        cout << "Time to calculate solution: " << time_calc << " seconds\n";
        cout << "Time to reach final solution: " << time_exec << " seconds\n";
        cout << "Total time from start to finish: " << time_total << " seconds\n";
        cout << "---------------------\n";

        char again;
        cout << "Voulez-vous tester à nouveau ? (o/n) : ";
        cin >> again;
        if (again != 'o' && again != 'O') break;
        cout << endl;
    }
    cout << "Au revoir !\n";
    return 0;
}
