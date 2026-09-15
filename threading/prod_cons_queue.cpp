#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#define QUEUE_CAPACITY 10

class Queue {
public:
    Queue(int maxItems) : premier(0), dernier(0), taille(0), nombre_items_pour_test(maxItems) {
        items.resize(QUEUE_CAPACITY);
    }

	void enfiler(int item) {
		std::unique_lock<std::mutex> lock(mutex_queue);

		// Wait until there is space in the queue or the production limit is reached
		while (taille == QUEUE_CAPACITY && compteur_prod < nombre_items_pour_test) {
			plein.wait(lock);
		}

		// Check if we should stop producing
		if (compteur_prod >= nombre_items_pour_test) {
			return;
		}

		items[dernier] = item;
		dernier = (dernier + 1) % QUEUE_CAPACITY;
		taille++;

		vide.notify_one();
	}

	int defiler() {
		std::unique_lock<std::mutex> lock(mutex_queue);

		// Wait until there is an item in the queue or all items have been consumed
		while (taille == 0 && compteur_cons < nombre_items_pour_test) {
			vide.wait(lock);
		}

		// Check if we should stop consuming
		if (compteur_cons >= nombre_items_pour_test) {
			return -1; // Use a sentinel value to indicate no more items
		}

		int item = items[premier];
		premier = (premier + 1) % QUEUE_CAPACITY;
		taille--;

		plein.notify_one();
		return item;
	}


    int getNombreItemsPourTest() const {
        return nombre_items_pour_test;
    }

    void incrementCompteurProd() {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        compteur_prod++;
    }

    void incrementCompteurCons() {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        compteur_cons++;
    }

    int getCompteurProd() const {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        return compteur_prod;
    }

    int getCompteurCons() const {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        return compteur_cons;
    }
	
	bool canProduce() const {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        return compteur_prod < nombre_items_pour_test;
    }

    bool canConsume() const {
        std::lock_guard<std::mutex> lock(compteur_mutex);
        return compteur_cons < nombre_items_pour_test;
    }

private:
    std::vector<int> items;
    int premier;
    int dernier;
    int taille;
    int nombre_items_pour_test;
	mutable std::mutex compteur_mutex; // Now mutable
    std::mutex mutex_queue;
    std::condition_variable plein, vide;

    int compteur_prod = 0;
    int compteur_cons = 0;
    
};

void producteur(Queue* queue) {
    while (queue->canProduce()) {
        int item = rand() % 1000;
        queue->enfiler(item);
        queue->incrementCompteurProd();

        std::cout << "Producteur a produit: " << item << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100000));
    }

    // Add sentinel value to signal consumers to stop
    queue->enfiler(-1);
}

void consommateur(Queue* queue) {
    while (true) {
        int item = queue->defiler();

        // Check for sentinel value
        if (item == -1) {
            break;
        }

        queue->incrementCompteurCons();
        std::cout << "Consommateur a consomme: " << item << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100000));
    }
}
// Function to create and join threads
void createJoinThreads(Queue* queue, int nom_producteurs, int nom_consommateurs) {
    std::vector<std::thread> producteurs(nom_producteurs);
    std::vector<std::thread> consommateurs(nom_consommateurs);

    // Create producers
    for (int i = 0; i < nom_producteurs; i++) {
        producteurs[i] = std::thread(producteur, queue);
    }

    // Create consumers
    for (int i = 0; i < nom_consommateurs; i++) {
        consommateurs[i] = std::thread(consommateur, queue);
    }

    // Join all producer and consumer threads
    for (auto& t : producteurs) {
        t.join();
    }

    for (auto& t : consommateurs) {
        t.join();
    }

    // Print final counts
    std::cout << "Total items produits: " << queue->getCompteurProd() << std::endl;
    std::cout << "Total items consommes: " << queue->getCompteurCons() << std::endl;
}

void test_3prod_3cons_50items(Queue* queue) {
    int nom_producteurs = 3;
    int nom_consommateurs = 3;
    createJoinThreads(queue, nom_producteurs, nom_consommateurs);
}

void test_5prod_10cons_100items(Queue* queue) {
    int nom_producteurs = 5;
    int nom_consommateurs = 10;
    createJoinThreads(queue, nom_producteurs, nom_consommateurs);
}

void test_10prod_5cons_100items(Queue* queue) {
    int nom_producteurs = 10;
    int nom_consommateurs = 5;
    createJoinThreads(queue, nom_producteurs, nom_consommateurs);
}

void test_20prod_20cons_500items(Queue* queue) {
    int nom_producteurs = 20;
    int nom_consommateurs = 20;
    createJoinThreads(queue, nom_producteurs, nom_consommateurs);
}

int main() {
    srand(time(NULL));

    // Create and initialize the queue
//     Queue* queue = new Queue(50);
// 	Queue* queue1 = new Queue(100);
// 	Queue* queue2 = new Queue(100);
	Queue* queue3 = new Queue(500);
	
    // Run tests
    // test_3prod_3cons_50items(queue);
    // test_5prod_10cons_100items(queue1);
    // test_10prod_5cons_100items(queue2);
    test_20prod_20cons_500items(queue3);

    // Cleanup (handled by RAII in C++)
//     delete queue;
// 	delete queue1;
// 	delete queue2;
	delete queue3;

    return 0;
}
