#include <iostream>
#include <fstream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <mutex>

#define NUM_READERS 5
#define NUM_WRITERS 2

// Shared resource and read-write lock
std::shared_mutex rwlock;
int counter_reader = 0;
int counter_writer = 0;
const std::string filename = "shared_file.txt";

// Reader function
void reader(int id) {
    while (true) {
        {
            std::shared_lock<std::shared_mutex> lock(rwlock);  // Shared lock for reading
            std::ifstream file(filename);
            if (!file) {
                std::cerr << "Erreur lors de l'ouverture du fichier en lecture" << std::endl;
                continue;
            }

            // Display the message in green and bold
            std::cout << "\033[1;32mLecteur " << id << ": ------------------------lecture du fichier----------------------------\033[0m" << std::endl;

            char ch;
            while (file.get(ch)) {
                std::cout.put(ch);
            }
            counter_reader++;
            std::cout << "FIN LECTURE, lecteur " << id << std::endl;
            std::cout << "Total acces pour les lecteurs: " << counter_reader << std::endl;
        }  // Lock automatically released here

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Writer function
void writer(int id) {
    while (true) {
        {
            std::unique_lock<std::shared_mutex> lock(rwlock);  // Exclusive lock for writing
            std::ofstream file(filename, std::ios::app);
            if (!file) {
                std::cerr << "Erreur lors de l'ouverture du fichier en ecriture" << std::endl;
                continue;
            }

            std::cout << "======================DEBUT ECRITURE, ecrivain " << id << std::endl;
            file << "Ecrivain " << id << " ecrit dans le fichier." << std::endl;
            counter_writer++;
            std::cout << "======================FIN ECRITURE, ecrivain " << id << std::endl;
            std::cout << "Total acces pour les ecrivains: " << counter_writer << std::endl;
        }  // Lock automatically released here

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    // Initialize the file by creating it
    std::ofstream file(filename);
    file.close();

    // Create reader and writer threads
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    // Create reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        readers.emplace_back(reader, i + 1);
    }

    // Create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        writers.emplace_back(writer, i + 1);
    }

    // Wait for all threads to finish
    for (auto& t : readers) {
        t.join();
    }

    for (auto& t : writers) {
        t.join();
    }

    return 0;
}
