#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>

const int N_MACHINES = 5;
const int N_STOCK = 100;

std::mutex mtx;                    // Mutex for thread synchronization
std::vector<int> stock(N_STOCK);    // Stock of articles
std::vector<bool> article_traite(N_STOCK, false); // Status of articles (processed or not)
std::atomic<int> article_index(0); // Atomic index for article processing

// Initialize stock and their processing status
void initialiser_stock() {
    for (int i = 0; i < N_STOCK; i++) {
        stock[i] = i + 1; // Stock of items, 1-based index
    }
}

// Simulate a task performed by a machine on an article
void simuler_tache(int machine_id, int article) {
    int resultat = machine_id * 100 + article;
    std::cout << "Machine: " << machine_id 
              << " traite l'article " << article 
              << ", assigne SKU: " << resultat << std::endl;
    
    // Simulate processing time
    std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10000));
}

// Worker function for each machine thread
void machine_worker(int machine_id) {
    int machine_compteur = 0;

    while (true) {
        int index = -1;

        // Locking the mutex to safely retrieve and increment the article_index
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (article_index >= N_STOCK) {
                break; // All articles processed
            }

            index = article_index++;  // Safely increment the article_index and assign it
        }

        // If the article is not processed, the machine processes it
        if (!article_traite[index]) {
            simuler_tache(machine_id + 1, stock[index]);
            article_traite[index] = true; // Mark the article as processed
            machine_compteur++;
        }
    }
    
    std::cout << "Machine " << machine_id << " a fini son travail avec " 
              << machine_compteur << " articles traites" << std::endl;
}

int main() {
    std::vector<std::thread> machines;
    
    initialiser_stock();
    
    // Create machine worker threads
    for (int i = 0; i < N_MACHINES; i++) {
        machines.push_back(std::thread(machine_worker, i));
    }

    // Wait for all machines to finish their work
    for (auto& t : machines) {
        t.join();
    }

    // Check if all articles were processed
    bool flag = true;
    for (int i = 0; i < N_STOCK; i++) {
        if (!article_traite[i]) {
            std::cout << "Erreur: l'article " << stock[i] << " n'a pas ete traite." << std::endl;
            flag = false;
        }
    }

    if (flag) {
        std::cout << "Tous les articles ont ete trates avec succes." << std::endl;
    }

    return 0;
}
